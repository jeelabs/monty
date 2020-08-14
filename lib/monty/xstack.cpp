// stack.cpp - execution state, stacks, and callables

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "xmonty.h"

using namespace Monty;

volatile uint32_t Context::pending;
Context* Context::active;

void Context::enter (Callable const& func) {
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
    f.result = result;          // previous result

    spIdx = f.stack-begin()-1;  // stack starts out empty
    ipIdx = 0;                  // code starts at first opcode
    epIdx = 0;                  // no exceptions pending
    callee = &func;             // new callable context
    locals = 0;                 // create a new dict on demand
    result = {};                // start with no result
}

Value Context::leave (Value v) {
    auto r = result;            // stored result
    if (r.isNil())              // use return result if set
        r = v;                  // ... else arg

    if (fill > 0) {
        auto& f = frame();      // restore values before popping frame
        int prev = f.link;      // previous frame offset
        assert(prev >= 0);

        spIdx = f.sp;
        ipIdx = f.ip;
        epIdx = f.ep;
        callee = &f.callee.asType<Callable>();
        result = f.result;

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
        size_t base = fill;     // current frame offset
        fill = limit;           // adjust current limit
        remove(base, limit - base); // delete current frame
        limit = fill;           // new limit
        fill = prev;            // new lower frame offset
    } else
        active = caller; // last frame, drop this stack context, restore caller

    return r;
}

auto Context::excBase (int incr) -> Value* {
    size_t ep = epIdx;
    epIdx = ep + incr;
    if (incr <= 0)
        --ep;
    return frame().stack + callee->code.fastSlotTop() + EXC_STEP * ep;
}

void Context::raise (Value exc) {
    uint32_t num = 0;
    if (exc.isInt()) {
        uint32_t n = exc;
        if (n < 8 * sizeof pending)
            num = n;     // trigger soft-irq 1..31 (interrupt-safe)
        else
            event = exc; // trigger exception or other outer-loop req
    }

    interrupt(num); // set pending, to force inner loop exit
}

void Context::caught (Value e) {
    assert(epIdx > 0); // simple exception, no stack unwind
    auto ep = excBase(0);
    ipIdx = ep[0];
    spIdx = ep[1];
    begin()[spIdx] = e.isNil() ? ep[3] : e;
}

auto Context::next () -> Value {
    assert(active != this);
    raise(); // force inner loop exit
    active = this;
    return {};
}

void Context::marker () const {
    List::marker();
    mark(callee);
    mark(locals);
    mark(event);
    mark(caller);
}

void Monty::interrupt (uint32_t num) {
    assert(num < 8 * sizeof Context::pending);
    __atomic_or_fetch(&Context::pending, 1 << num, __ATOMIC_RELAXED);
}

void Monty::exception (Value v) {
    assert(!v.isInt());
    assert(Context::active != nullptr);
    Context::active->raise(v);
}
