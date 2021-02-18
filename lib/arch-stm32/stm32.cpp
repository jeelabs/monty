#include "monty.h"
#include "arch.h"

#include <jee.h>
#include <jee/text-ihex.h>
#include "jee-stm32.h"
#include "jee-rf69.h"

#include <cassert>
#include <unistd.h>

const auto mrfsBase = (mrfs::Info*) 0x08010000;
const auto mrfsSize = 32*1024;

#if STM32F103xB
UartBufDev< PinA<2>, PinA<3>, 100 > console;
#elif STM32L432xx
UartBufDev< PinA<2>, PinA<15>, 100 > console;
#else
UartBufDev< PinA<9>, PinA<10>, 100 > console;
#endif

static void outch (int c) {
    console.putc(c);
}

static auto outFun = outch;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(outFun, fmt, ap); va_end(ap);
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
}

struct LineSerial : Stacklet {
    Event incoming;
    //Event outgoing;
    char buf [100]; // TODO avoid hard limit for input line length
    uint32_t fill = 0;

    LineSerial () {
        rxId = incoming.regHandler();
        txId = outQueue.regHandler();
        prevIsr = console.handler();

        console.handler() = []() {
            prevIsr();
            if (console.readable())
                Stacklet::setPending(rxId);
            // TODO this triggers too often, even when there is no TX activity
            if (console.xmit.almostEmpty())
                Stacklet::setPending(txId);
        };

        outFun = writer;
    }

    ~LineSerial () {
        outFun = outch; // restore non-stacklet-aware version
        console.handler() = prevIsr;
        incoming.deregHandler();
        outQueue.deregHandler();
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
                if (!exec(buf))
                    return false;
            } else if (fill < sizeof buf - 1)
                buf[fill++] = c;
        }
        yield();
        return true;
    }

    virtual auto exec (char const*) -> bool = 0;

    static auto writeMax (uint8_t const* ptr, size_t len) -> size_t {
        size_t i = 0;
        while (i < len && console.writable())
            console.putc(ptr[i++]);
        return i;
    }

    static auto writeAll (uint8_t const* ptr, size_t len) -> size_t {
        size_t i = 0;
        while (i < len) {
            i += writeMax(ptr + i, len - i);
            outQueue.wait();
            outQueue.clear();
        }
        return len;
    }

    static void writer (int c) {
        writeAll((uint8_t const*) &c, 1);
    }

    // TODO messy use of static scope
    static void (*prevIsr)();
    static uint8_t rxId, txId;
    static Event outQueue;
};

void (*LineSerial::prevIsr)();
uint8_t LineSerial::rxId;
uint8_t LineSerial::txId;
Event LineSerial::outQueue;

struct HexSerial : LineSerial {
    static constexpr auto PERSIST_SIZE = 2048;

    enum { MagicStart = 987654321, MagicValid = 123456789 };

    auto (*reader)(char const*)->bool;
    IntelHex<32> ihex; // max 32 data bytes per hex input line

    HexSerial (auto (*fun)(char const*)->bool) : reader (fun) {
        auto cmd = bootCmd();
        magic() = 0; // only use saved boot command once
        if (cmd != nullptr)
            exec(persist+4);
    }

    static auto bootCmd () -> char const* {
        if (persist == nullptr)
            persist = (char*) sbrk(4+PERSIST_SIZE); // never freed
        return magic() == MagicValid ? persist+4 : nullptr;
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
    printf("%d ms\n", 8 * count);
}

Command const commands [] = {
    { "bc *  set boot command [cmd ...]"  , bc_cmd },
    { "bv    show build version"          , [](char*) { printBuildVer(); }},
    { "di    show device info"            , [](char*) { printDevInfo(); }},
    { "gc    trigger garbage collection"  , [](char*) { Stacklet::gcAll(); }},
    { "gr    generate a GC report"        , [](char*) { gcReport(); }},
    { "ls    list files in MRFS"          , [](char*) { mrfs::dump(); }},
    { "od    object dump"                 , [](char*) { gcObjDump(); }},
    { "pd    power down"                  , [](char*) { powerDown(); }},
    { "sr    system reset"                , [](char*) { systemReset(); }},
    { "vd    vector dump"                 , [](char*) { gcVecDump(); }},
    { "wd N  set watchdog [0..4095] x8 ms", wd_cmd },
    { "?     this help"                   , nullptr },
};

auto execCmd (char const* buf) -> bool {
    if (buf[0] == '?') {
        for (auto& cmd : commands)
            printf("  %s\n", cmd.desc);
        return true;
    }
    for (auto& cmd : commands)
        if (memcmp(buf, cmd.desc, 2) == 0) {
            cmd.proc((char*) buf); // TODO get rid of const in caller
            return true;
        }
    auto data = arch::importer(buf);
    if (data != nullptr) {
        auto p = vmLaunch(data);
        assert(p != nullptr);
        Stacklet::tasks.append(p);
    } else
        printf("<%s> ?\n", buf);
    return true;
}

auto arch::cliTask(Loader loader) -> Stacklet* {
    auto task = loader((uint8_t const*) HexSerial::bootCmd());
    if (task != nullptr) {
        HexSerial::magic() = 0; // only load saved data once
        return task;
    }
    return new HexSerial (execCmd);
}

auto arch::importer (char const* name) -> uint8_t const* {
    auto pos = mrfs::find(name);
    return pos >= 0 ? (uint8_t const*) mrfsBase[pos].name : nullptr;
}

void arch::init () {
    console.init();
#if STM32F103xB
    enableSysTick(); // no HSE crystal
#else
    console.baud(115200, fullSpeedClock());
#endif

    printf("\n"); // TODO yuck, the uart TX sends a 0xFF junk char after reset

    mrfs::init(mrfsBase, mrfsSize);
    //mrfs::dump();
}

void arch::idle () {
    asm ("wfi");
}

auto arch::done () -> int {
    HexSerial::magic() = 0; // clear boot command buffer
    wait_ms(10);
    systemReset(); // will resume the cli task with a clean slate
    return 0;
}

namespace machine {
    // machine.pins.B3 = "PL"   : set mode of PB3 to push-pull, low speed
    // machine.pins.B3 = 1      : set output pin to 1
    // if machine.pins.B3: ...  : get input pin state

    struct Pins : Object {
        static Type info;
        auto type () const -> Type const& override { return info; }

        auto attr (char const* name, Value&) const -> Value override {
            assert(strlen(name) >= 2);
            jeeh::Pin p (name[0], atoi(name + 1));
            return p.read();
        }

        auto setAt (Value arg, Value val) -> Value override {
            assert(arg.isStr() && strlen(arg) >= 2);
            jeeh::Pin p (arg[0], atoi((char const*) arg + 1));
            if (val.isInt())
                p.write(val);
            else {
                assert(val.isStr());
                if (!p.mode((char const*) val))
                    return {E::ValueError, "invalid pin mode", val};
            }
            return {};
        }
    };

    Type Pins::info ("<machine.pins>");

    static Pins pins; // there is one static pins object, used via attr access

    // spi = machine.spi("A4,A5,A6,A7")
    // spi.enable()
    // x = spi.transfer(123)
    // spi.disable()

    struct Spi : Object, jeeh::SpiGpio {
        static Lookup const attrs;
        static Type info;
        auto type () const -> Type const& override { return info; }

        auto xfer (Value v) -> Value {
            assert(v.isInt());
            return transfer(v);
        }
    };

    static auto d_spi_enable = Method::wrap(&Spi::enable);
    static Method const m_spi_enable (d_spi_enable);

    static auto d_spi_disable = Method::wrap(&Spi::disable);
    static Method const m_spi_disable (d_spi_disable);

    static auto d_spi_transfer = Method::wrap(&Spi::xfer);
    static Method const m_spi_transfer (d_spi_transfer);

    Lookup::Item const spiMap [] = {
        { "enable", m_spi_enable },
        { "disable", m_spi_disable },
        { "transfer", m_spi_transfer },
    };

    Lookup const Spi::attrs (spiMap, sizeof spiMap);

    Type Spi::info ("<machine.spi>", nullptr, &Spi::attrs);

    auto f_spi (ArgVec const& args) -> Value {
        assert(args.num == 1);
        auto spi = new Spi;
        auto err = jeeh::Pin::define(args[0], &spi->_mosi, 4);
        if (err != nullptr || !spi->isValid())
            return {E::ValueError, "invalid SPI pin", err};
        spi->init();
        return spi;
    }

    struct RF69 : Object, jeeh::RF69<jeeh::SpiGpio> {
        static Lookup const attrs;
        static Type info;
        auto type () const -> Type const& override { return info; }

        auto recv (ArgVec const& args) -> Value {
            assert(args.num == 2);
            auto& a = args[1].asType<Array>();
            assert(a.size() >= 4);
            auto r = receive(a.begin()+4, a.size()-4);
            a[0] = rssi;
            a[1] = lna;
            a[2] = afc;
            a[3] = afc >> 8;
            return r+4;
        }

        auto xmit (ArgVec const& args) -> Value {
            assert(args.num == 2 && args[0].isInt());
            auto& a = args[1].asType<Array>();
            send(args[0], a.begin(), a.size());
            return {};
        }
    };

    static auto d_rf69_receive = Method::wrap(&RF69::recv);
    static Method const m_rf69_receive (d_rf69_receive);

    static auto d_rf69_send = Method::wrap(&RF69::xmit);
    static Method const m_rf69_send (d_rf69_send);

    static auto d_rf69_sleep = Method::wrap(&RF69::sleep);
    static Method const m_rf69_sleep (d_rf69_sleep);

    Lookup::Item const rf69Map [] = {
        { "receive", m_rf69_receive },
        { "send", m_rf69_send },
        { "sleep", m_rf69_sleep },
    };

    Lookup const RF69::attrs (rf69Map, sizeof rf69Map);

    Type RF69::info ("<machine.rf69>", nullptr, &RF69::attrs);

    auto f_rf69 (ArgVec const& args) -> Value {
        assert(args.num == 4);
        assert(args[1].isInt() && args[2].isInt() && args[3].isInt());
        auto rf69 = new RF69;
        auto err = jeeh::Pin::define(args[0], &rf69->spi._mosi, 4);
        if (err != nullptr || !rf69->spi.isValid())
            return {E::ValueError, "invalid SPI pin", err};
        rf69->spi.init();
        rf69->init(args[1], args[2], args[3]);
        return rf69;
    }

    Event tickEvent;
    int ms, tickerId;
    uint32_t start, last;

    auto msNow () -> Value {
        uint32_t t = ticks;
        static uint32_t begin;
        if (begin == 0)
            begin = t;
        return t - begin; // make all runs start out the same way
    }

    auto f_ticker (ArgVec const& args) -> Value {
        if (args.num > 0) {
            assert(args.num == 1 && args[0].isInt());
            ms = args[0];
            start = msNow(); // set first timeout relative to now
            last = 0;
            tickerId = tickEvent.regHandler();
            assert(tickerId > 0);

            VTableRam().systick = []() {
                ++ticks;
                uint32_t t = msNow(); // TODO messy
                if (ms > 0 && (t - start) / ms != last) {
                    last = (t - start) / ms;
                    Stacklet::setPending(tickerId);
                }
            };
        } else {
            VTableRam().systick = []() {
                ++ticks;
            };

            tickEvent.deregHandler();
            assert(tickerId > 0);
        }
        return tickEvent;
    }

    auto f_ticks (ArgVec const&) -> Value {
        return msNow();
    }

    auto f_dog (ArgVec const& args) -> Value {
        int count = 4095;
        if (args.num > 0 && args[0].isInt())
            count = args[0];

        static Iwdg dog;
        dog.reload(count);
        return {};
    }

    auto f_kick (ArgVec const& args) -> Value {
        assert(args.num == 0);
        Iwdg::kick();
        return {};
    }

    Function const fo_spi (f_spi);
    Function const fo_rf69 (f_rf69);
    Function const fo_ticker (f_ticker);
    Function const fo_ticks (f_ticks);
    Function const fo_dog (f_dog);
    Function const fo_kick (f_kick);

    Lookup::Item const map [] = {
        { "pins", pins },
        { "spi", fo_spi },
        { "rf69", fo_rf69 },
        { "ticker", fo_ticker },
        { "ticks", fo_ticks },
        { "dog", fo_dog },
        { "kick", fo_kick },
    };
}

extern Lookup const machine_attrs (machine::map, sizeof machine::map);

#ifdef UNIT_TEST
extern "C" {
    void unittest_uart_begin () { arch::init(); wait_ms(100); }
    void unittest_uart_putchar (char c) { console.putc(c); }
    void unittest_uart_flush () { while (!console.xmit.empty()) {} }
    void unittest_uart_end () { while (true) {} }
}
#endif
