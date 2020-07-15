// Implementation of the main datatypes, i.e. Value, TypeObj, FrameObj, Context.

#include "monty.h"

#include <assert.h>

void ResumableObj::mark (void (*gc)(const Object&)) const {
    // TODO no need to mark args[], it already belongs to the caller, AFAICT
    if (chain != 0)
        gc(*chain);
}

FrameObj::FrameObj (const BytecodeObj& bco, int argc, Value argv[], DictObj* dp)
        : bcObj (bco), locals (dp), savedIp (bco.code) {
    if (locals == 0) {
        locals = this;
        chain = &bco.owner;
    }

    // TODO many more arg cases, also default args
    argv = Context::prepareStack (*this, argv);
    for (int i = 0; i < bcObj.n_pos; ++i)
        fastSlot(i) = argv[i];

    if (!isCoro())
        caller = ctx->flip(this);
}

void FrameObj::mark (void (*gc)(const Object&)) const {
    DictObj::mark(gc);

    gc(bcObj);
    gc(*locals);
    if (caller != 0) gc(*caller);
    if (ctx != 0)    gc(*ctx);
    if (result != 0) gc(*result);
}

Value FrameObj::next () {
    ctx->resume(*this);
    return Value::nil; // TODO really?
}

Value* FrameObj::bottom () const {
    return ctx->limit() - bcObj.frameSize();
}

void FrameObj::leave () {
    ctx->shrink(bcObj.frameSize()); // note that the stack could move

    assert(ctx->tasks.len() > 0);
    if (this == ctx->tasks.at(0).asType<FrameObj>())
        ctx->tasks.pop(0);

    if (!isCoro()) // don't delete frame on return, it may have a reference
        delete this;
}

volatile uint32_t Context::pending;
Value Context::handlers [MAX_HANDLERS];
Context* Context::vm;
ListObj Context::tasks (0, 0);

void Context::mark (void (*gc)(const Object&)) const {
    markVec(gc);
    if (fp != 0)
        gc(*fp);
    if (this == vm)
        for (auto& h : handlers)
            if (h.isObj())
                gc(h.obj());
}

int Context::extend (int num) {
    auto n = length();
    saveState(); // Context::sp may change due the a realloc in ins()
    ins(n, num);
    restoreState();
    return n;
}

void Context::shrink (int num) {
    saveState(); // Context::sp may change due the a realloc in del()
    del(length() - num, num);
    restoreState();
}

void Context::start (ModuleObj& mod, const LookupObj& builtins) {
    mod.chain = &builtins;
    mod.atKey("__name__", DictObj::Set) = "__main__";
    mod.call(0, 0);

    assert(fp != 0);
    tasks.append(fp);
}

void Context::doCall (int argc, Value argv[]) {
    Value v = sp->obj().call(argc, argv);
    if (v.isNil())
        return;
    auto p = v.asType<ResumableObj>();
    if (p == 0)
        *sp = v;
    else {
        auto ofp = fp;
        // TODO also break if pending != 0, inner() may have to step() on entry!
        while (true)
            if (p->step(p->retVal)) {
                auto q = p->retVal.asType<ResumableObj>();
                if (q != 0) {
                    q->chain = p;
                    p = q;
                } else if (fp != ofp) { // need to leave, prepare to resume later
                    assert(fp->result == 0);
                    fp->result = p;
                    return;
                }
            } else if (p-> chain != 0)
                p = p->chain;
            else
                break;
        assert(fp->result == 0);
        // TODO if retVal not nil, store in *sp ?
    }
}

Value* Context::prepareStack (FrameObj& fo, Value* argv) {
    // TODO yuck, but FrameObj needs a stack (too) early on ...
    //  the problem is argv, which points either lower into this same stack
    //  (regular calls), or into the parent cor stack (then it won't move)
    auto sv = vm->fp != 0 ? vm->fp->ctx : vm;

    // TODO this is the only (?) place where the stack may be reallocated and
    // since argv points into it, it needs to be relocated when this happens
    // note: base() can't be called if there is no stack, i.e. no argv passed

    if (fo.isCoro()) {
        fo.ctx = new Context;
        fo.spOffset = fo.ctx->extend(fo.bcObj.frameSize()) - 1;
        assert(fo.spOffset == -1);
        return argv; // FIXME vectors can't have moved here, right?
    }

    fo.ctx = sv;
    int off = argv != 0 ? argv - sv->base() : 0; // TODO argc zero? yuck
    fo.spOffset = fo.ctx->extend(fo.bcObj.frameSize()) - 1;
    return sv->base() + off;
}

// raise(int n) triggers handler, 1..MAX_HANDLERS-1
// raise(str|obj) raises an exception, i.e triggers slot 0
// raise(nil) quits inner vm loop
void Context::raise (Value e) {
    assert(vm != 0);

    int slot = 0;
    if (e.isInt()) {
        slot = e;
        assert(0 < slot && slot < (int) MAX_HANDLERS);
        assert(!handlers[slot].isNil());
    } else
        handlers[0] = e;

    // this spinloop correctly sets one bit in volatile "pending" state
    do // potential race when an irq raises *inside* the "pending |= ..."
        pending |= 1<<slot;
    while ((pending & (1<<slot)) == 0);
}

Value Context::nextPending () {
    for (size_t slot = 0; slot < MAX_HANDLERS; ++slot)
        if (pending & (1<<slot)) {
            do // again a spinloop, see notes above in raise()
                pending &= ~(1<<slot);
            while (pending & (1<<slot));
            return handlers[slot];
        }

    return Value::nil;
}

int Context::setHandler (Value h) {
    if (h.isInt()) {
        int i = h;
        if (1 <= i && i < (int) MAX_HANDLERS)
            handlers[i] = Value::nil;
        return 0;
    }

    for (int i = 1; i < (int) MAX_HANDLERS; ++i)
        if (handlers[i].isNil()) {
            handlers[i] = h;
            return i;
        }

    return -1;
}

// TODO - This code still has a strange design quirk: the vm.run() called at
//  startup is the one which keeps running across coro context switches, by
//  copying the relevant state to this first instance - it would make much
//  more sense to have a single global "ctx" (iso "vm") which points to the
//  currently *running* coro/vm instance. This probably means that the inner
//  loop in the vm should return, and the outer loop should be static and
//  resume the proper context/coro/vm instance in every switch. The benefit
//  will be less state-saving/-restoring, and perhaps faster context switches.
// TODO - Will need more review to support multiple threads, and even cores.

void Context::saveState () {
    auto frame = vm->fp;
    if (frame != 0) {
        frame->savedIp = vm->ip;
        frame->spOffset = vm->sp - frame->ctx->base();
    }
}

void Context::restoreState () {
    auto frame = vm->fp;
    vm->ip = frame != 0 ? frame->savedIp : 0;
    vm->sp = frame != 0 ? frame->ctx->base() + frame->spOffset : 0;
}

FrameObj* Context::flip (FrameObj* frame) {
    assert(vm->fp != frame);
    auto fp = vm->fp;
    if (fp != 0)
        saveState();
    vm->fp = frame;
    restoreState();
    return fp;
}

void Context::popState () {
    assert(fp != 0);
    auto& caller = fp->caller; // TODO messy, fp will change in flip
    flip(caller);
    if (caller == 0)
        raise(Value::nil); // exit the vm loop
    caller = 0; // cleared to flag as suspended if this is a coro
}

void Context::suspend (ListObj& queue) {
    assert(vm->fp != 0 && tasks.len() > 0);
    Value v = tasks.at(0);
    auto fp = v.asType<FrameObj>();
    assert(fp != 0);

    auto top = vm->fp;
    while (top->caller != 0)
        top = top->caller;
    assert(top == fp);

    fp->caller = vm->flip(0);

    queue.append(v);
    tasks.pop(0);
    raise(Value::nil); // exit inner vm loop
}

void Context::wakeUp (Value task, Value retVal) {
    auto fp = task.asType<FrameObj>();
    assert(fp != 0);
    if (!retVal.isNil()) {
        assert(fp->result == 0 && retVal.isObj());
        // TODO also deal with ResumableObj's here?
        fp->result = &retVal.obj();
    }
    tasks.append(task);
}

void Context::resume (FrameObj& frame) {
    frame.caller = flip(frame.caller != 0 ? frame.caller : &frame);
    if (fp != 0 && fp->result != 0) {
        *sp = fp->result;
        fp->result = 0;
    }
}

bool Context::isAlive () const {
    if (pending != 0 || tasks.len() > 0)
        return true;
    for (size_t i = 1; i < MAX_HANDLERS; ++i) // TODO 1? or 0?
        if (!handlers[i].isNil())
            return true;
    return false;
}