#include "monty.h"
#include "arch.h"
#include <arduino.h>

using namespace Monty;

auto archTime () -> uint32_t {
    return millis();
}

void archMode (RunMode) {
    // whoops, no LEDs ...
}

static auto f_ticker (ArgVec const& args) -> Value {
    return args.num;
}

static auto f_ticks (ArgVec const& args) -> Value {
    return millis();
}

static Function const fo_ticker (f_ticker);
static Function const fo_ticks (f_ticks);

static Lookup::Item const lo_machine [] = {
    { "ticker", &fo_ticker },
    { "ticks", &fo_ticks },
};

static Lookup const ma_machine (lo_machine, sizeof lo_machine);
extern Module const m_machine (&ma_machine);

extern "C" uint32_t __atomic_fetch_or_4 (void volatile* p, uint32_t v, int o) {
    // see https://gcc.gnu.org/onlinedocs/gcc/_005f_005fatomic-Builtins.html
    // FIXME this version is not atomic!
    auto q = (uint32_t volatile*) p;
    // atomic start
    auto t = *q;
    *q |= v;
    // atomic end
    return t;
}

extern "C" uint32_t __atomic_fetch_and_4 (void volatile* p, uint32_t v, int o) {
    // see https://gcc.gnu.org/onlinedocs/gcc/_005f_005fatomic-Builtins.html
    // FIXME this version is not atomic!
    auto q = (uint32_t volatile*) p;
    // atomic start
    auto t = *q;
    *q &= v;
    // atomic end
    return t;
}
