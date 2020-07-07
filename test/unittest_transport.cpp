#if !NATIVE

#include "unittest_transport.h"
#include <jee.h>

#if BOARD_discovery_f4 || STM32L412xx
UartBufDev< PinA<2>, PinA<3> > console;
#else
UartBufDev< PinA<9>, PinA<10> > console; // Blue Pill
#endif

void unittest_uart_begin () {
    console.init();
#if BOARD_discovery_f4
    console.baud(115200, fullSpeedClock()/4);
#else
    console.baud(115200, fullSpeedClock());
#endif
    wait_ms(500);
}

void unittest_uart_putchar (char c) {
    console.putc(c);
}

void unittest_uart_flush () {}

void unittest_uart_end () {}

#endif
