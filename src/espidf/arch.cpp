#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "monty.h"
#include "arch.h"

using namespace Monty;

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
