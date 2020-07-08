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

static const LookupObj::Item lo_machine [] = {
    { "blah", &f_blah },
};

static const LookupObj ma_machine (lo_machine, sizeof lo_machine / sizeof *lo_machine);
const ModuleObj m_machine (&ma_machine);
