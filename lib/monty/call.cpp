// call.cpp - execution state, stacks, and callables

#include "monty.h"
#include <cassert>

using namespace Monty;

volatile uint32_t Interp::pending;
Context* Interp::context;
List Interp::tasks;
Value Interp::handlers [MAX_HANDLERS];

Callable::Callable (Value bc, Tuple* t, Dict* d, Module* mod)
        : mo (mod != nullptr ? *mod : Interp::context->globals()),
          code (bc.asType<Bytecode>()), pos (t), kw (d) {
}

auto Callable::call (Vector const& vec, int argc, int args) const -> Value {
    auto ctx = Interp::context;
    auto coro = code.isGenerator();
    if (coro)
        ctx = new Context;

    ctx->enter(*this);

    auto nPos = code.numArgs(0);
    auto nDef = code.numArgs(1);
    auto nKwo = code.numArgs(2);

    for (uint32_t i = 0; i < nPos; ++i)
        if ((int) i < argc)
            ctx->fastSlot(i) = vec[args+i];
        else if (pos != nullptr && i < nDef + pos->fill)
            //ctx->fastSlot(i) = (*pos)[(int) (i-nDef)]; // ???
            ctx->fastSlot(i) = (*pos)[i-argc]; // FIXME verify/args.py

    if (code.hasVarArgs())
        ctx->fastSlot(nPos+nKwo) = Tuple::create(vec, argc-nPos, args+nPos);

    // TODO this isn't quite right, inside bc, there's a list of indices ...
    //  but why not lazily turn the first deref load into a new cell?
    //  ... or keep a bitmap of which fast slots are / should be cells?
    for (size_t i = 0; i < code.numCells(); ++i)
        ctx->fastSlot(i) = new Cell (ctx->fastSlot(i));

    return coro ? ctx : Value {};
}

void Callable::marker () const {
    mo.marker();
    mark(code);
    mark(pos);
    mark(kw);
}

auto BoundMeth::call (Vector const& vec, int argc, int args) const -> Value {
    assert(args > 0 && this == &vec[args-1].obj());
    vec[args-1] = self;
    return meth.call(vec, argc + 1, args - 1);
}

Closure::Closure (Callable const& f, Vector const& vec, int argc, int args)
        : func (f) {
    insert(0, argc);
    for (int i = 0; i < argc; ++i)
        begin()[i] = vec[args+i];
}

auto Closure::call (Vector const& vec, int argc, int args) const -> Value {
    auto n = size();
    assert(n > 0);
    Vector v;
    v.insert(0, n + argc);
    for (size_t i = 0; i < n; ++i)
        v[i] = begin()[i];
    for (int i = 0; i < argc; ++i)
        v[n+i] = vec[args+i];
    return func.call(v, n + argc, 0);
}

void Context::enter (Callable const& func) {
    auto frameSize = func.code.fastSlotTop() + EXC_STEP * func.code.excLevel();
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

    if (base > 0) {
        int prev = f.base;      // previous frame offset
        spOff = f.spOff;        // restore stack index
        ipOff = f.ipOff;        // restore instruction index
        callee = &f.callee.asType<Callable>(); // restore callee

        assert(fill > base);
        remove(base, fill - base); // delete current frame

        assert(prev >= 0);
        base = prev;            // new lower frame offset
    } else {
        Interp::context = caller; // last frame, drop context, restore caller
        if (this == &Interp::tasks[0].obj())
            Interp::tasks.pop(0);

        // FIXME ...
        remove(0, fill); // delete last frame
        callee = nullptr;
        caller = nullptr;
    }

    return r;
}

auto Context::excBase (int incr) -> Value* {
    size_t ep = frame().ep;
    frame().ep = ep + incr;
    if (incr <= 0)
        --ep;
    return frame().stack + callee->code.fastSlotTop() + EXC_STEP * ep;
}

void Context::raise (Value exc) {
    uint32_t num = 0;
    if (exc.isInt())
        num = exc;      // trigger soft-irq 1..31 (interrupt-safe)
    else
        event = exc;    // trigger exception or other outer-loop req

    Interp::interrupt(num);     // set pending, to force inner loop exit
}

void Context::caught () {
    auto e = event;
    event = {};
    if (e.isNil())
        return; // there was no exception, just an inner loop exit

    assert(frame().ep > 0); // simple exception, no stack unwind
    auto ep = excBase(0);
    ipOff = ep[0];
    spOff = ep[1];
    begin()[++spOff] = e.isNil() ? ep[3] : e;
}

auto Context::call (Vector const&, int, int) const -> Value {
    assert(false);
    return {};
}

auto Context::next () -> Value {
    Interp::resume(*this);
    return {}; // no result yet
}

void Context::marker () const {
    List::marker();
    event.marker();
    mark(callee);
    mark(caller);
}

int Interp::setHandler (Value h) {
    if (h.isInt()) {
        int i = h;
        if (1 <= i && i < (int) MAX_HANDLERS)
            handlers[i] = {};
        return 0;
    }
    for (int i = 1; i < (int) MAX_HANDLERS; ++i)
        if (handlers[i].isNil()) {
            handlers[i] = h;
            return i;
        }
    return -1;
}

bool Interp::isAlive () const {
    if (context != nullptr || pending != 0 || tasks.len() > 0)
        return true;
    for (size_t i = 1; i < MAX_HANDLERS; ++i)
        if (!handlers[i].isNil())
            return true;
    return false;
}

void Interp::markAll () {
    //printf("\tgc started ...\n");
    mark(context);
    for (size_t i = 0; i < MAX_HANDLERS; ++i)
        handlers[i].marker();
}

void Interp::suspend (List& queue) {
    auto t = tasks.pop(0);
    assert(t.ifType<Context>() == context); (void) t;

    auto n = queue.size(); // TODO use append()
    queue.insert(n);
    queue[n] = context;

    auto ctx = context;
    context = context->caller;
    ctx->caller = nullptr;
}

void Interp::resume (Context& ctx) {
    assert(context != &ctx);
    assert(ctx.caller == nullptr);
    ctx.caller = context;
    context = &ctx;
    interrupt(0); // force inner loop exit
}

void Interp::exception (Value v) {
    assert(!v.isInt());
    assert(context != nullptr);
    context->raise(v);
}

void Interp::interrupt (uint32_t num) {
    assert(num < MAX_HANDLERS);
    // see https://gcc.gnu.org/onlinedocs/gcc/_005f_005fatomic-Builtins.html
    __atomic_fetch_or(&pending, 1U << num, __ATOMIC_RELAXED);
}

auto Interp::nextPending () -> int {
    if (pending != 0)
        for (size_t num = 0; num < MAX_HANDLERS; ++num)
            if (pendingBit(num))
                return num;
    return -1;
}

auto Interp::pendingBit (uint32_t num) -> bool {
    assert(num < MAX_HANDLERS);
    auto mask = 1U << num;
    // see https://gcc.gnu.org/onlinedocs/gcc/_005f_005fatomic-Builtins.html
    return (__atomic_fetch_and(&pending, ~mask, __ATOMIC_RELAXED) & mask) != 0;
}
