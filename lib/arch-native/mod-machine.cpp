#include <monty.h>

#include <cassert>
#include <ctime>

using namespace monty;

//CG: module machine

Event tickEvent;
int ms, tickerId;
uint32_t start, last;

static auto micros () -> uint64_t {
    struct timespec tv;
    clock_gettime(CLOCK_MONOTONIC, &tv);
    return tv.tv_sec * 1000000LL + tv.tv_nsec / 1000; // µs resolution
}

static auto msNow () -> Value {
    uint64_t us = micros();
    static uint64_t begin;
    if (begin == 0)
        begin = us;
    return (us - begin) / 1000; // make all runs start out the same way
}

// simulate in software, see INNER_HOOK in arch.h and monty/pyvm.h
void timerHook () {
    uint32_t t = msNow();
    if (ms > 0 && (t - start) / ms != last) {
        last = (t - start) / ms;
        if (tickerId > 0)
            Stacklet::setPending(tickerId);
    }
}

//CG1 bind ticker
static auto f_ticker (ArgVec const& args) -> Value {
    if (args._num > 0) {
        assert(args._num == 1 && args[0].isInt());
        ms = args[0];
        start = msNow(); // set first timeout relative to now
        last = 0;
        tickerId = tickEvent.regHandler();
        assert(tickerId > 0);
    } else {
        tickEvent.deregHandler();
        tickerId = 0;
    }
    return tickEvent;
}

//CG1 bind ticks
static auto f_ticks (ArgVec const& args) -> Value {
    assert(args._num == 0);
    return msNow();
}

//CG< wrappers
static Function const fo_ticker (f_ticker);
static Function const fo_ticks (f_ticks);

static Lookup::Item const machine_map [] = {
    { Q(196,"ticker"), fo_ticker },
    { Q(197,"ticks"), fo_ticks },
//CG>
};

//CG2 module-end
static Lookup const machine_attrs (machine_map, sizeof machine_map);
Module ext_machine (Q(198,"machine"), machine_attrs);
