// mod-demo.cpp - a little demo module

#include <monty.h>

using namespace monty;

//CG: module demo

//CG1 bind twice
static auto f_twice (ArgVec const& args) -> Value {
    //CG: args val:i
    return 2 * val;
}

//CG1 wrappers
static Lookup::Item const demo_map [] = {
};

//CG: module-end
