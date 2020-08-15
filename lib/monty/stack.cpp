// stack.cpp - execution state, stacks, and callables

#include "monty.h"
#include <cassert>

using namespace Monty;

volatile uint32_t Runner::pending;
Context* Runner::context;

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
        Runner::context = caller; // last frame, drop this stack context, restore caller

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
        num = exc;      // trigger soft-irq 1..31 (interrupt-safe)
        assert(num < 8 * sizeof Runner::pending);
    } else
        event = exc;    // trigger exception or other outer-loop req

    Runner::interrupt(num);     // set pending, to force inner loop exit
}

void Context::caught () {
    auto e = event;
    event = {};
    if (e.isNil())
        return; // there was no exception, just an inner loop exit

    assert(epIdx > 0); // simple exception, no stack unwind
    auto ep = excBase(0);
    ipIdx = ep[0];
    spIdx = ep[1];
    begin()[++spIdx] = e.isNil() ? ep[3] : e;
}

auto Context::next () -> Value {
    Runner::resume(*this);
    return {}; // no result yet
}

void Context::marker () const {
    List::marker();
    mark(callee);
    mark(locals);
    mark(event);
    mark(caller);
}

void Runner::resume (Context& ctx) {
    assert(context != &ctx);
    context = &ctx;
    interrupt(0); // force inner loop exit
}

void Runner::exception (Value v) {
    assert(!v.isInt());
    assert(context != nullptr);
    context->raise(v);
}

void Runner::interrupt (uint32_t num) {
    assert(num < 8 * sizeof Runner::pending);
    // see https://gcc.gnu.org/onlinedocs/gcc/_005f_005fatomic-Builtins.html
    __atomic_fetch_or(&pending, 1U << num, __ATOMIC_RELAXED);
}

auto Runner::nextPending () -> int {
    if (pending != 0)
        for (size_t num = 0; num < 8 * sizeof Runner::pending; ++num)
            if (wasPending(num))
                return num;
    return -1;
}

auto Runner::wasPending (uint32_t num) -> bool {
    assert(num < 8 * sizeof Runner::pending);
    auto mask = 1U << num;
    // see https://gcc.gnu.org/onlinedocs/gcc/_005f_005fatomic-Builtins.html
    return (__atomic_fetch_nand(&Runner::pending, mask, __ATOMIC_RELAXED) & mask) != 0;
}
