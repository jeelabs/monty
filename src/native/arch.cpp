#include "monty.h"
#include "arch.h"

#include <cstdarg>
#include <time.h>

using namespace Monty;

int debugf (const char* fmt, ...) {
    va_list ap; va_start(ap, fmt);
    vprintf(fmt, ap); va_end(ap);
    return 0;
}

void archInit () {
    setbuf(stdout, 0);    
}

void archIdle () {
    timespec ts { 0, 10000 };
    nanosleep(&ts, &ts); // 10 µs, i.e. 1% of ticks' 1 ms resolution
}

int archDone () {
    //Context::gcTrigger();
    //Object::gcStats();
    return 0;
}

static int ms, id;
static uint32_t start, begin, last;

static uint32_t getTime () {
    struct timespec tv;
    clock_gettime(CLOCK_MONOTONIC, &tv);
    return tv.tv_sec * 1000 + tv.tv_nsec / 1000000; // ms resolution
}

// interface exposed to the VM

// simulate in software, see INNER_HOOK in arch.h, main.cpp, and monty/interp.h
void timerHook () {
    uint32_t t = getTime();
    if (ms > 0 && (t - start) / ms != last) {
        last = (t - start) / ms;
        if (id > 0)
            Interp::interrupt(id);
    }
}

static Value f_ticker (Vector const& ctx, int argc, int args) {
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

static Value f_ticks (Vector const& ctx, int argc, int args) {
    uint32_t t = getTime();
    if (begin == 0)
        begin = t;
    return t - begin; // make all runs start out the same way
}

static const Function fo_ticker (f_ticker);
static const Function fo_ticks (f_ticks);

static const Lookup::Item lo_machine [] = {
    { "ticker", &fo_ticker },
    { "ticks", &fo_ticks },
};

static const Lookup ma_machine (lo_machine, sizeof lo_machine);
const Module m_machine ({}, &ma_machine);
