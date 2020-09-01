// sys.cpp - the sys module

#include "monty.h"
#include <cassert>

using namespace Monty;

//CG1 VERSION
constexpr auto VERSION = Q(167,"v0.94-4-g34c00fc");

static auto f_suspend (ArgVec const& args) -> Value {
    assert(args.num == 2 && args[1].isInt());
    int id = args[1];
    if (id >= 0)
        Interp::suspend(id);
    return {};
}

static Function const fo_suspend (f_suspend);

static List gcdata; // keep this around to avoid reallocating on each call

static auto f_gcstats (ArgVec const& args) -> Value {
    assert(args.num == 1);
    constexpr auto NSTATS = sizeof gcStats / sizeof (uint32_t);
    if (gcdata.size() != NSTATS+1) {
        gcdata.remove(0, gcdata.size());
        gcdata.insert(0, NSTATS+1);
    }
    gcdata.setAt(0, avail());
    for (size_t i = 0; i < NSTATS; ++i)
        gcdata.setAt(1+i, ((uint32_t const*) &gcStats)[i]);
    return gcdata;
}

static Function const fo_gcstats (f_gcstats);

static Lookup::Item const lo_sys [] = {
    { Q(168,"suspend"), fo_suspend },
    { Q(169,"tasks"), Interp::tasks },
    { Q(170,"modules"), Interp::modules },
    { Q(171,"gcstats"), fo_gcstats },
    { Q(172,"implementation"), Q(173,"monty") },
    { Q(174,"version"), VERSION },
};

static Lookup const ma_sys (lo_sys, sizeof lo_sys);
extern Module const m_sys (&ma_sys);
