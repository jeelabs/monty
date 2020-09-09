#include "monty.h"
#include "arch.h"

#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <ctime>

using namespace Monty;

auto micros () -> uint64_t {
    struct timespec tv;
    clock_gettime(CLOCK_MONOTONIC, &tv);
    return tv.tv_sec * 1000000LL + tv.tv_nsec / 1000; // µs resolution
}

void archInit () {
    setbuf(stdout, 0);    
    printf("main\n");
}

auto archTime () -> uint32_t {
    return micros() / 1000; // 32 bits @ millisecond resolution
}

void archIdle () {
    //printf("idle\n");
    timespec ts { 0, 100000 };
    nanosleep(&ts, &ts); // 100 µs, i.e. 10% of ticks' 1 ms resolution
}

void archMode (RunMode) {
    // whoops, no LEDs ...
}

auto archDone (char const* msg) -> int {
    printf("%s\n", msg != nullptr ? msg : "done");
    return msg == nullptr ? 0 : 1;
}

static int ms, tickerId, uartId;
static uint32_t start, begin, last;
static uint64_t uartBusy;

// interface exposed to the VM

// simulate in software, see INNER_HOOK in arch.h and monty/pyvm.h
void timerHook () {
    auto u = micros();
    uint32_t t = u / 1000;
    if (ms > 0 && (t - start) / ms != last) {
        last = (t - start) / ms;
        if (tickerId > 0)
            Interp::interrupt(tickerId);
    }
    if (uartBusy > 0 && u >= uartBusy) {
        if (uartId > 0)
            Interp::interrupt(uartId);
        uartBusy = 0;
    }
}

static auto f_ticker (ArgVec const& args) -> Value {
    if (args.num > 0) {
        assert(args.num == 1 && args[0].isInt());
        ms = args[0];
        start = archTime(); // set first timeout relative to now
        last = 0;
        tickerId = Interp::getQueueId();
        assert(tickerId > 0);
    } else {
        Interp::dropQueueId(tickerId);
        tickerId = -1;
    }
    return tickerId;
}

static auto f_ticks (ArgVec const& args) -> Value {
    (void) args; assert(args.num == 0);
    uint32_t t = archTime();
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

auto Uart::create (ArgVec const& args, Type const*) -> Value {
    assert(args.num == 0);
    uartId = Interp::getQueueId();
    return new Uart;
}

auto Uart::read (ArgVec const& args) -> Value {
    assert(args.num == 0);
    assert(false); // TODO
    return {};
}

auto Uart::write (ArgVec const& args) -> Value {
    assert(args.num == 4);
    assert(args[1].isInt() && args[2].isInt());
    auto& data = args[0].asType<Bytes>();
    int limit = args[1];
    int start = args[2];
    // TODO ignore args[3] deadline for now
    if (limit < 0)
        limit = data.len();

    if (start >= limit)
        return 0;

    constexpr auto baud = 19200;            // simulated baudrate
    constexpr auto pend = 32;               // max number of pending bytes
    constexpr auto rate = 10000000 / baud;  // time to send one byte in µs
    constexpr auto full = pend * rate;      // time to send a full "buffer"

    auto t = micros();
    if (uartBusy == 0)
        uartBusy = t;
    for (int i = start; i < limit; ++i) {
        if (uartBusy > t + full) {
            Interp::suspend(uartId);
            return i - start;
        }
        printf("%c", data[i]);
        uartBusy += rate;
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
    { "ticker", fo_ticker },
    { "ticks", fo_ticks },
    { "uart", Uart::info },
};

static Lookup const ma_machine (lo_machine, sizeof lo_machine);
extern Module const m_machine (ma_machine);
