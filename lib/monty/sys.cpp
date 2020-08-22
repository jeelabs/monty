// sys.cpp - the sys module

#include "monty.h"
#include <cassert>

using namespace Monty;

//CG1 VERSION
constexpr auto VERSION = "v0.93-55-g5916f37";

static auto f_suspend (ArgVec const& args) -> Value {
    auto queue = &Interp::tasks;
    if (args.num == 2)
        queue = &args[1].asType<List>();
    Interp::suspend(*queue);
    return Value ();
}

static Function const fo_suspend (f_suspend);

static Lookup::Item const lo_sys [] = {
    { "suspend", fo_suspend },
    { "tasks", Interp::tasks },
    //{ "modules", Interp::modules },
    { "implementation", "monty" },
    { "version", VERSION },
};

static Lookup const ma_sys (lo_sys, sizeof lo_sys);
extern Module const m_sys (&ma_sys);
