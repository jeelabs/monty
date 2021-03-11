// lib-demo.cpp - a little demo library

#include <monty.h>

using namespace monty;

//CG1 bind triple
static auto f_triple (ArgVec const& args) -> Value {
    //CG: args val:i
    return 3 * val;
}

//CG1 bind quadruple
static auto f_quadruple (ArgVec const& args) -> Value {
    //CG: args val
    return val.binOp(BinOp::Multiply, 4); // also handles Int type
}

//CG: wrappers

void initMyLib () {
    Module::builtins.at(Q(0,"triple")) = fo_triple; // add as built-in
    Int::info.at(Q(0,"quadruple")) = fo_quadruple;  // add to the Int type
}
