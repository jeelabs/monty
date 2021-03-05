// sys.cpp - the sys module

#include <monty.h>
#include <cassert>

using namespace monty;

//CG: module sys

//CG1 bind event
static auto f_event (ArgVec const& args) -> Value {
    assert(args.size() == 0);
    return new Event;
}

//CG1 bind gc
static auto f_gc (ArgVec const& args) -> Value {
    assert(args.size() == 0);
    Stacklet::gcAll();
    return {};
}

//CG1 bind gcmax
static auto f_gcmax (ArgVec const& args) -> Value {
    assert(args.size() == 0);
    return gcMax();
}

//CG1 bind gcstats
static auto f_gcstats (ArgVec const& args) -> Value {
    assert(args.size() == 0);
    auto data = new List;
    for (auto e : gcStats.v)
        data->append(e);
    return data;
}

//CG< wrappers
static Function const fo_event (f_event);
static Function const fo_gc (f_gc);
static Function const fo_gcmax (f_gcmax);
static Function const fo_gcstats (f_gcstats);

static Lookup::Item const sys_map [] = {
    { Q(171,"event"), fo_event },
    { Q(172,"gc"), fo_gc },
    { Q(173,"gcmax"), fo_gcmax },
    { Q(174,"gcstats"), fo_gcstats },
//CG>
    { Q(176,"ready"), Stacklet::ready },
    { Q(177,"modules"), Module::loaded },
    { Q(63,"builtins"), Module::builtins },
    { Q(178,"implementation"), Q(179,"monty") },
#ifdef VERSION
    { Q(180,"version"), VERSION },
#endif
};

//CG2 module-end
static Lookup const sys_attrs (sys_map);
Module ext_sys (Q(175,"sys"), sys_attrs);
