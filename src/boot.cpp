#include <jee.h>
#include "segment.h"

extern UartBufDev< PinA<2>, PinA<15> > console;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
    return 0;
}

PinB<3> led;

void echoBlinkCheck () {
    led.mode(Pinmode::out);
    while (true) {
        led.toggle();
        auto ch = console.getc();
        console.putc(ch);
        if (ch < ' ') // any control char
            break;
    }
    console.putc('\n');
}

void printDeviceInfo () {
    // the 0x1F... addresses are cpu-family specific
    printf("  cpuid 0x%p, %d kB flash, package type %d\n",
            (void*) MMIO32(0xE000ED00),
            MMIO16(0x1FFF75E0),
            (int) MMIO32(0x1FFF7500) & 0x1F);
    printf("  devid 0x%p-%p-%p, clock %d kHz\n",
            (void*) MMIO32(0x1FFF7590),
            (void*) MMIO32(0x1FFF7594),
            (void*) MMIO32(0x1FFF7598),
            (int) MMIO32(0xE000E014) + 1);
}

void printMemoryRanges () {
    extern uint8_t g_pfnVectors[], _sidata [], _sdata [], _ebss [], _estack [];
    printf("  flash %p..%p, ram %p..%p, stack top %p\n",
            g_pfnVectors, _sidata-1, _sdata, _ebss-1, _estack);
    auto romSize = _sidata - g_pfnVectors;
    auto romAlign = romSize + (-romSize & (flashSegSize-1));
    auto romNext = g_pfnVectors + romAlign;
    auto ramSize = _ebss - _sdata;
    auto ramAlign = ramSize + (-ramSize & 7);
    auto ramNext = _sdata + ramAlign;
    printf("  flash align %db => %db, ram align %db => %db, ram free %db\n",
            romSize, romAlign, ramSize, ramAlign, _estack - ramNext);
    printf("  flash next %p, ram next %p\n", romNext, ramNext);
}

int main () {
    console.init();

    printf("\r<RESET>\n");
    //echoBlinkCheck();
    printDeviceInfo();
    //printMemoryRanges();

    auto hdr = nextSegment();
    if (hdr.magic == 0x12345678) {
        printf("  main -> regFun %p\n", hdr.regFun);
        hdr.regFun();
        printf("  main -> deregFun %p\n", hdr.deregFun);
        hdr.deregFun();
    }

    printf("\r<EXIT>\n");
    while (true) {}
}
