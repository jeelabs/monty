#include <monty.h>
#include "arch.h"

#include <jee.h>
#include <jee/text-ihex.h>

#include <cassert>
#include <unistd.h>

#if STM32F103xB
UartBufDev< PinA<2>, PinA<3>, 100 > console;
#elif STM32L432xx
UartBufDev< PinA<2>, PinA<15>, 100 > console;
#else
UartBufDev< PinA<9>, PinA<10>, 100 > console;
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
    wait_ms(250);
    arch::done();
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

void printBuildVer () {
    printf("Monty " VERSION " (" __DATE__ ", " __TIME__ ")\n");
}

void printDevInfo () {
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
        yield();
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

    enum { MagicStart = 987654321, MagicValid = 123456789 };

    auto (*reader)(char const*)->bool;
    IntelHex<32> ihex; // max 32 data bytes per hex input line

    HexSerial (auto (*fun)(char const*)->bool) : reader (fun) {
        if (persist == nullptr)
            persist = (char*) sbrk(4+PERSIST_SIZE); // never freed

        if (magic() == MagicValid)
            exec(persist+4);
    }

    static auto magic () -> uint32_t& { return *(uint32_t*) persist; }

    auto exec (char const* cmd) -> bool override {
        if (cmd[0] != ':')
            return reader(cmd);
        // Intel hex: store valid data in persist buf, sys reset when done
        ihex.init();
        while (!ihex.parse(*++cmd)) {}
        if (ihex.check == 0 && ihex.type <= 1 &&
                                ihex.addr + ihex.len <= PERSIST_SIZE) {
            if (ihex.type == 0) {
                if (ihex.addr == 0)
                    magic() = MagicStart;
                memcpy(persist + 4 + ihex.addr, ihex.data, ihex.len);
            } else {
                if (magic() == MagicStart)
                    magic() = MagicValid;
                systemReset();
            }
        } else {
            printf("ihex? %d t%d @%04x #%d\n",
                    ihex.check, ihex.type, ihex.addr, ihex.len);
            *persist = 0; // mark persistent buffer as invalid
        }
        return true;
    }

    static char* persist; // pointer to a buffer which persists across resets
};

char* HexSerial::persist;

struct Command {
    char const* desc;
    void (*proc)(char*);
};

void bc_cmd (char* cmd) {
    if (strlen(cmd) > 3) {
        HexSerial::magic() = HexSerial::MagicValid;
        strcpy(HexSerial::persist + 4, cmd + 3);
    } else
        HexSerial::magic() = 0;
}

void wd_cmd (char* cmd) {
    int count = 4095;
    if (strlen(cmd) > 3)
        count = atoi(cmd+3);

    static Iwdg dog;
    dog.reload(count);
}

void help_cmd (char*);

Command const commands [] = {
    { "bc *  set boot command (cmd ...)"  , bc_cmd },
    { "bv    show build version"          , [](char*) { printBuildVer(); }},
    { "di    show device info"            , [](char*) { printDevInfo(); }},
//  { "gc    trigger garbage collection"  , [](char*) { ... }},
    { "gr    generate a GC report"        , [](char*) { gcReport(); }},
    { "od    object dump"                 , [](char*) { gcObjDump(); }},
    { "ps    print stacklet list"         , [](char*) { Stacklet::dump(); }},
    { "sr    system reset"                , [](char*) { systemReset(); }},
    { "wd N  set watchdog count (0..4095)", wd_cmd },
    { "?     this help"                   , help_cmd },
};

void help_cmd (char*) {
    for (auto& cmd : commands)
        printf("  %s\n", cmd.desc);
}

static auto (*shFun)(char const*) -> bool;

auto execCmd (char const* buf) -> bool {
    if (buf[0] == '?') {
        help_cmd(nullptr);
        return true;
    }
    for (auto& cmd : commands)
        if (memcmp(buf, cmd.desc, 2) == 0) {
            cmd.proc((char*) buf); // TODO get rid of const in caller
            return true;
        }
    return shFun(buf);
}

auto arch::cliTask(auto(*fun)(char const*)->bool) -> Stacklet* {
    shFun = fun; // TODO yuck
    return new HexSerial (execCmd);
}

void arch::init () {
    console.init();
#if STM32F103xB
    enableSysTick(); // no HSE crystal
#else
    console.baud(115200, fullSpeedClock());
#endif
    led.mode(Pinmode::out);

    printf("\n"); // TODO yuck, the uart TX sends a 0xFF junk char after reset
}

void arch::idle () {
    asm ("wfi");
}

auto arch::done () -> int {
    HexSerial::magic() = 0; // clear boot command buffer
    systemReset(); // will resume the cli task with a clean slate
    return 0;
}

namespace machine {
    static Event tickEvent;
    static int ms, tickerId;
    static uint32_t start, last;

    Value f_ticker (ArgVec const& args) {
        if (args.num > 0) {
            assert(args.num == 1 && args[0].isInt());
            ms = args[0];
            start = ticks; // set first timeout relative to now
            last = 0;
            tickerId = tickEvent.regHandler();

            VTableRam().systick = []() {
                uint32_t t = ++ticks;
                if (ms > 0 && (t - start) / ms != last) {
                    last = (t - start) / ms;
                    exception(tickerId);
                }
            };
        } else {
            VTableRam().systick = []() {
                ++ticks;
            };

            tickEvent.deregHandler();
        }
        return tickEvent;
    }

    static Function const fo_ticker (f_ticker);

    Value f_ticks (ArgVec const&) {
        uint32_t t = ticks;
        static uint32_t begin;
        if (begin == 0)
            begin = t;
        return t - begin; // make all runs start out the same way
    }

    static Function const fo_ticks (f_ticks);

    static Lookup::Item const attrs [] = {
        { "ticker", fo_ticker },
        { "ticks", fo_ticks },
    };
}

static Lookup const ma_machine (machine::attrs, sizeof machine::attrs);
extern Module const m_machine (ma_machine);

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
    while (true) {}
}

#endif
