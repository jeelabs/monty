#include "monty.h"
#include "arch.h"

#include <jee.h>

UartBufDev< PINS_CONSOLE > console;

int printf (const char* fmt, ...) {
    va_list ap; va_start(ap, fmt);
    veprintf(console.putc, fmt, ap); va_end(ap);
    return 0;
}

int debugf (const char* fmt, ...) {
    va_list ap; va_start(ap, fmt);
    veprintf(console.putc, fmt, ap); va_end(ap);
    return 0;
}

extern "C" void __assert_func (const char *f, int l, const char * n, const char *e) {
    printf("assert(%s) in %s\n\t%s:%d\n", e, n, f, l);
    while (true) {}
}

extern "C" void __assert (const char *f, int l, const char *e) {
    __assert_func(f, l, "-", e);
}

void archInit () {
    console.init();
    console.baud(115200, fullSpeedClock() / UART_BUSDIV);
    wait_ms(100);
}

int archDone () {
    Object::gcStats();
    //while (!console.xmit.empty()) {}
    while (true) {}
}

static Value bi_blah (int argc, Value argv []) {
    return argc;
}

static const FunObj f_blah (bi_blah);

static int ms, id;
static uint32_t start, begin, last;

// interface exposed to the VM

Value f_ticker (int argc, Value argv []) {
    Value h = id;
    if (argc > 1) {
        if (argc != 3 || !argv[1].isInt())
            return -1;
        ms = argv[1];
        h = argv[2];
        start = ticks; // set first timeout relative to now
        last = 0;
        VTableRam().systick = []() {
            uint32_t t = ++ticks;
            if (ms > 0 && (t - start) / ms != last) {
                last = (t - start) / ms;
                if (id > 0)
                    Context::raise(id);
            }
        };
    }
    id = Context::setHandler(h);
    return id;
}

Value f_ticks (int argc, Value argv []) {
    uint32_t t = ticks;
    if (begin == 0)
        begin = t;
    return t - begin; // make all runs start out the same way
}

static const FunObj fo_ticker (f_ticker);
static const FunObj fo_ticks (f_ticks);

static const LookupObj::Item lo_machine [] = {
    { "blah", &f_blah },
    { "ticker", &fo_ticker },
    { "ticks", &fo_ticks },
};

static const LookupObj ma_machine (lo_machine, sizeof lo_machine / sizeof *lo_machine);
const ModuleObj m_machine (&ma_machine);
