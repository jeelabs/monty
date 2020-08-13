// stack.cpp - execution state, stacks, and callables

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "xmonty.h"

using namespace Monty;

void Context::enter (Callable const& func,
                        Vector const& vec, int argc, int args, Dict* d) {
    auto frameSize = func.code.fastSlotTop() + EXC_STEP * func.code.excLevel();
    auto need = (frame().stack + frameSize) - end();

    auto curr = fill;           // current frame offset
    fill = limit;               // current limit
    insert(fill, need);         // make room
    fill = limit;               // frame offset is old limit
    limit += need;              // new limit of vector use

    auto& f = frame();          // new frame
    f.link = curr;              // index of (now previous) frame
    f.sp = spIdx;               // previous stack index
    f.ip = ipIdx;               // previous instruction index
    f.ep = epIdx;               // previous exception level
    f.callee = callee;          // previous callable
    f.locals = locals;          // previous locals dict

    spIdx = f.stack-begin()-1;  // stack starts out empty
    ipIdx = 0;                  // code starts at first opcode
    epIdx = 0;                  // no exceptions pending
    callee = &func;             // new callable context
    locals = d != nullptr ? d : new Dict (&globals());

    // TODO handle argument setup
}

Value Context::leave (Value v) {
    auto& f = frame();          // restore values before popping frame
    auto r = f.result;          // stored result
    int prev = f.link;          // previous frame offset
    assert(prev >= 0);

    spIdx = f.sp;
    ipIdx = f.ip;
    epIdx = f.ep;
    callee = &f.callee.asType<Callable>();

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

    assert(limit > fill);
    size_t base = fill;         // current frame offset
    fill = limit;               // adjust current limit
    remove(base, limit - base); // delete current frame
    limit = fill;               // new limit
    fill = prev;                // new lower frame offset

    return r.isNil() ? v : r;   // return result if set, else arg
}

auto Context::excBase (int incr) -> Value* {
    size_t ep = epIdx;
    epIdx = ep + incr;
    if (incr <= 0)
        --ep;
    return frame().stack + callee->code.fastSlotTop() + EXC_STEP * ep;
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
