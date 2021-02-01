#include <monty.h>
#include "arch.h"

#include <cassert>
#include <cstdio>
#include <ctime>

using namespace monty;

auto micros () -> uint64_t {
    struct timespec tv;
    clock_gettime(CLOCK_MONOTONIC, &tv);
    return tv.tv_sec * 1000000LL + tv.tv_nsec / 1000; // µs resolution
}

auto arch::loadFile (char const* name) -> uint8_t const* {
    auto fp = fopen(name, "rb");
    if (fp == nullptr) {
        perror(name);
        return nullptr;
    }
    fseek(fp, 0, SEEK_END);
    uint32_t bytes = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    auto data = (uint8_t*) malloc(bytes + 64);
    fread(data, 1, bytes, fp);
    fclose(fp);
    return data;
}

void arch::init () {
    setbuf(stdout, nullptr);
}

void arch::idle () {
    timespec ts { 0, 100000 };
    nanosleep(&ts, &ts); // 100 µs, i.e. 10% of ticks' 1 ms resolution
}

auto arch::done () -> int {
    return 0;
}

namespace machine {
    static Event tickEvent;
    static int ms, tickerId;
    static uint32_t start, last;

    // simulate in software, see INNER_HOOK in arch.h and monty/pyvm.h
    void timerHook () {
        auto u = micros();
        uint32_t t = u / 1000;
        if (ms > 0 && (t - start) / ms != last) {
            last = (t - start) / ms;
            if (tickerId > 0)
                exception(tickerId);
        }
    }

    static auto f_ticker (ArgVec const& args) -> Value {
        if (args.num > 0) {
            assert(args.num == 1 && args[0].isInt());
            ms = args[0];
            start = micros() / 1000; // set first timeout relative to now
            last = 0;
            tickerId = tickEvent.regHandler();
            assert(tickerId > 0);
        } else {
            tickEvent.deregHandler();
            tickerId = 0;
        }
        return tickEvent;
    }

    static Function const fo_ticker (f_ticker);

    static auto f_ticks (ArgVec const&) -> Value {
        uint32_t t = micros() / 1000;
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
