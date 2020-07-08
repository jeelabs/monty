#include "monty.h"
#include "arch.h"

#include <jee.h>

#if BOARD_discovery_f4 || STM32L412xx
UartBufDev< PinA<2>, PinA<3> > console;
#else
UartBufDev< PinA<9>, PinA<10> > console;
#endif

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

void archInit () {
    console.init();
#if BOARD_discovery_f4
    console.baud(115200, fullSpeedClock() / 4);
#else
    console.baud(115200, fullSpeedClock());
#endif
    wait_ms(10);
}

int archDone () {
    while (true) {}
}

static Value bi_blah (int argc, Value argv []) {
    return argc;
}

static const FunObj f_blah (bi_blah);

static int ms, id;
static uint32_t start, begin, last = ~0;

void timerHook () {
    uint32_t t = ticks;
    if (ms > 0 && (t - start) / ms != last) {
        last = (t - start) / ms;
        if (id > 0)
            Context::raise(id);
    }
}

// interface exposed to the VM

Value f_timer (int argc, Value argv []) {
    if (argc != 2 || !argv[0].isInt())
        return -1;
    ms = argv[0];
    id = Context::setHandler(ms > 0 ? argv[1] : (Value) id);
    start = ticks; // set first timeout relative to now
    last = 0;
    VTableRam().systick = timerHook;
    return id;
}

Value f_ticks (int argc, Value argv []) {
    uint32_t t = ticks;
    if (begin == 0)
        begin = t;
    return t - begin; // make all runs start out the same way
}
static const FunObj fo_timer (f_timer);
static const FunObj fo_ticks (f_ticks);

static const LookupObj::Item lo_machine [] = {
    { "blah", &f_blah },
    { "timer", &fo_timer },
    { "ticks", &fo_ticks },
};

static const LookupObj ma_machine (lo_machine, sizeof lo_machine / sizeof *lo_machine);
const ModuleObj m_machine (&ma_machine);