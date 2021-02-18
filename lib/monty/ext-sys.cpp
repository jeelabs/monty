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

static Function const fo_event (f_event);
static Function const fo_gc (f_gc);
static Function const fo_gcmax (f_gcmax);
static Function const fo_gcstats (f_gcstats);

static Lookup::Item const lo_sys [] = {
    { Q(188,"tasks"), Stacklet::tasks },
    { Q(189,"modules"), Module::loaded },
    { Q(181,"event"), fo_event },
    { Q(190,"gc"), fo_gc },
    { Q(191,"gcmax"), fo_gcmax },
    { Q(192,"gcstats"), fo_gcstats },
    { Q(193,"implementation"), Q(194,"monty") },
#ifdef VERSION
    { Q(195,"version"), VERSION },
#endif
};

static Lookup const ma_sys (lo_sys, sizeof lo_sys);
extern Module const m_sys (ma_sys);
