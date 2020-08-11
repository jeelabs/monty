// stack.cpp - execution state, stacks, and callables

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "xmonty.h"

using namespace Monty;

void Context::push (Callable const& callee) {
    auto need = (frame().stack + callee.frameSize()) - end();

    auto curr = fill;           // current frame offset
    fill = begin()[0];          // current limit
    insert(fill, need);         // make room
    fill = begin()[0];          // frame offset is old limit

    auto& f = frame();
    f.link = curr;              // index of (now previous) frame
    f.sp = -1; // stack is empty
    f.ip = 0;                   // code starts at first opcode
    f.ep = 0;                   // no exceptions pending
    f.code = callee;            // actual bytecode object
}

Value Context::pop (Value v) {
    auto r = frame().result;    // stored result
    int prev = frame().link;    // previous frame offset
    assert(prev > 0);

    size_t base = fill;         // current frame offset
    fill = begin()[0];          // current limit
    assert(fill > base);
    remove(fill, fill - base);  // delete current frame
    assert(fill == base);
    begin()[0] = base;          // new limit
    fill = prev;                // new lower frame offset

    return r.isNil() ? v : r;   // return result if set, else arg
}

auto Context::ipBase () const -> uint8_t const* {
    return frame().code.asType<Callable>().codeStart();
}

auto Context::fastSlot (size_t i) const -> Value& {
    auto fastBase = 0; // FIXME
    return frame().stack[fastBase + ~i];
}

auto Context::asDict (int n) const -> Dict& {
    auto& d = frame().dicts[n];
    if (d.isNil())
        d = new Dict;
    return d.asType<Dict>();
}

auto Context::asArgs (size_t len, Value const* ptr) -> Chunk {
    assert(frame().stack <= ptr && ptr + len < begin() + limit());
    Chunk args (*this);
    args.len = len;
    args.off = ptr - begin();
    return args;
}
