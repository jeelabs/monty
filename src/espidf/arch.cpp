#include "monty.h"
#include "arch.h"

#include <ctime>

using namespace Monty;

auto archTime () -> uint32_t {
    struct timespec tv;
    clock_gettime(CLOCK_MONOTONIC, &tv);
    return tv.tv_sec * 1000 + tv.tv_nsec / 1000000; // ms resolution
}

void archMode (RunMode) {
    // whoops, no LEDs ...
}

static auto f_ticker (ArgVec const& args) -> Value {
    return args.num;
}

static auto f_ticks (ArgVec const& args) -> Value {
    return args.num;
}

static Function const fo_ticker (f_ticker);
static Function const fo_ticks (f_ticks);

static Lookup::Item const lo_machine [] = {
    { "ticker", &fo_ticker },
    { "ticks", &fo_ticks },
};

static Lookup const ma_machine (lo_machine, sizeof lo_machine);
extern Module const m_machine (&ma_machine);
