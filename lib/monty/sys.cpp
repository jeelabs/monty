// sys.cpp - the sys module

#include "monty.h"
#include <cassert>

using namespace Monty;

//CG1 VERSION
constexpr auto VERSION = Q(167,"v0.94-3-g146b52a");

static auto f_suspend (ArgVec const& args) -> Value {
    assert(args.num == 2 && args[1].isInt());
    int id = args[1];
    if (id >= 0)
        Interp::suspend(id);
    return {};
}

static Function const fo_suspend (f_suspend);

static Lookup::Item const lo_sys [] = {
    { Q(168,"suspend"), fo_suspend },
    { Q(169,"tasks"), Interp::tasks },
    { Q(170,"modules"), Interp::modules },
    { Q(171,"implementation"), Q(172,"monty") },
    { Q(173,"version"), VERSION },
};

static Lookup const ma_sys (lo_sys, sizeof lo_sys);
extern Module const m_sys (&ma_sys);
