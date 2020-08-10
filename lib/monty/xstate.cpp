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

auto Context::ipBase () const -> uint8_t const* {
    return stack[Code].asType<Callable>().codeStart();
}

auto Context::fastSlot (size_t i) -> Value& {
    auto fastBase = 0; // FIXME
    return stack[fastBase + ~i];
}

auto Context::asDict (Reg r) -> Dict& {
    if (stack[r].isNil())
        stack[r] = new Dict;
    return stack[r].asType<Dict>();
}

auto Context::asArgs (size_t len, Value const* ptr) const -> CofV {
    CofV args = stack;
    assert(args.begin() <= ptr && ptr < args.end());
    args.off = ptr - args.begin();
    args.len = len;
    return stack;
}
