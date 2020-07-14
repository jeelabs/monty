#include "monty.h"
#include "config.h"

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

static SpiGpio< PINS_NETWORK > spi;

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
    }

    assert(argc == 5);
    ip4_addr ifconf [4];
    for (int i = 0; i < 4; ++i) {
        assert(argv[i+1].isStr());
        auto ok = ip4addr_aton(argv[i+1], ifconf + i);
        (void) ok; assert(ok == 1);
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

    SocketObj (tcp_pcb* p) : socket (p), readQueue (0, 0),
                             writeQueue (0, 0), toSend (Value::nil) {
        tcp_arg(socket, this);
    }

    TypeObj& type () const override;
    void mark (void (*gc)(const Object&)) const override;
    Value attr (const char* key, Value& self) const override;

    Value bind (int arg);
    Value connect (int argc, Value argv []);
    Value listen (int arg);
    Value accept (Value arg);
    Value read (int arg);
    Value write (Value arg);
    Value close ();

private:
    bool sendIt (Value arg);

    tcp_pcb* socket;
    BytecodeObj* accepter = 0;
    ListObj readQueue, writeQueue; // TODO don't need to be queues: fix suspend!
    Value toSend;
};

Value SocketObj::create (const TypeObj&, int argc, Value argv[]) {
    assert(argc == 1);
    auto p = tcp_new();
    assert(p != 0);
    return new SocketObj (p);
}

void SocketObj::mark (void (*gc)(const Object&)) const {
    gc(readQueue);
    gc(writeQueue);
    if (accepter != 0)
        gc(*accepter);
    // TODO toSend always a str for now, no gc needed yet
}

Value SocketObj::attr (const char* key, Value& self) const {
    self = Value::nil;
    return attrs.at(key);
}

Value SocketObj::bind (int arg) {
    auto r = tcp_bind(socket, IP_ADDR_ANY, arg);
    (void) r; assert(r == 0);
    return Value::nil;
}

Value SocketObj::connect (int argc, Value argv []) {
    assert(argc == 2 && argv[0].isStr() && argv[1].isInt());
    ip4_addr host;
    auto ok = ip4addr_aton(argv[0], &host);
    (void) ok; assert(ok == 1);

    auto r = tcp_connect(socket, &host, argv[1], [](void *arg, tcp_pcb *tpcb, err_t err) -> err_t {
        auto& self = *(SocketObj*) arg;
        assert(self.socket == tpcb);
        assert(self.readQueue.len() > 0);
        Context::tasks.append(self.readQueue.pop(0));
        return ERR_OK;
    });
    (void) r; assert(r == 0);

    Context::suspend(readQueue);
    return Value::nil;
}

Value SocketObj::listen (int arg) {
    socket = tcp_listen_with_backlog(socket, arg);
    assert(socket != NULL);
    return Value::nil;
}

Value SocketObj::accept (Value arg) {
    auto bco = arg.asType<BytecodeObj>();
    assert(bco != 0 && (bco->scope & 1) != 0); // make sure it's a generator
    accepter = bco;

    tcp_accept(socket, [](void *arg, tcp_pcb *newpcb, err_t err) -> err_t {
        auto& self = *(SocketObj*) arg;
        assert(self.accepter != 0);

        Value argv = new SocketObj (newpcb);
        Value v = self.accepter->call(1, &argv);
        assert(v.isObj());
        Context::tasks.append(v);
        return ERR_OK;
    });

    return Value::nil;
}

Value SocketObj::read (int arg) {
    Context::suspend(readQueue);

    tcp_recv(socket, [](void *arg, tcp_pcb *tpcb, pbuf *p, err_t err) -> err_t {
        auto& self = *(SocketObj*) arg;
        assert(self.socket == tpcb);
        if (p != 0) {
            if (self.readQueue.len() == 0)
                return ERR_BUF;
            // TODO copy incoming data to a buffer, and pass it to task
            Context::tasks.append(self.readQueue.pop(0));
            tcp_recved(tpcb, p->tot_len);
            pbuf_free(p);
        } else
            self.close();
        return ERR_OK;
    });

    return Value::nil;
}

bool SocketObj::sendIt (Value arg) {
#if 0
    assert(arg.isStr()); // TODO for now ...
    const char* s = arg;
#else
    const char* s = "mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm\n";
#endif
    auto n = strlen(s);
    if (n + 50 > tcp_sndbuf(socket)) // check for some spare room, just in case
        return false;

    auto r = tcp_write(socket, s, n, TCP_WRITE_FLAG_COPY);
    if (r != 0) { // session probably closed by peer
        printf("write err %d sndbuf %d\n", r, tcp_sndbuf(socket));
        close();
        return false;
    }

    return true;
}

Value SocketObj::write (Value arg) {
    assert(toSend.isNil()); // don't allow multiple outstanding sends
    if (sendIt(arg))
        return Value::nil;

    toSend = arg;
    printf("suspending write\n");
    Context::suspend(writeQueue);

    tcp_sent(socket, [](void *arg, tcp_pcb *tpcb, uint16_t len) -> err_t {
        printf("sent %p %d\n", arg, len);
        auto& self = *(SocketObj*) arg;
        if (self.socket != 0) {
            assert(self.socket == tpcb);
            if (self.sendIt(self.toSend)) {
                tcp_sent(tpcb, 0);
                self.toSend = Value::nil;
                assert(self.writeQueue.len() > 0);
                Context::tasks.append(self.writeQueue.pop(0));
            }
        }
        return ERR_OK;
    });

    return Value::nil;
}

Value SocketObj::close () {
    printf("\t CLOSE! %p r %d w %d\n",
            socket, (int) readQueue.len(), (int) writeQueue.len());
    if (socket != 0) {
        tcp_recv(socket, 0);
        tcp_close(socket);
        socket = 0;
    }
    toSend = Value::nil;
    if (readQueue.len() > 0)
        readQueue.pop(0); // TODO throw Context::tasks.append(readQueue.pop(0));
    assert(readQueue.len() == 0);
    if (writeQueue.len() > 0)
        writeQueue.pop(0); // TODO throw Context::tasks.append(writeQueue.pop(0));
    assert(writeQueue.len() == 0);
    return Value::nil;
}

static const auto m_bind = MethObj::wrap(&SocketObj::bind);
static const MethObj mo_bind = m_bind;

static const auto m_connect = MethObj::wrap(&SocketObj::connect);
static const MethObj mo_connect = m_connect;

static const auto m_listen = MethObj::wrap(&SocketObj::listen);
static const MethObj mo_listen = m_listen;

static const auto m_accept = MethObj::wrap(&SocketObj::accept);
static const MethObj mo_accept = m_accept;

static const auto m_read = MethObj::wrap(&SocketObj::read);
static const MethObj mo_read = m_read;

static const auto m_write = MethObj::wrap(&SocketObj::write);
static const MethObj mo_write = m_write;

static const auto m_close = MethObj::wrap(&SocketObj::close);
static const MethObj mo_close = m_close;

static const LookupObj::Item socketMap [] = {
    { "bind", &mo_bind },
    { "connect", &mo_connect },
    { "listen", &mo_listen },
    { "accept", &mo_accept },
    { "read", &mo_read },
    { "write", &mo_write },
    { "close", &mo_close },
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

#endif // INCLUDE_NETWORK
