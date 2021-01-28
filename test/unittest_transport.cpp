#if !NATIVE

#include "unittest_transport.h"
#include <jee.h>

// FIXME lots of duplication with the code lib/arch-stm32/stm32.cpp

extern // <== !!! console is in lib/arch-*/... which is (always?) pulled in
#if STM32F103xB
UartBufDev< PinA<2>, PinA<3> > console;
#elif STM32L432xx
UartBufDev< PinA<2>, PinA<15> > console;
#else
UartBufDev< PinA<9>, PinA<10> > console;
#endif

void unittest_uart_begin () {
    console.init();
#if STM32F103xB
    enableSysTick(); // no HSE crystal
#else
    console.baud(115200, fullSpeedClock());
#endif
    wait_ms(200);
}

void unittest_uart_putchar (char c) {
    console.putc(c);
}

void unittest_uart_flush () {
    while (!console.xmit.empty()) {}
}

void unittest_uart_end () {}

#endif
