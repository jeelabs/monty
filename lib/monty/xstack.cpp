// stack.cpp - execution state, stacks, and callables

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "xmonty.h"

using namespace Monty;

void Context::enter (Callable const& func, Chunk const& args, Dict* d) {
    auto frameSize = func.fastSlotTop() + EXC_STEP * func.excDepth();
    auto need = (frame().stack + frameSize) - end();

    auto curr = fill;           // current frame offset
    fill = limit;               // current limit
    insert(fill, need);         // make room
    fill = limit;               // frame offset is old limit
    limit += need;              // new limit of vector use

    auto& f = frame();
    f.link = curr;              // index of (now previous) frame
    f.sp = spIdx;               // previous stack index
    f.ip = ipIdx;               // previous instruction index
    f.ep = epIdx;               // previous exception level
    f.code = callee;            // previous callable
    f.locals = locals;          // previous locals dict

    spIdx = fill + 7 - 1; // FIXME Frame   // stack is empty
    ipIdx = 0;                  // code starts at first opcode
    epIdx = 0;                  // no exceptions pending
    callee = &func;             // new callable context
    locals = d != nullptr ? d : new Dict (&globals());
}

Value Context::leave (Value v) {
    auto r = frame().result;    // stored result
    int prev = frame().link;    // previous frame offset
    assert(prev >= 0);

    auto& f = frame();          // restore values before popping frame
    spIdx = f.sp;
    ipIdx = f.ip;
    epIdx = f.ep;
    callee = &f.code.asType<Callable>();

    // TODO ifType & asType don't know anything about derived classes
#if 0
    locals = f.locals.ifType<Dict>();
    if (locals == nullptr) {
        locals = f.locals.ifType<Class>();
        if (locals == nullptr)
            locals = &f.locals.asType<Module>();
    }
#else
    locals = (Dict*) &f.locals.obj();
#endif

    size_t base = fill;         // current frame offset
    fill = limit;               // current limit
    assert(fill > base);
    remove(base, fill - base);  // delete current frame
    assert(fill == base);
    limit = base;               // new limit
    fill = prev;                // new lower frame offset

    return r.isNil() ? v : r;   // return result if set, else arg
}

auto Context::excBase (int incr) -> Value* {
    size_t ep = epIdx;
    epIdx = ep + incr;
    if (incr <= 0)
        --ep;
    return frame().stack + callee->fastSlotTop() + EXC_STEP * ep;
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
