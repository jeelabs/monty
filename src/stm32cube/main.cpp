#include <assert.h>
#include <string.h>

#include "monty.h"
#include "arch.h"
#include "defs.h"
#include "qstr.h"
#include "builtin.h"
#include "interp.h"
#include "loader.h"

#include <jee.h>

#if BOARD_discovery_f4
UartDev< PinA<2>, PinA<3> > console;
#else
UartDev< PinA<9>, PinA<10> > console;
#endif

int printf (const char* fmt, ...) {
    va_list ap; va_start(ap, fmt);
    veprintf(console.putc, fmt, ap); va_end(ap);
    return 0;
}

extern "C" int debugf (const char* fmt, ...) {
    va_list ap; va_start(ap, fmt);
    veprintf(console.putc, fmt, ap); va_end(ap);
    return 0;
}

static bool runInterp (const uint8_t* data) {
    Interp vm;

    ModuleObj* mainMod = 0;
    if (data[0] == 'M' && data[1] == 5) {
        Loader loader;
        mainMod = loader.load (data);
        vm.qPool = loader.qPool;
    }

    if (mainMod == 0)
        return false;

    mainMod->chain = &builtinDict;
    mainMod->atKey("__name__", DictObj::Set) = "__main__";
    mainMod->call(0, 0);

    vm.run();

    // must be placed here, before the vm destructor is called
    Object::gcStats();
    Context::gcTrigger();
    return true;
}

#include "net.h"

static void testNet () {
    mch_net_init();
    printf("Setup completed\n");

    auto pcb = tcp_new(); assert(pcb != NULL);
    auto r = tcp_bind(pcb, IP_ADDR_ANY, 1234); assert(r == 0);
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

#if 0
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
#endif

    while (true) {
        mch_net_poll();
        sys_check_timeouts();
    }
}

int main () {
    console.init();
#if BOARD_discovery_f4
    console.baud(115200, fullSpeedClock() / 4);
#else
    console.baud(115200, fullSpeedClock());
#endif
    wait_ms(500);

    printf("\xFF" // send out special marker for easier remote output capture
           "main qstr #%d %db %s\n",
            (int) qstrNext, (int) sizeof qstrData, VERSION);

    //testNet();

    auto bcData = (const uint8_t*) 0x20004000;
    if (!runInterp(bcData))
        printf("can't load bytecode\n");

    printf("done\n");
    while (true) {}
}
