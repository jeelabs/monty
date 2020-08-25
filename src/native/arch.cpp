#include "monty.h"
#include "arch.h"

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

static Lookup::Item const lo_machine [] = {
    { "ticker", fo_ticker },
    { "ticks", fo_ticks },
};

static Lookup const ma_machine (lo_machine, sizeof lo_machine);
extern Module const m_machine (&ma_machine);
