// state.cpp - execution state, stacks, and callables

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "xmonty.h"

using namespace Monty;

void Context::push (Callable const& callee) {
    auto diff = Extra + callee.frameSize();
    stack.insert(stack.len, diff);
    stack.off += diff;
    stack.len = diff;

    stack[Sp] = Result;
    stack[Code] = callee;
}

void Context::pop () {
    int diff = stack[Link];
    assert(diff > 0);
    stack.len = diff;
    stack.off -= diff;
}
