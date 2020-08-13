// stack.cpp - execution state, stacks, and callables

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "xmonty.h"

using namespace Monty;

void Context::enter (Callable const& callee, Chunk const& args, Dict const* d) {
    auto frameSize = callee.fastSlotTop() + EXC_STEP * callee.excDepth();
    auto need = (frame().stack + frameSize) - end();

    auto curr = fill;           // current frame offset
    fill = limit;               // current limit
    insert(fill, need);         // make room
    fill = limit;               // frame offset is old limit
    limit += need;              // new limit of vector use

    auto& f = frame();
    f.link = curr;              // index of (now previous) frame
    f.sp = fill + 7 - 1; // FIXME Frame   // stack is empty
    f.ip = 0;                   // code starts at first opcode
    f.ep = 0;                   // no exceptions pending
    f.code = callee;            // actual bytecode object

    // TODO only allocate the locals dict on first store
    f.locals = d != nullptr ? d : new Dict (&globals());
}

Value Context::leave (Value v) {
    auto r = frame().result;    // stored result
    int prev = frame().link;    // previous frame offset
    assert(prev >= 0);

    size_t base = fill;         // current frame offset
    fill = limit;               // current limit
    assert(fill > base);
    remove(base, fill - base);  // delete current frame
    assert(fill == base);
    limit = base;               // new limit
    fill = prev;                // new lower frame offset

    return r.isNil() ? v : r;   // return result if set, else arg
}

auto Context::excBase (int incr) const -> Value* {
    auto& callee = frame().code.asType<Callable>();
    size_t ep = frame().ep;
    frame().ep = ep + incr;
    if (incr <= 0)
        --ep;
    return frame().stack + callee.fastSlotTop() + EXC_STEP * ep;
}

auto Context::asArgs (size_t len, Value const* ptr) -> Chunk {
    assert((len == 0 && ptr == nullptr) ||
            (frame().stack <= ptr && ptr + len < begin() + limit));
    Chunk args (*this);
    args.len = len;
    args.off = len == 0 ? 0 : ptr - begin();
    return args;
}

void Context::raise (Value exc) {
    size_t flag = 0;
    if (exc.isInt()) {
        int n = exc;
        if (0 <= n && n < 32)
            flag = n;    // trigger soft-irq 1..31 (interrupt-safe)
        else
            event = exc; // trigger exception or other outer-loop req
    }

    // this spinloop correctly sets one bit in volatile "pending" state
    do // avoid potential race when an irq raises inside the "pending |= ..."
        pending |= 1<<flag;
    while ((pending & (1<<flag)) == 0);
}
