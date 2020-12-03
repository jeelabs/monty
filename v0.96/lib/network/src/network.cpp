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

using namespace Monty;

static SpiGpio< NETWORK_SPI_PINS > spi;

void enchw_setup (enchw_device_t*) {
    spi.init();
#ifdef NETWORK_RESET_PIN
    NETWORK_RESET_PIN reset; reset.mode(Pinmode::out);
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

static Value f_poll (ArgVec const& args) {
    assert(args.num == 0);
    mn_poll(&enc_if);
    sys_check_timeouts();
    return {};
}

static Value f_ifconfig (ArgVec const& args) {
    static bool inited = false;
    if (!inited) {
        inited = true;
        lwip_init();
    }

    assert(args.num == 4);
    ip4_addr ifconf [4];
    for (int i = 0; i < 4; ++i) {
        assert(args[i].isStr());
        auto ok = ip4addr_aton(args[i], ifconf + i);
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

    return {};
}

struct Socket: Object {
    static auto create (ArgVec const&, Type const*) -> Value;
    static Lookup const attrs;
    static Type const info;
    auto type () const -> Type const& override { return info; }

    Socket (tcp_pcb* p) : socket (p) { tcp_arg(socket, this); }

    Value bind (int arg);
    Value connect (ArgVec const& args);
    Value listen (ArgVec const& args);
    Value read (Value arg);
    Value write (Value arg);
    Value close ();

    auto attr (char const* name, Value& self) const -> Value override;

    void marker () const override;
private:
    bool sendIt (Value arg);

    tcp_pcb* socket;
    Callable* accepter {nullptr};
    List readQueue, writeQueue; // TODO don't use queues: fix suspend!
    Value toSend;
    Array* recvBuf {nullptr};
};

auto Socket::create (ArgVec const& args, Type const*) -> Value {
    assert(args.num == 0);
    auto p = tcp_new();
    assert(p != 0);
    return new Socket (p);
}

void Socket::marker () const {
    readQueue.marker();
    writeQueue.marker();
    toSend.marker();
    mark(accepter);
    mark(recvBuf);
}

Value Socket::attr (const char* key, Value& self) const {
    return attrs.getAt(key);
}

Value Socket::bind (int arg) {
    auto r = tcp_bind(socket, IP_ADDR_ANY, arg);
    (void) r; assert(r == 0);
    return {};
}

Value Socket::connect (ArgVec const& args) {
    assert(args.num == 2 && args[0].isStr() && args[1].isInt());
    ip4_addr host;
    auto ok = ip4addr_aton(args[0], &host);
    (void) ok; assert(ok == 1);

    auto r = tcp_connect(socket, &host, args[1], [](void *arg, tcp_pcb *tpcb, err_t err) -> err_t {
        auto& self = *(Socket*) arg;
        assert(self.socket == tpcb);
        assert(self.readQueue.len() > 0);
        Interp::tasks.append(self.readQueue.pop(0));
        return ERR_OK;
    });
    (void) r; assert(r == 0);

    Interp::suspend(readQueue);
    return {};
}

Value Socket::listen (ArgVec const& args) {
    assert(args.num == 2 && args[1].isInt());
    auto& cao = args[0].asType<Callable>();
    assert(cao.bc.isGenerator());

    socket = tcp_listen_with_backlog(socket, args[1]);
    assert(socket != NULL);

    accepter = &cao;
    tcp_accept(socket, [](void *arg, tcp_pcb *newpcb, err_t err) -> err_t {
        auto& self = *(Socket*) arg;
        assert(self.accepter != 0);

        Vector avec;
        avec.insert(0);
        avec[0] = new Socket (newpcb);

        Value v = self.accepter->call({avec, 1, 0});
        assert(v.isObj());
        Interp::tasks.append(v);
        return ERR_OK;
    });

    return {};
}

Value Socket::read (Value arg) {
    recvBuf = arg.isInt() ? new Array ('B', arg) : &arg.asType<Array>();
    assert(recvBuf->isBuffer());
    Interp::suspend(readQueue);

    tcp_recv(socket, [](void *arg, tcp_pcb *tpcb, pbuf *p, err_t err) -> err_t {
        auto& self = *(Socket*) arg;
        assert(self.socket == tpcb);
        if (p != 0) {
            if (self.readQueue.len() == 0)
                return ERR_BUF;
            auto n = self.recvBuf->write(p->payload, p->tot_len);
            assert(n > 0);
            tcp_recved(tpcb, n);
            pbuf_free(p);
            Interp::wakeUp(self.readQueue.pop(0), self.recvBuf);
        } else
            self.close();
        return ERR_OK;
    });

    return {};
}

bool Socket::sendIt (Value arg) {
    const void* p;
    int n;
    if (arg.isStr()) {
        p = (const char*) arg;
        n = strlen(arg);
    } else {
        auto& o = arg.asType<Bytes>();
        p = o.begin();
        n = o.size();
    }
    if (n + 50 > tcp_sndbuf(socket)) // check for some spare room, just in case
        return false;
    printf("444\n");

    auto r = tcp_write(socket, p, n, TCP_WRITE_FLAG_COPY);
    if (r != 0) { // session probably closed by peer
        printf("write err %d sndbuf %d\n", r, tcp_sndbuf(socket));
        close();
        return false;
    }

    return true;
}

Value Socket::write (Value arg) {
    assert(toSend.isNil()); // don't allow multiple outstanding sends
    if (sendIt(arg))
        return {};

    toSend = arg;
    printf("suspending write\n");
    Interp::suspend(writeQueue);

    tcp_sent(socket, [](void *arg, tcp_pcb *tpcb, uint16_t len) -> err_t {
        printf("sent %p %d\n", arg, len);
        auto& self = *(Socket*) arg;
        if (self.socket != 0) {
            assert(self.socket == tpcb);
            if (self.sendIt(self.toSend)) {
                tcp_sent(tpcb, 0);
                self.toSend = {};
                assert(self.writeQueue.len() > 0);
                Interp::tasks.append(self.writeQueue.pop(0));
            }
        }
        return ERR_OK;
    });

    return {};
}

Value Socket::close () {
    printf("\t CLOSE! %p r %d w %d\n",
            socket, (int) readQueue.len(), (int) writeQueue.len());
    if (socket != 0) {
        tcp_recv(socket, 0);
        tcp_close(socket);
        socket = 0;
    }
    toSend = {};
    if (readQueue.len() > 0)
        readQueue.pop(0); // TODO throw Interp::tasks.append(readQueue.pop(0));
    assert(readQueue.len() == 0);
    if (writeQueue.len() > 0)
        writeQueue.pop(0); // TODO throw Interp::tasks.append(writeQueue.pop(0));
    assert(writeQueue.len() == 0);
    return {};
}

static const auto d_socket_bind = Method::wrap(&Socket::bind);
static const Method m_socket_bind = d_socket_bind;

static const auto d_socket_connect = Method::wrap(&Socket::connect);
static const Method m_socket_connect = d_socket_connect;

static const auto d_socket_listen = Method::wrap(&Socket::listen);
static const Method m_socket_listen = d_socket_listen;

static const auto d_socket_read = Method::wrap(&Socket::read);
static const Method m_socket_read = d_socket_read;

static const auto d_socket_write = Method::wrap(&Socket::write);
static const Method m_socket_write = d_socket_write;

static const auto d_socket_close = Method::wrap(&Socket::close);
static const Method m_socket_close = d_socket_close;

static const Lookup::Item socketMap [] = {
    { "bind", m_socket_bind },
    { "connect", m_socket_connect },
    { "listen", m_socket_listen },
    { "read", m_socket_read },
    { "write", m_socket_write },
    { "close", m_socket_close },
};

const Lookup Socket::attrs (socketMap, sizeof socketMap);

Type const Socket::info ("<socket>", Socket::create, &Socket::attrs);

const Function fo_ifconfig = f_ifconfig;
const Function fo_poll = f_poll;

static const Lookup::Item lo_network [] = {
    { "ifconfig", fo_ifconfig },
    { "poll", fo_poll },
    { "socket", Socket::info },
};

static const Lookup ma_network (lo_network, sizeof lo_network);
const Module m_network (&ma_network);

#endif // INCLUDE_NETWORK
