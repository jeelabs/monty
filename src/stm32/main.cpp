#include <jee.h>
#include <cassert>
#include <cstdlib>
#include <mrfs.h>
#include "monty.h"
#include "pyvm.h"

UartDev< PinA<2>, PinA<15> > console;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
    return 0;
}

extern "C" int puts (char const* s) { return printf("%s\n", s); }
extern "C" int putchar (int ch) { return printf("%c", ch); }

void systemReset () {
    // ARM Cortex specific
    MMIO32(0xE000ED0C) = (0x5FA<<16) | (1<<2); // SCB AIRCR reset
    while (true) {}
}

extern "C" void abort () {
    printf("\nabort\n");
    wait_ms(5000);
    systemReset();
    while (true) {}
}

extern "C" void __assert_func (char const* f, int l, char const* n, char const* e) {
    printf("\nassert(%s) in %s\n\t%s:%d\n", e, n, f, l);
    abort();
}

extern "C" void __assert (char const* f, int l, char const* e) {
    __assert_func(f, l, "-", e);
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

using namespace monty;

static void runInterp (monty::Callable& init) {
    monty::PyVM vm (init);

    printf("Running ...\n");
    while (vm.isAlive()) {
        vm.scheduler();
        //XXX archIdle();
    }
    printf("Stopped.\n");
}

char mem [10000];

int main () {
    console.init();
    //enableSysTick();
    console.baud(115200, fullSpeedClock());
    wait_ms(500);

    printf("\r<RESET>\n");
    //echoBlinkCheck();
    //printDeviceInfo();

    // STM32L432-specific
    //mrfs::init((void*) 0x08000000, 256*1024, 10*1024); // romend 0x2800
    mrfs::init((void*) 0x08000000, 256*1024, 44*1024); // romend 0xB000
    //mrfs::dump();
    auto pos = mrfs::find("hello");
    printf("hello @ %d\n", pos);
    assert(pos > 0);

    printf("hello from %s\n", "core");

    extern uint8_t g_pfnVectors [], _sidata [], _sdata [], _ebss [], _estack [];
    printf("  flash %p..%p, ram %p..%p, stack top %p\n",
            g_pfnVectors, _sidata, _sdata, _ebss, _estack);

    monty::setup(mem, sizeof mem);
    //monty::gcReport();

    monty::Bytecode* bc = nullptr;
    auto mod = new monty::Module (monty::builtins);
    monty::Callable dummy (*bc, mod);
    runInterp(dummy);

    monty::gcNow();
    monty::gcReport();

    printf("\r<EXIT>\n");
    while (true) {}
}
