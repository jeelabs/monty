#include "monty.h"
#include "arch.h"

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include <jee.h>

using namespace Monty;

#if BOARD_discovery_f4
static PinD<12> ledG;  // green = running
static PinD<13> ledO;  // orange = collecting
static PinD<14> ledR;  // red = halted
static PinD<15> ledB;  // blue = idle

static void initLeds () {
    ledG.mode(Pinmode::out);
    ledO.mode(Pinmode::out);
    ledR.mode(Pinmode::out);
    ledB.mode(Pinmode::out);
}
#else
static void initLeds () {}
#endif

UartBufDev< PINS_CONSOLE > console;

int printf (char const* fmt, ...) {
    va_list ap; va_start(ap, fmt);
    veprintf(console.putc, fmt, ap); va_end(ap);
    return 0;
}

extern "C" void __assert_func (char const* f, int l, char const* n, char const* e) {
    printf("assert(%s) in %s\n\t%s:%d\n", e, n, f, l);
    while (true) {}
}

extern "C" void __assert (char const* f, int l, char const* e) {
    __assert_func(f, l, "-", e);
}

void archInit () {
    console.init();
    console.baud(115200, fullSpeedClock() / UART_BUSDIV);
    wait_ms(200);
    printf("\xFF" "main\n"); // insert marker for serial capture by dog.c
    initLeds();
    archMode(RunMode::Run);
}

auto archTime () -> uint32_t {
    return ticks;
}

void archIdle () {
    archMode(RunMode::Idle);
    asm ("wfi");
    archMode(RunMode::Run);
}

void archMode (RunMode r) {
#if BOARD_discovery_f4
    ledG = ((int) r >> 0) & 1;
    ledO = ((int) r >> 1) & 1;
    ledR = ((int) r >> 2) & 1;
    ledB = ((int) r >> 3) & 1;
#endif
}

auto archDone (char const* msg) -> int {
    // add a null byte at end of output to quickly stop dog.c
    printf("%s\n%c", msg != nullptr ? msg : "done", 0);
    //while (!console.xmit.empty()) {}
    archMode(RunMode::Done);
    while (true) asm ("wfi");
    //return msg == nullptr ? 0 : 1);
}

static int ms, tickerId, uartId;
static uint32_t start, begin, last;

// interface exposed to the VM

Value f_ticker (ArgVec const& args) {
    if (args.num > 1) {
        if (args.num != 3 || !args[1].isInt())
            return -1;
        ms = args[1];
        start = ticks; // set first timeout relative to now
        last = 0;
        tickerId = Interp::getQueueId();
        VTableRam().systick = []() {
            uint32_t t = ++ticks;
            if (ms > 0 && (t - start) / ms != last) {
                last = (t - start) / ms;
                if (tickerId > 0)
                    Interp::interrupt(tickerId);
            }
        };
    } else {
        Interp::dropQueueId(tickerId);
        tickerId = -1;
    }
    return tickerId;
}

Value f_ticks (ArgVec const&) {
    uint32_t t = ticks;
    if (begin == 0)
        begin = t;
    return t - begin; // make all runs start out the same way
}

static Function const fo_ticker (f_ticker);
static Function const fo_ticks (f_ticks);

struct Uart: Object {
    static auto create (ArgVec const&, Type const*) -> Value;
    static Lookup const attrs;
    static Type const info;
    auto type () const -> Type const& override { return info; }

    Uart () {}

    Value read (ArgVec const& args);
    Value write (ArgVec const& args);
    Value close ();

    auto attr (char const* name, Value& self) const -> Value override;

    void marker () const override;
};

static void (*prevIrq)();

auto Uart::create (ArgVec const& args, Type const*) -> Value {
    assert(args.num == 1);
    uartId = Interp::getQueueId();

    prevIrq = VTableRam().usart2;
    VTableRam().usart2 = []() {
        auto f = console.xmit.empty();
        prevIrq();
        if (!f && console.xmit.empty())
            Interp::interrupt(uartId); // it became writable again
    };

    return new Uart;
}

auto Uart::read (ArgVec const& args) -> Value {
    assert(args.num == 1);
    assert(false); // TODO
    return {};
}

auto Uart::write (ArgVec const& args) -> Value {
    assert(args.num == 5);
    assert(args[2].isInt() && args[3].isInt());
    auto& data = args[1].asType<Bytes>();
    int limit = args[2];
    int start = args[3];
    // TODO ignore args[4] deadline for now
    if (limit < 0)
        limit = data.len();
    for (int i = start; i < limit; ++i)
        if (console.writable())
            console.putc(data[i]);
        else {
            Interp::suspend(uartId);
            return i - start;
        }
    return limit - start;
}

auto Uart::close () -> Value {
    if (uartId > 0)
        Interp::dropQueueId(uartId);
    uartId = 0;
    return {};
}

Value Uart::attr (const char* key, Value&) const {
    return attrs.getAt(key);
}

void Uart::marker () const {
    // TODO
}

static auto d_uart_read = Method::wrap(&Uart::read);
static Method const m_uart_read (d_uart_read);

static auto d_uart_write = Method::wrap(&Uart::write);
static Method const m_uart_write (d_uart_write);

static auto d_uart_close = Method::wrap(&Uart::close);
static Method const m_uart_close (d_uart_close);

static const Lookup::Item uartMap [] = {
    { "read", m_uart_read },
    { "write", m_uart_write },
    { "close", m_uart_close },
};

const Lookup Uart::attrs (uartMap, sizeof uartMap);

Type const Uart::info ("<uart>", Uart::create, &Uart::attrs);

static Lookup::Item const lo_machine [] = {
    { "ticker", &fo_ticker },
    { "ticks", &fo_ticks },
    { "uart", Uart::info },
};

static Lookup const ma_machine (lo_machine, sizeof lo_machine);
extern Module const m_machine (&ma_machine);

#if STM32L0

extern "C" unsigned __atomic_fetch_or_4 (void volatile* p, unsigned v, int o) {
    // see https://gcc.gnu.org/onlinedocs/gcc/_005f_005fatomic-Builtins.html
    // FIXME this version is not atomic!
    auto q = (unsigned volatile*) p;
    // atomic start
    auto t = *q;
    *q |= v;
    // atomic end
    return t;
}

extern "C" unsigned __atomic_fetch_and_4 (void volatile* p, unsigned v, int o) {
    // see https://gcc.gnu.org/onlinedocs/gcc/_005f_005fatomic-Builtins.html
    // FIXME this version is not atomic!
    auto q = (unsigned volatile*) p;
    // atomic start
    auto t = *q;
    *q &= v;
    // atomic end
    return t;
}

#endif
