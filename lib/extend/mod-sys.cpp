// sys.cpp - the sys module

#include <monty.h>
#include <cassert>

using namespace monty;

//CG: module sys

//CG1 bind gc
static auto f_gc (ArgVec const& args) -> Value {
    //CG: args
    Stacklet::gcAll();
    return {};
}

//CG1 bind gcmax
static auto f_gcmax (ArgVec const& args) -> Value {
    //CG: args
    return gcMax();
}

//CG1 bind gcstats
static auto f_gcstats (ArgVec const& args) -> Value {
    //CG: args
    auto data = new List;
    for (auto e : gcStats.v)
        data->append(e);
    return data;
}

//CG1 wrappers
static Lookup::Item const sys_map [] = {
    { Q(0,"ready"), Stacklet::ready },
    { Q(0,"modules"), Module::loaded },
    { Q(0,"builtins"), Module::builtins },
    { Q(0,"implementation"), Q(0,"monty") },
    { Q(0,"version"), VERSION },
};

//CG: module-end
