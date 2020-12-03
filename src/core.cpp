#include <jee.h>

extern UartBufDev< PinA<2>, PinA<15> > console;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
    return 0;
}

int main () {
    console.init();
    printf("hello from %s\n", "core");
}
