#include "monty.h"
#include "config.h"
#include "network.h"

#if INCLUDE_NETWORK

#include <lwip/inet.h>
#include <lwip/tcp.h>
#include <lwip/netif.h>
#include <lwip/dhcp.h>
#include <lwip/dns.h>
#include <lwip/udp.h>
#include <lwip/init.h>
#include <lwip/pbuf.h>
#include <lwip/stats.h>
#include <lwip/sys.h>
#include <lwip/timeouts.h>
#include <lwip/err.h>
#include <netif/etharp.h>

extern "C" {
#include "enchw.h"
#include "enc28j60.h"
}

#include <string.h>
#include <jee.h>

extern "C" int debugf (const char* fmt, ...); // TODO put this in config.h?

SpiGpio< PINS_NETWORK > spi;

void enchw_setup (enchw_device_t*) {
    spi.init();
#if CONFIG == F103ZE_NET
    PinA<3> reset; reset.mode(Pinmode::out);
    reset = 0; wait_ms(2); reset = 1; wait_ms(10);
#endif
}

void enchw_select (enchw_device_t*) {
    spi.enable();
}

void enchw_unselect (enchw_device_t*) {
    spi.disable();
}

uint8_t enchw_exchangebyte (enchw_device_t*, uint8_t b) {
    return spi.transfer(b);
}

uint32_t sys_now () {
    return ticks;
}

static netif enc_if;
static enc_device_t enc_hw;

static void linkIsUp (netif* netif, bool up) {
    static auto linkState = false;
    if (up == linkState)
        return;
    linkState = up;
    (linkState ? netif_set_link_up : netif_set_link_down)(netif);
}

static void mn_poll (netif* netif) {
    auto dev = (enc_device_t*) netif->state;

    auto linkState = enc_MII_read(dev, (enc_register_t) ENC_PHSTAT1) & (1<<2);
    linkIsUp(netif, linkState != 0);

    if (enc_RCR(dev, ENC_EPKTCNT)) {
        pbuf* buf = NULL;
        if (enc_read_received_pbuf(dev, &buf) == 0)
            netif->input(buf, netif);
    }
}

static err_t mn_linkoutput (netif* netif, pbuf* p) {
    auto dev = (enc_device_t*) netif->state;
    enc_transmit_pbuf(dev, p);
    return ERR_OK;
}

static err_t mn_init (netif* netif) {
    auto dev = (enc_device_t*) netif->state;

    if (enc_setup_basic(dev) != 0 || enc_bist_manual(dev) != 0)
        return ERR_IF;

    enc_ethernet_setup(dev, 4*1024, netif->hwaddr);
    //enc_set_multicast_reception(dev, 1); // always enable?

    netif->output = etharp_output;
    netif->linkoutput = mn_linkoutput;
    netif->mtu = 1500;
    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;

    return ERR_OK;
}

static Value f_poll (int argc, Value argv []) {
    //printf("poll %d\n", argc);
    assert(argc == 1);
    mn_poll(&enc_if);
    sys_check_timeouts();
    return Value::nil;
}

static Value f_ifconfig (int argc, Value argv []) {
    static bool inited = false;
    if (!inited) {
        inited = true;
        lwip_init();
        wait_ms(500); // delay console logging
    }

    printf("ifconfig %d\n", argc);
    assert(argc == 5);
    ip4_addr ifconf [4];
    for (int i = 0; i < 4; ++i) {
        assert(argv[i+1].isStr());
        auto ok = ip4addr_aton(argv[i+1], ifconf + i);
        assert(ok == 1);
    }

    enc_if.hwaddr_len = 6; /* demo mac address */
    memcpy(enc_if.hwaddr, "\x00\x01\x02\x03\x04\x05", 6);

    auto p = netif_add(&enc_if, ifconf+0, ifconf+1, ifconf+2, &enc_hw,
                        mn_init, ethernet_input);
    (void) p;
    assert(p != 0);

    netif_set_default(&enc_if);
    netif_set_up(&enc_if);

    return Value::nil;
}

struct SocketObj : Object {
    static Value create (const TypeObj&, int argc, Value argv[]);
    static const LookupObj attrs;
    static TypeObj info;

    SocketObj (tcp_pcb* p) : socket (p), sess (Value::nil) {}

    TypeObj& type () const override;
    void mark (void (*gc)(const Object&)) const override;
    Value attr (const char* key, Value& self) const override;

    tcp_pcb* socket;
    BytecodeObj* accepter = 0;
    Value sess; // TODO new session, but what if another one comes in?
    ListObj* pending = 0;
};

Value SocketObj::create (const TypeObj&, int argc, Value argv[]) {
    debugf("socketobj %d\n", argc);
    assert(argc == 1);
    auto p = tcp_new();
    assert(p != 0);
    return new SocketObj (p);
}

void SocketObj::mark (void (*gc)(const Object&)) const {
    if (accepter != 0)
        gc(*accepter);
    if (sess.isObj())
        gc(sess.obj());
    if (pending != 0)
        gc(*pending);
}

Value SocketObj::attr (const char* key, Value& self) const {
    self = Value::nil;
    return attrs.at(key);
}

static Value f_bind (int argc, Value argv []) {
    debugf("bind %d\n", argc);
    assert(argc == 2 && argv[1].isInt());
    auto& self = argv[0].asType<SocketObj>();
    auto r = tcp_bind(self.socket, IP_ADDR_ANY, argv[1]);
    assert(r == 0);
    return Value::nil;
}

static Value f_listen (int argc, Value argv []) {
    debugf("listen %d\n", argc);
    assert(argc == 2 && argv[1].isInt());
    auto& self = argv[0].asType<SocketObj>();
    self.socket = tcp_listen_with_backlog(self.socket, argv[1]);
    assert(self.socket != NULL);
    return Value::nil;
}

static Value f_accept (int argc, Value argv []) {
    debugf("accept %d\n", argc);
    assert(argc == 2);
    auto& self = argv[0].asType<SocketObj>();
    auto& bco = argv[1].asType<BytecodeObj>();
    assert((bco.scope & 1) != 0);

    self.accepter = &bco;
    tcp_arg(self.socket, &self);

    tcp_accept(self.socket, [](void *arg, struct tcp_pcb *newpcb, err_t err) -> err_t {
        auto& self = *(SocketObj*) arg;
        assert(self.accepter != 0);

        auto conn = new SocketObj (newpcb);
        conn->pending = new ListObj (0, 0);
        self.sess = conn;

        Value v = self.accepter->call(1, &self.sess);
        assert(!v.isNil()); // it better be a generator!
        Context::tasks.append(v);

        return ERR_OK;
    });

    return Value::nil;
}

static Value f_read (int argc, Value argv []) {
    assert(argc == 2 && argv[1].isInt());
    auto& self = argv[0].asType<SocketObj>();
    assert(self.pending != 0);

    tcp_arg(self.socket, &self);
    tcp_recv(self.socket, [](void *arg, tcp_pcb *tpcb, pbuf *p, err_t err) -> err_t {
        auto& self = *(SocketObj*) arg;
        assert(self.pending != 0);
        //printf("\t %p %d\n", p, err);
        if (p != 0) {
            if (self.pending->len() > 0) {
                Value v = self.pending->pop(0);
                Context::tasks.append(v);
            }
            tcp_recved(tpcb, p->tot_len);
            pbuf_free(p);
        } else {
            printf("\t CLOSE!\n");
            tcp_recv(tpcb, 0);
            tcp_close(tpcb);
        }
        return ERR_OK;
    });

    printf("suspend on read\n");
    Context::suspend(*self.pending);

    return Value::nil;
}

const FunObj fo_bind = f_bind;
const FunObj fo_listen = f_listen;
const FunObj fo_accept = f_accept;
const FunObj fo_read = f_read;

static const LookupObj::Item socketMap [] = {
    { "bind", &fo_bind },
    { "listen", &fo_listen },
    { "accept", &fo_accept },
    { "read", &fo_read },
};

const LookupObj SocketObj::attrs (socketMap, sizeof socketMap / sizeof *socketMap);

TypeObj SocketObj::info ("<socket>", SocketObj::create, &SocketObj::attrs);
TypeObj& SocketObj::type () const { return info; }

const FunObj fo_poll = f_poll;
const FunObj fo_ifconfig = f_ifconfig;

static const LookupObj::Item lo_network [] = {
    { "ifconfig", &fo_ifconfig },
    { "poll", &fo_poll },
    { "socket", &SocketObj::info },
};

static const LookupObj ma_network (lo_network, sizeof lo_network / sizeof *lo_network);
const ModuleObj m_network (&ma_network);

static const LookupObj::Item lo_socket [] = {
};

static const LookupObj ma_socket (lo_socket, sizeof lo_socket / sizeof *lo_socket);
const ModuleObj m_socket (&ma_socket);

#endif // INCLUDE_NETWORK
