// Implementation of the main datatypes, i.e. Value, TypeObj, FrameObj, Context.

#include "monty.h"
#include "defs.h"

#include <assert.h>
#include <string.h>

void Value::check (const TypeObj& t) const {
    assert(isObj() && &obj().type() == &t);
}

bool Value::isEq (Value val) const {
    if (v == val.v)
        return true;
    if (tag() == val.tag())
        switch (tag()) {
            case Nil: assert(false); // handled above
            case Int: return false;  // handled above
            case Str: return strcmp(*this, val) == 0;
            case Obj: break;
        }
    return binOp(BinOp::Equal, val);
}

Value Value::unOp (UnOp op) const {
    return objPtr()->unop(op);
}

Value Value::binOp (BinOp op, Value rhs) const {
    // fast path for common operations, TODO: is this really worth it?
    if (tag() == rhs.tag())
        switch (tag()) {
            case Int: {
                auto l = (int) *this, r = (int) rhs;
                switch (op) {
                    case BinOp::Less: return l < r;
                    case BinOp::More: return l > r;
                    case BinOp::Add:
                    case BinOp::InplaceAdd: return l + r;
                    case BinOp::Subtract: return l - r;
                    case BinOp::Multiply: return l * r;
                    case BinOp::Equal: return l == r;
                    case BinOp::FloorDivide:
                        if (r == 0) {
                            Context::raise("blah"); // TODO
                            return 0;
                        }
                        return l / r;
                    default: break;
                }
                break;
            }
            case Str: {
                auto l = (const char*) *this, r = (const char*) rhs;
                switch (op) {
                    case BinOp::Add: {
                        auto buf = (char*) malloc(strlen(l) + strlen(r) + 1);
                        strcpy(buf, l);
                        strcat(buf, r);
                        return buf;
                    }
                    default:
                        break;
                }
                break;
            }
            default:
                switch (op) {
                    case BinOp::Equal:
                        return id() == rhs.id(); // TODO too strict!
                    default:
                        break;
                }
                break;
        }
    if (op == BinOp::NotEqual)
        return 1; // FIXME, None != True - hardwired for features.py 45
    return objPtr()->binop(op, rhs);
}

const Value Value::nil;
Value Value::invalid;

void VecOfValue::markVec (void (*gc)(const Object&)) const {
    for (size_t i = 0; i < length(); ++i)
        if (get(i).isObj())
            gc(get(i).obj());
}

Value Object::repr   () const                    { assert(false); }
Value Object::call   (int, Value[]) const        { assert(false); }
Value Object::unop   (UnOp) const                { assert(false); }
Value Object::binop  (BinOp, Value) const        { assert(false); }
Value Object::attr   (const char*, Value&) const { assert(false); }
Value Object::at     (Value) const               { assert(false); }
Value Object::iter   () const                    { assert(false); }
Value Object::next   ()                          { assert(false); }

SeqObj const SeqObj::dummy;

const SeqObj& Object::asSeq () const { return SeqObj::dummy; }

const NoneObj NoneObj::noneObj;

const BoolObj BoolObj::trueObj;
const BoolObj BoolObj::falseObj;

Object& ForceObj::operator* () const {
    switch (tag()) {
        case Value::Nil: assert(false); break;
        case Value::Int: return (Object&) i; // lose const
        case Value::Str: return (Object&) s; // lose const
        case Value::Obj: break;
    }
    return obj();
}

Value TypeObj::noFactory (const TypeObj&, int, Value[]) {
    assert(false);
    return Value::nil;
}

Value TypeObj::call (int argc, Value argv[]) const {
    return factory(*this, argc, argv);
}

Value TypeObj::attr (const char* name, Value& self) const {
    self = Value::nil;
    return at(name);
}

Value ClassObj::create (const TypeObj&, int argc, Value argv[]) {
    assert(argc >= 2 && argv[0].isObj() && argv[1].isStr());
    return new ClassObj (argc, argv);
}

ClassObj::ClassObj (int argc, Value argv[])
        : TypeObj (argv[1], InstanceObj::create) {
    auto& bc = (const BytecodeObj&) argv[0].obj();
    atKey("__name__", DictObj::Set) = argv[1];
    auto fp = new FrameObj (bc, argc, argv, this);
    fp->result = this;
}

Value InstanceObj::create (const TypeObj& type, int argc, Value argv[]) {
    return new InstanceObj ((const ClassObj&) type, argc, argv);
}

InstanceObj::InstanceObj (const ClassObj& parent, int argc, Value argv[]) {
    chain = &parent;

    Value self = Value::nil;
    Value init = attr("__init__", self);
    if (!init.isNil()) {
        argv[-1] = this;
        auto& bc = (const BytecodeObj&) init.obj();
        auto fp = new FrameObj (bc, argc + 1, argv - 1, this);
        fp->result = this;
    }
}

Value InstanceObj::attr (const char* key, Value& self) const {
    self = this;
    Value v = at(key);
    if (v.isNil())
        v = chain->attr(key, self);
    return v;
}

Value IntObj::create (const TypeObj&, int argc, Value argv[]) {
    assert(argc == 1);
    switch (argv[0].tag()) {
        case Value::Int: return argv[0];
        case Value::Str: return atoi(argv[0]);
        default: assert(false);
    }
    return Value::nil;
}

BytesObj::BytesObj (const void* p, size_t n) : Vector (8) {
    if (n <= MAX_NOVEC) {
        // short data *overwrites* the Vector in this struct
        auto& p = noVec();
        p.flag = 1; // never possible in a vector (assuming little-endian!)
        p.size = n;
    }
    auto ptr = (void*) (const uint8_t*) *this;
    if (p != 0)
        memcpy(ptr, p, n);
    else
        memset(ptr, 0, n);
}

BytesObj::~BytesObj () {
    // make sure ~Vector doesn't get confused
    if (!hasVec()) {
        data = 0;
        logBits = 8;
        capacity = 0;
        fill = 0;
    }
}

Value BytesObj::create (const TypeObj&, int argc, Value argv[]) {
    assert(argc == 1);
    const void* p = 0;
    size_t n = 0;
    if (argv[0].isInt())
        n = argv[0];
    else if (argv[0].isStr()) {
        p = (const char*) argv[0];
        n = strlen((const char*) p);
    } else {
        assert(argv[0].isObj());
        auto& v = argv[0].obj();
        if (&v.type().info == &StrObj::info) {
            auto& o = argv[0].asType<StrObj>();
            p = (const char*) o;
            n = o.len();
        } else if (&v.type().info == &BytesObj::info) {
            auto& o = argv[0].asType<BytesObj>();
            p = (const uint8_t*) o;
            n = o.len();
        } else
            assert(false); // TODO iterables
    }
    if (n > MAX_NOVEC)
        return new BytesObj (p, n);
    // special case: store data inline, replacing Vector, see BytesObj::BytesObj
    auto m = operator new (sizeof (SeqObj) + 2 + n);
    return new (m) BytesObj (p, n);
}

BytesObj::operator const uint8_t* () const {
    return hasVec() ? (const uint8_t*) getPtr(0) : noVec().bytes;
}

Value BytesObj::at (Value idx) const {
    assert(idx.isInt());
    int i = idx;
    if (i < 0)
        i += len();
    return ((const uint8_t*) *this)[i];
}

Value BytesObj::decode () const {
    return Value::nil; // TODO
}

Value StrObj::create (const TypeObj&, int argc, Value argv[]) {
    assert(argc == 1 && argv[0].isStr());
    return new StrObj (argv[0]);
}

Value StrObj::at (Value idx) const {
    assert(idx.isInt());
    int i = idx;
    if (i < 0)
        i += strlen(s);
    auto buf = (char*) malloc(2);
    buf[0] = s[i];
    buf[1] = 0;
    return buf;
}

Value StrObj::len () const {
    return strlen(s);
}

Value StrObj::encode () const {
    return Value::nil; // TODO
}

Value SeqObj::isIn  (Value) const { assert(false); }
Value SeqObj::plus  (Value) const { assert(false); }
Value SeqObj::times (Value) const { assert(false); }
Value SeqObj::len   ()      const { assert(false); }
Value SeqObj::min   ()      const { assert(false); }
Value SeqObj::max   ()      const { assert(false); }
Value SeqObj::index (Value) const { assert(false); }
Value SeqObj::count (Value) const { assert(false); }

void  MutSeqObj::remove  (Value)      { assert(false); }
void  MutSeqObj::reverse ()           { assert(false); }

void MutSeqObj::mark (void (*gc)(const Object&)) const {
    markVec(gc);
}

void  MutSeqObj::insert  (int idx, Value val) {
    ins(idx);
    set(idx, val);
}

Value MutSeqObj::pop (int idx) {
    assert(len() > 0);
    if (idx < 0)
        idx += len();
    Value v = get(idx);
    del(idx);
    return v;
}

Value TupleObj::create (const TypeObj&, int argc, Value argv[]) {
    auto m = operator new (sizeof (TupleObj) + argc * sizeof (Value));
    return new (m) TupleObj (argc, argv);
}

TupleObj::TupleObj (int argc, Value argv[]) : length (argc) {
    memcpy(vec, argv, length * sizeof (Value));
}

void TupleObj::mark (void (*gc)(const Object&)) const {
    for (int i = 0; i < length; ++i)
        if (vec[i].isObj())
            gc(vec[i].obj());
}

Value TupleObj::at (Value idx) const {
    assert(idx.isInt());
    int i = idx;
    if (i < 0)
        i += length;
    return vec[i];
}

Value ListObj::create (const TypeObj&, int argc, Value argv[]) {
    return new ListObj (argc, argv);
}

ListObj::ListObj (int argc, Value argv[]) {
    ins(0, argc);
    if (argc > 0)
        memcpy(getPtr(0), argv, argc * sizeof *argv);
}

Value ListObj::at (Value idx) const {
    assert(idx.isInt());
    return get(idx);
}

Value IterObj::next () {
    auto& so = seq.obj().asSeq();
    if (pos < so.len())
        return seq.obj().at(pos++);
    return Value::nil; // end of iteration
}

// TODO is this needed, because items might contain GC'd objects?
void LookupObj::mark (void (*gc)(const Object&)) const {
    for (size_t i = 0; i < len; ++i)
        if (vec[i].v != 0)
            gc(*vec[i].v);
}

Value LookupObj::at (Value key) const {
    assert(key.isStr());
    auto s = (const char*) key;
    for (size_t i = 0; i < len; ++i)
        if (strcmp(s, vec[i].k) == 0)
            return vec[i].v;
    return Value::nil;
}

Value DictObj::create (const TypeObj&, int argc, Value argv[]) {
    // arg handling ...
    return new DictObj;
}

void DictObj::mark (void (*gc)(const Object&)) const {
    MutSeqObj::mark(gc);
    if (chain != 0)
        gc(*chain);
}

Value DictObj::at (Value key) const {
    // with Get, the const property can safely be ignored
    Value v = ((DictObj*) this)->atKey(key);
    if (v.isNil() && chain != 0)
        return chain->at(key);
    return v;
}

Value& DictObj::atKey (Value key, Mode mode) {
    auto n = length();
    for (size_t i = 0; i < n; i += 2)
        if (key.isEq(get(i))) {
            if (mode == Del)
                del(i, 2);
            return get(i+1);
        }
    if (mode == Set) {
        ins(n, 2);
        set(n, key);
        return get(n+1);
    }
    // result can't be const, but may not be changed
    assert(Value::invalid.isNil());
    return Value::invalid;
}

Value BoundMethObj::call (int argc, Value argv[]) const {
    assert(false); // TODO
    return Value::nil;
}

BytecodeObj& BytecodeObj::create (ModuleObj& mo, int bytes) {
    auto p = operator new (sizeof (BytecodeObj) + bytes);
    return *new (p) BytecodeObj (mo);
}

void BytecodeObj::mark (void (*gc)(const Object&)) const {
    gc(owner);
    constObjs.markVec(gc);
}

Value BytecodeObj::call (int argc, Value argv[]) const {
    auto fp = new FrameObj (*this, argc, argv);
    return fp->isCoro() ? fp : Value::nil; // no result yet
}

void ModuleObj::mark (void (*gc)(const Object&)) const {
    DictObj::mark(gc);
    if (init != 0)
        gc(*init);
}

Value ModuleObj::call (int argc, Value argv[]) const {
    assert(init != 0);
    new FrameObj (*init, argc, argv, (DictObj*) this); // FIXME loses const
    return Value::nil; // no result yet, this call has only started
}

Value ModuleObj::attr (const char* name, Value& self) const {
    self = Value::nil;
    return at(name);
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
    if (this == &ctx->tasks.at(0).asType<FrameObj>())
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
    auto& fp = v.asType<FrameObj>();

    auto top = vm->fp;
    while (top->caller != 0)
        top = top->caller;
    assert(top == &fp);

    fp.caller = vm->flip(0);

    queue.append(v);
    tasks.pop(0);
    raise(Value::nil); // exit inner vm loop
}

void Context::resume (FrameObj& frame) {
    frame.caller = flip(frame.caller != 0 ? frame.caller : &frame);
}

bool Context::isAlive () const {
    if (pending != 0 || tasks.len() > 0)
        return true;
    for (size_t i = 1; i < MAX_HANDLERS; ++i) // TODO 1? or 0?
        if (!handlers[i].isNil())
            return true;
    return false;
}

static const auto mo_count = MethObj::wrap(&SeqObj::count);
static const MethObj m_count = mo_count;

static const auto mo_decode = MethObj::wrap(&BytesObj::decode);
static const MethObj m_decode = mo_decode;

static const LookupObj::Item bytesMap [] = {
    { "count", &m_count }, // TODO can move to SeqObj lookup, once it chains
    { "decode", &m_decode },
};

const LookupObj BytesObj::attrs (bytesMap, sizeof bytesMap / sizeof *bytesMap);

Value BytesObj::attr (const char* key, Value& self) const {
    self = Value::nil;
    return attrs.at(key);
}

static const auto mo_encode = MethObj::wrap(&StrObj::encode);
static const MethObj m_encode = mo_encode;

static const FunObj f_format = StrObj::format;

static const LookupObj::Item strMap [] = {
    { "count", &m_count }, // TODO can move to SeqObj lookup, once it chains
    { "encode", &m_encode },
    { "format", &f_format },
};

const LookupObj StrObj::attrs (strMap, sizeof strMap / sizeof *strMap);

Value StrObj::attr (const char* key, Value& self) const {
    self = Value::nil;
    return attrs.at(key);
}

static const auto mo_append = MethObj::wrap(&MutSeqObj::append);
static const MethObj m_append = mo_append;

static const LookupObj::Item mutSeqMap [] = {
    { "append", &m_append },
};

const LookupObj ListObj::attrs (mutSeqMap, sizeof mutSeqMap / sizeof *mutSeqMap);

Value ListObj::attr (const char* key, Value& self) const {
    self = Value::nil;
    return attrs.at(key);
}

const LookupObj IntObj::attrs (0, 0);
const LookupObj TupleObj::attrs (0, 0);
const LookupObj DictObj::attrs (0, 0);
const LookupObj ClassObj::attrs (0, 0);
