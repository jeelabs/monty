#include <monty.h>
#include "arch.h"

#include <jee.h>
#include <jee/text-ihex.h>

#include <cassert>
#include <unistd.h>

#if STM32F103xB
UartBufDev< PinA<2>, PinA<3> > console;
#elif STM32L432xx
UartBufDev< PinA<2>, PinA<15> > console;
#else
UartBufDev< PinA<9>, PinA<10> > console;
#endif

PinB<3> led;

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
    while (true) {} // comply with abort's "noreturn" attribute
}

extern "C" void __assert_func (char const* f, int l, char const* n, char const* e) {
    printf("\nassert(%s) in %s\n\t%s:%d\n", e, n, f, l);
    abort();
}

extern "C" void __assert (char const* f, int l, char const* e) {
    __assert_func(f, l, "-", e);
}

using namespace monty;

void printDeviceInfo () {
    printf("\b \n"); // FIXME hides invalid char sent after a reset (why?)
#ifndef NDEBUG
    extern int g_pfnVectors [], _sidata [], _sdata [], _ebss [], _estack [];
    printf("flash 0x%p..0x%p, ram 0x%p..0x%p, stack top 0x%p\n",
            g_pfnVectors, _sidata, _sdata, _ebss, _estack);

#if STM32F1
    // the 0x1F... addresses are cpu-family specific
    printf("cpuid 0x%p, %d kB flash, %d kB ram, package type %d\n",
            MMIO32(0xE000ED00),
            MMIO16(0x1FFFF7E0),
            (_estack - _sdata) >> 8,
            MMIO32(0x1FFFF700) & 0x1F); // FIXME wrong!
    printf("clock %d kHz, devid %p-%p-%p\n",
            MMIO32(0xE000E014) + 1,
            MMIO32(0x1FFFF7E8),
            MMIO32(0x1FFFF7EC),
            MMIO32(0x1FFFF7F0));
#elif STM32L4
    // the 0x1F... addresses are cpu-family specific
    printf("cpuid 0x%p, %d kB flash, %d kB ram, package type %d\n",
            MMIO32(0xE000ED00),
            MMIO16(0x1FFF75E0),
            (_estack - _sdata) >> 8,
            MMIO32(0x1FFF7500) & 0x1F);
    printf("clock %d kHz, devid %p-%p-%p\n",
            MMIO32(0xE000E014) + 1,
            MMIO32(0x1FFF7590),
            MMIO32(0x1FFF7594),
            MMIO32(0x1FFF7598));
#endif
#endif // NDEBUG
}

struct LineSerial : Stacklet {
    Event incoming;
    char buf [100]; // TODO avoid hard limit for input line length
    uint32_t fill = 0;

    LineSerial () {
        irqId = incoming.regHandler();
        prevIsr = console.handler();

        console.handler() = []() {
            prevIsr();
            if (console.readable())
                Stacklet::setPending(irqId);
        };
    }

    ~LineSerial () {
        console.handler() = prevIsr;
        incoming.deregHandler();
    }

    auto run () -> bool override {
        incoming.wait();
        incoming.clear();
        while (console.readable()) {
            auto c = console.getc();
            if (c == '\r' || c == '\n') {
                if (fill == 0)
                    continue;
                buf[fill] = 0;
                fill = 0;
                if (!exec(buf)) {
                    incoming.deregHandler(); // TODO also called in destructor
                    return false;
                }
            } else if (fill < sizeof buf - 1)
                buf[fill++] = c;
        }
        return true;
    }

    virtual auto exec (char const*) -> bool = 0;

    // these are static because the replacement (static) console ISR uses them
    static void (*prevIsr)();
    static uint32_t irqId;
};

void (*LineSerial::prevIsr)();
uint32_t LineSerial::irqId;

struct HexSerial : LineSerial {
    static constexpr auto PERSIST_SIZE = 2048;

    auto (*reader)(char const*)->bool;
    IntelHex<32> ihex; // max 32 data bytes per hex input line

    HexSerial (auto (*fun)(char const*)->bool) : reader (fun) {
        if (persist == nullptr)
            persist = (char*) sbrk(4+PERSIST_SIZE); // never freed

        if (magic() == 123456789)
            exec(persist+4);
    }

    auto magic () const -> uint32_t& {
        assert(persist != nullptr);
        return *(uint32_t*) persist;
    }

    auto exec (char const* cmd) -> bool override {
        if (cmd[0] != ':')
            return reader(cmd);
        // it's Intel hex: store valid data in persist buf, sys reset when done
        ihex.init();
        while (!ihex.parse(*++cmd)) {}
        if (ihex.check == 0 && ihex.type <= 1 &&
                                ihex.addr + ihex.len <= PERSIST_SIZE) {
            if (ihex.addr == 0)
                magic() = 123456789;
            if (ihex.type == 0)
                memcpy(persist + 4 + ihex.addr, ihex.data, ihex.len);
            else
                systemReset();
        } else {
            printf("ihex? %d t%d @%04x #%d\n",
                    ihex.check, ihex.type, ihex.addr, ihex.len);
            magic() = 0; // mark persistent buffer as invalid
        }
        return true;
    }

    static char* persist; // pointer to buffer which persists across resets
};

char* HexSerial::persist;

auto arch::cliTask(auto(*fun)(char const*)->bool) -> Stacklet* {
    return new HexSerial (fun);
}

void arch::init () {
    console.init();
#if STM32F103xB
    enableSysTick(); // no HSE crystal
#else
    console.baud(115200, fullSpeedClock());
#endif
    led.mode(Pinmode::out);

    printDeviceInfo();
}

void arch::idle () {
    asm ("wfi");
}

auto arch::done () -> int {
    while (true) {}
}

#ifdef UNIT_TEST

extern "C" void unittest_uart_begin () {
    arch::init();
    wait_ms(100);
}

extern "C" void unittest_uart_putchar (char c) {
    console.putc(c);
}

extern "C" void unittest_uart_flush () {
    while (!console.xmit.empty()) {}
}

extern "C" void unittest_uart_end () {
    arch::done();
}

#endif
