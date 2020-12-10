// stack.cpp - run time contexts and interpreter

#include "monty.h"
#include <cassert>

using namespace monty;

volatile uint32_t Interp::pending;
uint32_t          Interp::queueIds;
List              Interp::tasks;
Dict              Interp::modules;
Context*          Interp::context;

void Context::enter (Callable const& func) {
    auto frameSize = 0;//XXX func.bc.fastSlotTop() + EXC_STEP * func.bc.excLevel();
assert(false);
    int need = (frame().stack + frameSize) - (begin() + base);

    auto curr = base;           // current frame offset
    base = fill;                // new frame offset
    insert(fill, need);         // make room

    auto& f = frame();          // new frame
    f.base = curr;              // index of (now previous) frame
    f.spOff = spOff;            // previous stack index
    f.ipOff = ipOff;            // previous instruction index
    f.callee = callee;          // previous callable

    spOff = f.stack-begin()-1;  // stack starts out empty
    ipOff = 0;                  // code starts at first opcode
    callee = &func;             // new callable context
    f.ep = 0;                   // no exceptions pending
}

Value Context::leave (Value v) {
    auto& f = frame();
    auto r = f.result;          // stored result
    if (r.isNil())              // use return result if set
        r = v;                  // ... else arg

    if (base > NumSlots) {
        int prev = f.base;      // previous frame offset
        spOff = f.spOff;        // restore stack index
        ipOff = f.ipOff;        // restore instruction index
        callee = &f.callee.asType<Callable>(); // restore callee

        assert(fill > base);
        remove(base, fill - base); // delete current frame

        assert(prev >= 0);
        base = prev;            // new lower frame offset
    } else {
        // last frame, drop context, restore caller
        Interp::context = caller().ifType<Context>();
        auto n = Interp::findTask(*this);
        if (n >= 0)
            Interp::tasks.remove(n);

        fill = NumSlots; // delete stack entries
        adj(NumSlots); // release vector
    }

    return r;
}

#if 0 //XXX
auto Context::excBase (int incr) -> Value* {
    uint32_t ep = frame().ep;
    frame().ep = ep + incr;
    if (incr <= 0)
        --ep;
    return frame().stack + callee->bc.fastSlotTop() + EXC_STEP * ep;
}
#endif

void Context::raise (Value exc) {
    if (Interp::context == nullptr) {
#if 0 //XXX
        Buffer buf; // TODO wrong place: bail out and print exception details
        buf.print("uncaught exception: ");
        exc.obj().repr(buf);
        buf.putc('\n');
#else
        assert(false);
#endif
        return;
    }

    uint32_t num = 0;
    if (exc.isInt())
        num = exc;              // trigger soft-irq 1..31 (interrupt-safe)
    else
        event() = exc;          // trigger exception or other outer-loop req

    Interp::interrupt(num);     // set pending, to force inner loop exit
}

void Context::caught () {
    auto e = event();
    if (e.isNil())
        return; // there was no exception, just an inner loop exit
    event() = {};

    auto& einfo = e.asType<Exception>().extra();
    einfo.ipOff = ipOff;
    einfo.callee = callee;

    if (frame().ep > 0) { // simple exception, no stack unwind
        auto ep = excBase(0);
        ipOff = ep[0] & (FinallyTag - 1);
        spOff = ep[1];
        begin()[++spOff] = e.isNil() ? ep[2] : e;
    } else {
        leave();
        raise(e);
    }
}

auto Context::next () -> Value {
    assert(fill > 0); // can only resume if not ended
    Interp::resume(*this);
    return {}; // no result yet
}

auto Interp::findTask (Context& ctx) -> int {
    for (auto& e : tasks)
        if (&e.obj() == &ctx)
            return &e - tasks.begin();
    return -1;
}

auto Interp::getQueueId () -> int {
    static_assert (NUM_QUEUES <= 8 * sizeof pending, "NUM_QUEUES too large");

    for (uint32_t id = 1; id < NUM_QUEUES; ++id) {
        auto mask = 1U << id;
        if ((queueIds & mask) == 0) {
            queueIds |= mask;
            return id;
        }
    }
    return -1;
}

void Interp::dropQueueId (int id) {
    auto mask = 1U << id;
    assert(queueIds & mask);
    queueIds &= ~mask;

    // deal with tasks pending on this queue
    for (auto e : tasks) {
        auto& ctx = e.asType<Context>();
        if (ctx.qid == id)
            ctx.qid = 0; // make runnable again TODO also raise an exception?
    }
}

void Interp::markAll () {
    //printf("\tgc started ...\n");
    assert(context != nullptr);
    mark(context); // TODO
    assert(findTask(*context) >= 0);
}

void Interp::suspend (uint32_t id) {
    assert(id < NUM_QUEUES);

    assert(context != nullptr);
    assert(findTask(*context) >= 0);
    auto ctx = context;
    context = ctx->caller().ifType<Context>();
    //ctx->caller() = {};

    ctx->qid = id;
}

void Interp::resume (Context& ctx) {
    assert(context != &ctx);
    assert(ctx.caller().isNil());
    ctx.caller() = context;
    context = &ctx;
}

void Interp::exception (Value v) {
    assert(!v.isInt());
    assert(context != nullptr);
    context->raise(v);
}

void Interp::interrupt (uint32_t num) {
    assert(num < NUM_QUEUES);
    // see https://gcc.gnu.org/onlinedocs/gcc/_005f_005fatomic-Builtins.html
    __atomic_fetch_or(&pending, 1U << num, __ATOMIC_RELAXED);
}

auto Interp::nextPending () -> int {
    if (pending != 0)
        for (uint32_t num = 0; num < NUM_QUEUES; ++num)
            if (pendingBit(num))
                return num;
    return -1;
}

auto Interp::pendingBit (uint32_t num) -> bool {
    assert(num < NUM_QUEUES);
    auto mask = 1U << num;
    // see https://gcc.gnu.org/onlinedocs/gcc/_005f_005fatomic-Builtins.html
    return (__atomic_fetch_and(&pending, ~mask, __ATOMIC_RELAXED) & mask) != 0;
}
