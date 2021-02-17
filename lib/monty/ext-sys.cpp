// sys.cpp - the sys module

#include "monty.h"
#include <cassert>

using namespace monty;

static auto f_event (ArgVec const& args) -> Value {
    assert(args.num == 0);
    return new Event;
}

static auto f_gc (ArgVec const& args) -> Value {
    assert(args.num == 0);
    Stacklet::gcAll();
    return {};
}

static auto f_gcmax (ArgVec const& args) -> Value {
    assert(args.num == 0);
    return gcMax();
}

static List gcdata; // keep this around to avoid reallocating on each call

static auto f_gcstats (ArgVec const& args) -> Value {
    assert(args.num == 0);
    constexpr auto NSTATS = sizeof gcStats / sizeof (uint32_t);
    if (gcdata.size() == 0)
        gcdata.insert(0, NSTATS);
    for (uint32_t i = 0; i < NSTATS; ++i)
        gcdata[i] = gcStats.v[i];
    return gcdata;
}

static Lookup::Item const lo_sys [] = {
    { Q(187,"tasks"), Stacklet::tasks },
    { Q(188,"modules"), Module::loaded },
    { Q(180,"event"), f_event },
    { Q(189,"gc"), f_gc },
    { Q(190,"gcmax"), f_gcmax },
    { Q(191,"gcstats"), f_gcstats },
    { Q(192,"implementation"), Q(193,"monty") },
#ifdef VERSION
    { Q(194,"version"), VERSION },
#endif
};

static Lookup const ma_sys (lo_sys, sizeof lo_sys);
extern Module const m_sys (ma_sys);
