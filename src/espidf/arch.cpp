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

static auto bi_blah (ArgVec const& args) -> Value {
    return args.num;
}

static Function const f_blah (bi_blah);

static Lookup::Item const lo_machine [] = {
    { "blah", &f_blah },
};

static Lookup const ma_machine (lo_machine, sizeof lo_machine);
extern Module const m_machine (&ma_machine);
