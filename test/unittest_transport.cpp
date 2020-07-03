#if !NATIVE

#include "unittest_transport.h"
#include <jee.h>

#if STM32F1
UartBufDev< PinA<9>, PinA<10>, 2, 99 > console; // Blue Pill
#elif STM32F4
UartBufDev< PinA<2>, PinA<3>, 2 > console;      // F4 Discovery
#else
UartBufDev< PinA<2>, PinA<3> > console;         // ESP32 TinyPico
#endif

void unittest_uart_begin () {
    console.init();
#if STM32F1
    console.baud(115200, fullSpeedClock());     // Blue Pill
    (void) powerDown; (void) enableClkAt8MHz;   // suppress warnings
#else
    console.baud(115200, fullSpeedClock()/4);   // F4 Discovery
#endif
    wait_ms(100);
}

void unittest_uart_putchar (char c) {
    console.putc(c);
}

void unittest_uart_flush () {}

void unittest_uart_end () {}

#endif
