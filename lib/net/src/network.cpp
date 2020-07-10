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
        ipaddr_aton(argv[i+1], ifconf + i);
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
    TypeObj& type () const override;

    SocketObj (tcp_pcb* p) : socket (p) {}

    Value attr (const char* key, Value& self) const override;

    tcp_pcb* socket;
};

Value SocketObj::create (const TypeObj&, int argc, Value argv[]) {
    debugf("socketobj %d\n", argc);
    assert(argc == 1);
    auto p = tcp_new();
    assert(p != 0);
    return new SocketObj (p);
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

const FunObj fo_bind = f_bind;
const FunObj fo_listen = f_listen;

static const LookupObj::Item socketMap [] = {
    { "bind", &fo_bind },
    { "listen", &fo_listen },
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

void testNetwork () {
    ip4_addr ifconf [4];
    ipaddr_aton("192.168.188.2", ifconf + 0); // my ip
    ipaddr_aton("255.255.255.0", ifconf + 1); // netmask
    ipaddr_aton("192.168.188.1", ifconf + 2); // gateway
    ipaddr_aton("8.8.8.8",       ifconf + 3); // dns

    lwip_init();

    enc_if.hwaddr_len = 6; /* demo mac address */
    memcpy(enc_if.hwaddr, "\x00\x01\x02\x03\x04\x05", 6);

    auto p = netif_add(&enc_if, ifconf+0, ifconf+1, ifconf+2, &enc_hw,
                        mn_init, ethernet_input);
    (void) p;
    assert(p != 0);

    netif_set_default(&enc_if);
    netif_set_up(&enc_if);
    //dns_setserver(0, ifconf+3);

    printf("Setup completed\n");

    auto pcb = tcp_new(); assert(pcb != NULL);
    auto r = tcp_bind(pcb, IP_ADDR_ANY, 1234); (void) r; assert(r == 0);
    pcb = tcp_listen_with_backlog(pcb, 3); assert(pcb != NULL);
    //pcb = tcp_listen(pcb); assert(pcb != NULL);

    tcp_accept(pcb, [](void *arg, struct tcp_pcb *newpcb, err_t err) -> err_t {
        printf("\t ACCEPT!\n");

        tcp_recv(newpcb, [](void *arg, tcp_pcb *tpcb, pbuf *p, err_t err) -> err_t {
            //printf("\t %p %d\n", p, err);
            if (p != 0) {
                printf("\t RECEIVE! %s\n", p->payload);
                tcp_recved(tpcb, p->tot_len);
                pbuf_free(p);
            } else {
                printf("\t CLOSE!\n");
                tcp_recv(tpcb, 0);
                tcp_close(tpcb);
            }
            return ERR_OK;
        });

        return ERR_OK;
    });

/*
    auto my_pcb = udp_new();
    assert(my_pcb != NULL);

    udp_recv(my_pcb, [](void *arg, udp_pcb *pcb, pbuf *p, const ip_addr_t *addr, uint16_t port) {
        printf("\t UDP! %s\n", p->payload);

        auto r = pbuf_alloc(PBUF_TRANSPORT, 0, PBUF_RAM);
        auto m = pbuf_alloc(PBUF_RAW, 5, PBUF_ROM);
        m->payload = (char*) "hello";
        pbuf_cat(r, m);
        udp_sendto(pcb, r, addr, port);
        pbuf_free(r);

	pbuf_free(p);
    }, 0);
    udp_bind(my_pcb, IP_ADDR_ANY, 4321);
*/

    while (true) {
        mn_poll(&enc_if);
        sys_check_timeouts();
    }
}

#endif // INCLUDE_NETWORK
