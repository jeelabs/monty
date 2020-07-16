// Implementation of stack frame classes, i.e. ResumableObj, FrameObj, Context.

#include "monty.h"

#include <assert.h>

Value BoundMethObj::call (int argc, Value argv[]) const {
    argv[-1] = self; // TODO writes in caller's stack! is this always safe ???
    // FIXME no, it isn't: some calls are (0, 0) and some are with a single
    //  Value arg, passed as &v - need to revisit all lines which do a call()
    // idea: support a "vcall(fmt, arg1, arg2, ...)" - with in-line conversion
    //  of all args to Values, and then allowing the fmt arg to be overwritten
    return meth.obj().call(argc + 1, argv - 1);
}

BytecodeObj& BytecodeObj::create (ModuleObj& mo, int bytes) {
    return *new (bytes) BytecodeObj (mo);
}

void BytecodeObj::mark (void (*gc)(const Object&)) const {
    gc(owner);
    constObjs.markVec(gc);
}

void CallArgsObj::mark (void (*gc)(const Object&)) const {
    gc(bytecode);
    if (posArgs != 0)
        gc(*posArgs);
    if (kwArgs != 0)
        gc(*kwArgs);
}

Value CallArgsObj::call (int argc, Value argv[]) const {
    return call (argc, argv, 0, 0);
}

Value CallArgsObj::call (int argc, Value argv[], DictObj* dp, const Object* retVal) const {
    auto fp = new FrameObj (bytecode, dp, retVal);

    auto ndp = bytecode.n_def_pos; // shorthand, yuck

    // TODO more arg cases, i.e. keyword args
    for (int i = 0; i < bytecode.n_pos; ++i)
        if (i < argc)
            fp->fastSlot(i) = argv[i];
        else if (posArgs != 0 && i < ndp + (int) posArgs->len())
            fp->fastSlot(i) = posArgs->at(i-ndp); // FIXME see verify/args.py !

    if (hasVarArgs())
        fp->fastSlot(bytecode.n_pos + bytecode.n_kwonly) =
            TupleObj::create(TupleObj::info, argc - bytecode.n_pos,
                                                argv + bytecode.n_pos);

    return fp->isCoro() ? fp : Value::nil; // no result yet
}

void ModuleObj::mark (void (*gc)(const Object&)) const {
    DictObj::mark(gc);
    if (init != 0)
        gc(*init);
}

Value ModuleObj::attr (const char* name, Value& self) const {
    self = Value::nil;
    return at(name);
}

void ResumableObj::mark (void (*gc)(const Object&)) const {
    // TODO no need to mark args[], it already belongs to the caller, AFAICT
    if (chain != 0)
        gc(*chain);
}

FrameObj::FrameObj (const BytecodeObj& bco, DictObj* dp, const Object* retVal)
        : bcObj (bco), locals (dp), result (retVal), savedIp (bco.code) {
    if (locals == 0) {
        locals = this;
        chain = &bco.owner;
    }

    ctx = Context::prepare(isCoro());
    spOffset = ctx->extend(bcObj.frameSize()) - 1;

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
    ctx->resume(this);
    return Value::nil; // TODO really?
}

Value* FrameObj::bottom () const {
    return ctx->limit() - bcObj.frameSize();
}

void FrameObj::leave () {
    ctx->shrink(bcObj.frameSize()); // note that the stack could move

    if (ctx->tasks.len() > 0 && this == &ctx->tasks.at(0).asType<FrameObj>())
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

    assert(mod.init != 0);
    CallArgsObj cao (*mod.init); // only needed to set up args in call()
    cao.call(0, 0, &mod, 0);

    assert(fp != 0);
    tasks.append(fp);
}

void Context::doCall (Value func, int argc, Value argv[]) {
    assert(func.isObj());
    Value v = func.obj().call(argc, argv);
    if (v.isNil())
        return;

    auto p = v.ifType<ResumableObj>();
    if (p != 0) {
        auto ofp = fp;
        // TODO also break if pending != 0, inner() may have to step() on entry!
        while (true)
            if (p->step(p->retVal)) {
                auto q = p->retVal.ifType<ResumableObj>();
                if (q != 0) {
                    q->chain = p;
                    p = q;
                } else if (fp != ofp) { // leave now, prepare to resume later
                    assert(fp->result == 0);
                    fp->result = p;
                    return;
                }
            } else if (p-> chain != 0)
                p = p->chain;
            else
                break;
        assert(fp->result == 0);
        v = p->retVal;
        assert(v.isNil()); // TODO is this just to clear it ???
    }

    if (!v.isNil() && sp >= fp->bottom()) // TODO why can sp be below bottom?
        *sp = v;
}

Context* Context::prepare (bool coro) {
    return coro ? new Context : vm->fp != 0 ? vm->fp->ctx : vm;
}

// raise() quits inner vm loop
// raise(int n) triggers handler, 1..MAX_HANDLERS-1
// raise(str|obj) raises an exception, i.e triggers slot 0
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
        raise(); // exit the vm loop
    caller = 0; // cleared to flag as suspended if this is a coro
}

void Context::suspend (ListObj& queue) {
    assert(vm->fp != 0 && tasks.len() > 0);
    Value v = tasks.at(0);
    auto& fo = v.asType<FrameObj>();

    auto top = vm->fp;
    while (top->caller != 0)
        top = top->caller;
    assert(top == &fo);

    fo.caller = vm->flip(0);

    queue.append(v);
    tasks.pop(0);
    raise(); // exit inner vm loop
}

void Context::wakeUp (Value task, Value retVal) {
    auto& fo = task.asType<FrameObj>();
    if (!retVal.isNil()) {
        assert(fo.result == 0 && retVal.isObj());
        // TODO also deal with ResumableObj's here?
        fo.result = &retVal.obj();
    }
    tasks.append(task);
}

void Context::resume (Value v) {
    auto& fo = v.asType<FrameObj>();
    fo.caller = flip(fo.caller != 0 ? fo.caller : &fo);
    if (fp != 0 && fp->result != 0) {
        assert(sp >= fp->bottom());
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
