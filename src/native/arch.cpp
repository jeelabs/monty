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

void archIdle () {
    timespec ts { 0, 10000 };
    nanosleep(&ts, &ts); // 10 µs, i.e. 1% of ticks' 1 ms resolution
}

auto archDone (char const* msg) -> int {
    printf("%s\n", msg != nullptr ? msg : "done");
    return msg == nullptr ? 0 : 1;
}

static int ms, id;
static uint32_t start, begin, last;

static auto getTime () -> uint32_t {
    struct timespec tv;
    clock_gettime(CLOCK_MONOTONIC, &tv);
    return tv.tv_sec * 1000 + tv.tv_nsec / 1000000; // ms resolution
}

// interface exposed to the VM

// simulate in software, see INNER_HOOK in arch.h and monty/pyvm.h
void timerHook () {
    uint32_t t = getTime();
    if (ms > 0 && (t - start) / ms != last) {
        last = (t - start) / ms;
        if (id > 0)
            Interp::interrupt(id);
    }
}

static auto f_ticker (Vector const& ctx, int argc, int args) -> Value {
    Value h = id;
    if (argc > 1) {
        if (argc != 3 || !ctx[args+1].isInt())
            return -1;
        ms = ctx[args+1];
        h = ctx[args+2];
        start = getTime(); // set first timeout relative to now
        last = 0;
    }
    id = Interp::setHandler(h);
    return id;
}

static auto f_ticks (Vector const& ctx, int argc, int args) -> Value {
    uint32_t t = getTime();
    if (begin == 0)
        begin = t;
    return t - begin; // make all runs start out the same way
}

static Function const fo_ticker (f_ticker);
static Function const fo_ticks (f_ticks);

static Lookup::Item const lo_machine [] = {
    { "ticker", &fo_ticker },
    { "ticks", &fo_ticks },
};

static Lookup const ma_machine (lo_machine, sizeof lo_machine);
extern Module const m_machine ({}, &ma_machine);
