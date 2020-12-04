#ifndef UNITEST_TRANSPORT_H
#define UNITEST_TRANSPORT_H

extern "C" void unittest_uart_begin();
extern "C" void unittest_uart_putchar(char c);
extern "C" void unittest_uart_flush();
extern "C" void unittest_uart_end();

#endif 
