// sys.cpp - the sys module

#include "monty.h"
#include <cassert>

using namespace Monty;

//CG1 VERSION
constexpr auto VERSION = "v0.92-39-g20e385d";

static auto f_suspend (Vector const& vec, int argc, int args) -> Value {
    auto queue = &Interp::tasks;
    if (argc == 2)
        queue = &vec[args+1].asType<List>();
    Interp::suspend(*queue);
    return Value ();
}

static Function const fo_suspend (f_suspend);

static Lookup::Item const lo_sys [] = {
    { "version", VERSION },
    { "suspend", &fo_suspend },
    { "tasks", &Interp::tasks },
    //{ "modules", &Interp::modules },
};

static Lookup const ma_sys (lo_sys, sizeof lo_sys);
extern Module const m_sys ({}, &ma_sys);
