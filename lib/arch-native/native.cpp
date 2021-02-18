#include <monty.h>
#include "arch.h"

#include <cassert>
#include <cstdio>
#include <ctime>

using namespace monty;

//CG: module machine

auto micros () -> uint64_t {
    struct timespec tv;
    clock_gettime(CLOCK_MONOTONIC, &tv);
    return tv.tv_sec * 1000000LL + tv.tv_nsec / 1000; // µs resolution
}

auto arch::loadFile (char const* name) -> uint8_t const* {
    auto fp = fopen(name, "rb");
    if (fp == nullptr)
        return nullptr;
    fseek(fp, 0, SEEK_END);
    uint32_t bytes = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    auto data = (uint8_t*) malloc(bytes + 64);
    fread(data, 1, bytes, fp);
    fclose(fp);
    return data;
}

auto arch::importer (char const* name) -> uint8_t const* {
    auto data = loadFile(name);
    if (data != nullptr)
        return data;
    assert(strlen(name) < 25);
    char buf [40];
    sprintf(buf, "pytests/%s.mpy", name);
    data = loadFile(buf);
    if (data == nullptr)
        perror(name);
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
    Event tickEvent;
    int ms, tickerId;
    uint32_t start, last;

    auto msNow () -> Value {
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

    auto f_ticker (ArgVec const& args) -> Value {
        if (args.num > 0) {
            assert(args.num == 1 && args[0].isInt());
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

    Function const fo_ticker (f_ticker);

    auto f_ticks (ArgVec const&) -> Value {
        return msNow();
    }

    Function const fo_ticks (f_ticks);

    Lookup::Item const attrs [] = {
        { Q(199,"ticker"), fo_ticker },
        { Q(200,"ticks"), fo_ticks },
    };
}

static Lookup const machine_attrs (machine::attrs, sizeof machine::attrs);
Module ext_machine (machine_attrs, Q(201,"machine"));
