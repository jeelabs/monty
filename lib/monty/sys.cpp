// sys.cpp - the sys module

#include "monty.h"
#include <cassert>

using namespace Monty;

//CG1 VERSION
constexpr auto VERSION = Q(167,"v0.93-118-g1ad1c20");

static auto f_snooze (ArgVec const& args) -> Value {
    assert(2 <= args.num && args.num <= 4 && args[1].isInt());

    size_t id = args[1];
    int ms = -1;
    if (args.num > 2 && args[2].isInt()) {
        ms = args[2];
        if (ms < -1)
            ms = -1;
    }
    uint32_t flags = 0;
    if (args.num > 3)
        for (char const* s = args[3]; *s != 0; ++s) {
            auto c = *s | 0x20; // ignore case
            if ('a' <= c && c <= 'z')
                flags |= 1 << (c-'a');
        }

    Interp::snooze(id, ms, flags);
    return {};
}

static Function const fo_snooze (f_snooze);

static auto f_suspend (ArgVec const& args) -> Value {
    auto queue = &Interp::tasks;
    if (args.num == 2)
        queue = &args[1].asType<List>();

    Interp::suspend(*queue);
    return {};
}

static Function const fo_suspend (f_suspend);

static Lookup::Item const lo_sys [] = {
    { Q(168,"snooze"), fo_snooze },
    { Q(169,"suspend"), fo_suspend },
    { Q(170,"tasks"), Interp::tasks },
    { Q(171,"modules"), Interp::modules },
    { Q(172,"implementation"), Q(173,"monty") },
    { Q(174,"version"), VERSION },
};

static Lookup const ma_sys (lo_sys, sizeof lo_sys);
extern Module const m_sys (&ma_sys);
