#include "monty.h"
#include "arch.h"

#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <ctime>

using namespace Monty;

void archInit () {
    setbuf(stdout, 0);    
    printf("main\n");
}

auto archTime () -> uint32_t {
    struct timespec tv;
    clock_gettime(CLOCK_MONOTONIC, &tv);
    return tv.tv_sec * 1000 + tv.tv_nsec / 1000000; // ms resolution
}

void archIdle () {
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

static int ms, id;
static uint32_t start, begin, last;

// interface exposed to the VM

// simulate in software, see INNER_HOOK in arch.h and monty/pyvm.h
void timerHook () {
    uint32_t t = archTime();
    if (ms > 0 && (t - start) / ms != last) {
        last = (t - start) / ms;
        if (id > 0)
            Interp::interrupt(id);
    }
}

static auto f_ticker (ArgVec const& args) -> Value {
    Value h = id;
    if (args.num > 1) {
        if (args.num != 3 || !args[1].isInt())
            return -1;
        ms = args[1];
        h = args[2];
        start = archTime(); // set first timeout relative to now
        last = 0;
    }
    id = Interp::setHandler(h);
    return id;
}

static auto f_ticks (ArgVec const&) -> Value {
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

    auto attr (char const* name, Value& self) const -> Value override;

    void marker () const override;
};

auto Uart::create (ArgVec const& args, Type const*) -> Value {
    assert(args.num == 1);
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
        printf("%c", data[i]);
    return limit - start;
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

static const Lookup::Item uartMap [] = {
    { "read", m_uart_read },
    { "write", m_uart_write },
};

const Lookup Uart::attrs (uartMap, sizeof uartMap);

Type const Uart::info ("<uart>", Uart::create, &Uart::attrs);

static Lookup::Item const lo_machine [] = {
    { "ticker", fo_ticker },
    { "ticks", fo_ticks },
    { "uart", Uart::info },
};

static Lookup const ma_machine (lo_machine, sizeof lo_machine);
extern Module const m_machine (&ma_machine);
