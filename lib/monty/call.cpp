// call.cpp - execution state, stacks, and callables

#include "monty.h"
#include <cassert>

using namespace Monty;

volatile uint32_t Interp::pending;
uint32_t          Interp::queueIds;
List              Interp::tasks;
Dict              Interp::modules;
Context*          Interp::context;

Callable::Callable (Value callee, Tuple* t, Dict* d, Module* mod)
        : mo (mod != nullptr ? *mod : Interp::context->globals()),
          bc (callee.asType<Bytecode>()), pos (t), kw (d) {
}

auto Callable::call (ArgVec const& args) const -> Value {
    auto ctx = Interp::context;
    auto coro = bc.isGenerator();
    if (coro)
        ctx = new Context;

    ctx->enter(*this);

    int nPos = bc.numArgs(0);
    int nDef = bc.numArgs(1);
    int nKwo = bc.numArgs(2);

    for (int i = 0; i < nPos; ++i)
        if (i < args.num)
            ctx->fastSlot(i) = args[i];
        else if (pos != nullptr && (uint32_t) i < nDef + pos->fill)
            ctx->fastSlot(i) = (*pos)[i+nDef-nPos];

    if (bc.hasVarArgs())
        ctx->fastSlot(nPos+nKwo) =
            Tuple::create({args.vec, args.num-nPos, args.off+nPos});

    // TODO this isn't quite right, inside bc, there's a list of indices ...
    //  but why not lazily turn the first deref load into a new cell?
    //  ... or keep a bitmap of which fast slots are / should be cells?
    for (uint32_t i = 0; i < bc.numCells(); ++i)
        ctx->fastSlot(i) = new Cell (ctx->fastSlot(i));

    return coro ? ctx : Value {};
}

void Callable::marker () const {
    mo.marker();
    mark(bc);
    mark(pos);
    mark(kw);
}

auto BoundMeth::call (ArgVec const& args) const -> Value {
    assert(args.num > 0 && this == &args[-1].obj());
    args[-1] = self; // overwrites the entry before first arg
    return meth.call({args.vec, (int) args.num + 1, (int) args.off - 1});
}

Closure::Closure (Callable const& f, ArgVec const& args)
        : func (f) {
    insert(0, args.num);
    for (int i = 0; i < args.num; ++i)
        begin()[i] = args[i];
}

auto Closure::call (ArgVec const& args) const -> Value {
    int n = size();
    assert(n > 0);
    Vector v;
    v.insert(0, n + args.num);
    for (int i = 0; i < n; ++i)
        v[i] = begin()[i];
    for (int i = 0; i < args.num; ++i)
        v[n+i] = args[i];
    return func.call({v, n + args.num, 0});
}

void Context::enter (Callable const& func) {
    auto frameSize = func.bc.fastSlotTop() + EXC_STEP * func.bc.excLevel();
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

        fill = 0; // delete stack entries
        adj(0); // release vector
    }

    return r;
}

auto Context::excBase (int incr) -> Value* {
    uint32_t ep = frame().ep;
    frame().ep = ep + incr;
    if (incr <= 0)
        --ep;
    return frame().stack + callee->bc.fastSlotTop() + EXC_STEP * ep;
}

void Context::raise (Value exc) {
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

    if (frame().ep > 0) { // simple exception, no stack unwind
        auto ep = excBase(0);
        ipOff = ep[0] & (FinallyTag - 1);
        spOff = ep[1];
        begin()[++spOff] = e.isNil() ? ep[3] : e;
    } else {
        leave({});
        raise(e);
    }
}

auto Context::next () -> Value {
    assert(fill > 0); // can only resume if not ended
    Interp::resume(*this);
    return {}; // no result yet
}

void Context::marker () const {
    List::marker();
    mark(callee);
}

auto Interp::findTask (Context& ctx) -> int {
    for (auto& e : tasks)
        if (&e.obj() == &ctx)
            return &e - tasks.begin();
    return -1;
}

auto Interp::getQueueId () -> int {
    static_assert (MAX_QUEUES <= 8 * sizeof pending, "MAX_QUEUES too large");

    for (uint32_t id = 1; id < MAX_QUEUES; ++id) {
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
    assert(id < MAX_QUEUES);

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
    assert(num < MAX_QUEUES);
    // see https://gcc.gnu.org/onlinedocs/gcc/_005f_005fatomic-Builtins.html
    __atomic_fetch_or(&pending, 1U << num, __ATOMIC_RELAXED);
}

auto Interp::nextPending () -> int {
    if (pending != 0)
        for (uint32_t num = 0; num < MAX_QUEUES; ++num)
            if (pendingBit(num))
                return num;
    return -1;
}

auto Interp::pendingBit (uint32_t num) -> bool {
    assert(num < MAX_QUEUES);
    auto mask = 1U << num;
    // see https://gcc.gnu.org/onlinedocs/gcc/_005f_005fatomic-Builtins.html
    return (__atomic_fetch_and(&pending, ~mask, __ATOMIC_RELAXED) & mask) != 0;
}
