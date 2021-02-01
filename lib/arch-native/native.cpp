#include <monty.h>
#include "arch.h"

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

static auto f_ticks (ArgVec const&) -> Value {
    uint32_t t = micros() / 1000;
    static uint32_t begin;
    if (begin == 0)
        begin = t;
    return t - begin; // make all runs start out the same way
}

static Function const fo_ticks (f_ticks);

static Lookup::Item const lo_machine [] = {
    //XXX { "ticker", fo_ticker },
    { "ticks", fo_ticks },
    //XXX { "uart", Uart::info },
};

static Lookup const ma_machine (lo_machine, sizeof lo_machine);
extern Module const m_machine (ma_machine);
