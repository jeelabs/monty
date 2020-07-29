#if !NATIVE

#include "config.h"

#include "unittest_transport.h"
#include <jee.h>

UartBufDev< PINS_CONSOLE > console;

void unittest_uart_begin () {
    console.init();
    console.baud(115200, fullSpeedClock() / UART_BUSDIV);
    wait_ms(100);
}

void unittest_uart_putchar (char c) {
    console.putc(c);
}

void unittest_uart_flush () {
    while (!console.xmit.empty()) {}
}

void unittest_uart_end () {}

#endif
