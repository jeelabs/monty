// sys.cpp - the sys module

#include "monty.h"
#include <cassert>

using namespace monty;

//CG: module sys

//CG1 bind event
static auto f_event (ArgVec const& args) -> Value {
    assert(args.num == 0);
    return new Event;
}

//CG1 bind gc
static auto f_gc (ArgVec const& args) -> Value {
    assert(args.num == 0);
    Stacklet::gcAll();
    return {};
}

//CG1 bind gcmax
static auto f_gcmax (ArgVec const& args) -> Value {
    assert(args.num == 0);
    return gcMax();
}

static List gcdata; // keep this around to avoid reallocating on each call

//CG1 bind gcstats
static auto f_gcstats (ArgVec const& args) -> Value {
    assert(args.num == 0);
    constexpr auto NSTATS = sizeof gcStats / sizeof (uint32_t);
    if (gcdata.size() == 0)
        gcdata.insert(0, NSTATS);
    for (uint32_t i = 0; i < NSTATS; ++i)
        gcdata[i] = gcStats.v[i];
    return gcdata;
}

//CG< wrappers
static Function const fo_event (f_event);
static Function const fo_gc (f_gc);
static Function const fo_gcmax (f_gcmax);
static Function const fo_gcstats (f_gcstats);

static Lookup::Item const sys_map [] = {
    { Q(167,"event"), fo_event },
    { Q(168,"gc"), fo_gc },
    { Q(169,"gcmax"), fo_gcmax },
    { Q(170,"gcstats"), fo_gcstats },
//CG>
    { Q(171,"tasks"), Stacklet::tasks },
    { Q(172,"modules"), Module::loaded },
    { Q(173,"implementation"), Q(174,"monty") },
#ifdef VERSION
    { Q(175,"version"), VERSION },
#endif
};

static Lookup const sys_attrs (sys_map, sizeof sys_map);
extern Module const ext_sys (sys_attrs, Q(176,"sys"));
