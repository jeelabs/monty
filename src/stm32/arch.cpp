#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "monty.h"
#include "arch.h"

#include <jee.h>

using namespace Monty;

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
    //Object::gcStats();
    //while (!console.xmit.empty()) {}
    while (true) {}
}

static Value bi_blah (Context& ctx, int argc, int args) {
    return argc;
}

static const Function f_blah (bi_blah);

static int ms, id;
static uint32_t start, begin, last;

// interface exposed to the VM

Value f_ticker (Context& ctx, int argc, int args) {
    Value h = id;
    if (argc > 1) {
        if (argc != 3 || !ctx[args+1].isInt())
            return -1;
        ms = ctx[args+1];
        h = ctx[args+2];
        start = ticks; // set first timeout relative to now
        last = 0;
        VTableRam().systick = []() {
            uint32_t t = ++ticks;
            if (ms > 0 && (t - start) / ms != last) {
                last = (t - start) / ms;
                if (id > 0)
                    Context::interrupt(id);
            }
        };
    }
    //id = Context::setHandler(h);
    return id;
}

Value f_ticks (Context& ctx, int argc, int args) {
    uint32_t t = ticks;
    if (begin == 0)
        begin = t;
    return t - begin; // make all runs start out the same way
}

static const Function fo_ticker (f_ticker);
static const Function fo_ticks (f_ticks);

static const Lookup::Item lo_machine [] = {
    { "blah", &f_blah },
    { "ticker", &fo_ticker },
    { "ticks", &fo_ticks },
};

static const Lookup ma_machine (lo_machine, sizeof lo_machine);
const Module m_machine (&ma_machine);
