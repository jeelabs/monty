#include "monty.h"
#include "arch.h"

#include <jee.h>
#include <jee/text-ihex.h>

#include <cassert>
#include <unistd.h>

using namespace monty;

const auto mrfsBase = (mrfs::Info*) 0x08020000;
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
        if (ihex.check == 0 && ihex.type <= 2 &&
                                ihex.addr + ihex.len <= PERSIST_SIZE) {
            switch (ihex.type) {
                case 0: // data bytes
                        if (ihex.addr == 0)
                            magic() = MagicStart;
                        memcpy(persist + 4 + ihex.addr, ihex.data, ihex.len);
                        last = ihex.addr + ihex.len;
                        break;
                case 2: // flash offset
                        offset = (uint32_t) mrfsBase +
                                    (ihex.data[0]<<12) + (ihex.data[1]<<4);
                        break;
                case 1: // end of hex input
                        if (magic() == MagicStart) {
                            if (offset != 0) {
                                saveToFlash(offset, persist+4, last);
                                printf("offset %x last %d\n", offset, last);
                                *persist = 0;
                                offset = 0;
                                break;
                            }
                            magic() = MagicValid;
                        }
                        // fall through
                default: systemReset();
            }
        } else {
            printf("ihex? %d t%d @%04x #%d\n",
                    ihex.check, ihex.type, ihex.addr, ihex.len);
            *persist = 0; // mark persistent buffer as invalid
        }
        return true;
    }

    void saveToFlash (uint32_t off, void const* buf, int len) {
        if (off % 2048 == 0) // TODO STM32L4-specific
            Flash::erasePage((void*) off);
        auto words = (uint32_t const*) buf;
        for (int i = 0; i < len; i += 8)
            Flash::write64((void const*) (off+i), words[i/4], words[i/4+1]);
        Flash::finish();
    }

    uint32_t offset = 0, last = 0;
    static char* persist; // pointer to a buffer which persists across resets
};

char* HexSerial::persist;

struct Command {
    char const* desc;
    void (*proc)(char const*);
};

void bc_cmd (char const* cmd) {
    if (strlen(cmd) > 3) {
        HexSerial::magic() = HexSerial::MagicValid;
        strcpy(HexSerial::persist + 4, cmd + 3);
    } else
        HexSerial::magic() = 0;
}

void wd_cmd (char const* cmd) {
    int count = 4095;
    if (strlen(cmd) > 3)
        count = atoi(cmd+3);

    static Iwdg dog;
    dog.reload(count);
    printf("%d ms\n", 8 * count);
}

Command const commands [] = {
    { "bc *  set boot command [cmd ...]"  , bc_cmd },
    { "bv    show build version"          , [](char const*) { printBuildVer(); }},
    { "di    show device info"            , [](char const*) { printDevInfo(); }},
    { "gc    trigger garbage collection"  , [](char const*) { Stacklet::gcAll(); }},
    { "gr    generate a GC report"        , [](char const*) { gcReport(); }},
    { "ls    list files in MRFS"          , [](char const*) { mrfs::dump(); }},
    { "od    object dump"                 , [](char const*) { gcObjDump(); }},
    { "pd    power down"                  , [](char const*) { powerDown(); }},
    { "sr    system reset"                , [](char const*) { systemReset(); }},
    { "vd    vector dump"                 , [](char const*) { gcVecDump(); }},
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
            cmd.proc(buf);
            return true;
        }
    auto data = vmImport(buf);
    if (data != nullptr) {
        auto p = vmLaunch(data);
        assert(p != nullptr);
        Stacklet::tasks.append(p);
    } else
        printf("<%s> ?\n", buf);
    return true;
}

auto arch::cliTask() -> Stacklet* {
    auto task = vmLaunch((uint8_t const*) HexSerial::bootCmd());
    if (task != nullptr) {
        HexSerial::magic() = 0; // only load saved data once
        return task;
    }
    return new HexSerial (execCmd);
}

auto monty::vmImport (char const* name) -> uint8_t const* {
    auto pos = mrfs::find(name);
    return pos >= 0 ? (uint8_t const*) mrfsBase[pos].name : nullptr;
}

void arch::init (int size) {
    console.init();
#if STM32F103xB
    enableSysTick(); // no HSE crystal
#else
    console.baud(115200, fullSpeedClock());
#endif

    printf("\n"); // TODO yuck, the uart TX sends a 0xFF junk char after reset

    mrfs::init(mrfsBase, mrfsSize);
    //mrfs::dump();

    // negative sizes indicate required amount to not allocate
    if (size <= 0) {
        uint8_t top;
        int avail = &top - (uint8_t*) sbrk(0);
        size = avail + (size < -1024 ? size : -1024);
        assert(size > 1024); // there better be at least 1k for the pool
    }

    gcSetup(sbrk(size), size);
    Event::triggers.append(0); // TODO yuck, reserve 1st entry for VM
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

#ifdef UNIT_TEST
extern "C" {
    void unittest_uart_begin () { arch::init(1024); wait_ms(100); }
    void unittest_uart_putchar (char c) { console.putc(c); }
    void unittest_uart_flush () { while (!console.xmit.empty()) {} }
    void unittest_uart_end () { while (true) {} }
}
#endif
