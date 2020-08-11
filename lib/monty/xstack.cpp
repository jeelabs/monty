// stack.cpp - execution state, stacks, and callables

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "xmonty.h"

using namespace Monty;

void Context::push (Callable const& callee) {
    auto prev = fill - base();
    begin()[0] = fill;

    auto need = Extra + callee.frameSize();
    insert(fill, need);

    frame(Link) = prev;
    frame(Sp) = base() + Result;
    frame(Code) = callee;
}

void Context::pop () {
    int prev = frame(Link);
    assert(prev > 0);
    auto off = base();
    remove(off, fill - off);
    begin()[0] = off - prev;
}

auto Context::ipBase () -> uint8_t const* {
    return frame(Code).asType<Callable>().codeStart();
}

auto Context::fastSlot (size_t i) -> Value& {
    auto fastBase = 0; // FIXME
    return frame(fastBase + ~i);
}

auto Context::asDict (Reg r) -> Dict& {
    if (frame(r).isNil())
        frame(r) = new Dict;
    return frame(r).asType<Dict>();
}

auto Context::asArgs (size_t len, Value const* ptr) -> Chunk {
    Chunk args = *this;
    assert(&frame(0) <= ptr && ptr < end());
    args.off = ptr - begin();
    args.len = len;
    return args;
}
