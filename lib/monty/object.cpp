// Implementation of the main datatypes, i.e. Value, ArrayObj, DictObj, TypeObj.

#include "monty.h"
#include "defs.h"

#include <assert.h>
#include <string.h>

bool Value::check (const TypeObj& t) const {
    return isObj() && &obj().type() == &t;
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
    auto cao = argv[0].asType<CallArgsObj>();
    assert(cao != 0);
    atKey("__name__", DictObj::Set) = argv[1];
    cao->call (argc, argv, this, this);
}

Value InstanceObj::create (const TypeObj& type, int argc, Value argv[]) {
    return new InstanceObj ((const ClassObj&) type, argc, argv);
}

InstanceObj::InstanceObj (const ClassObj& parent, int argc, Value argv[]) {
    chain = &parent;

    Value self = Value::nil;
    Value init = attr("__init__", self);
    if (!init.isNil()) {
        argv[-1] = this; // TODO is this alwats ok ???
        auto cao = init.asType<CallArgsObj>();
        assert(cao != 0);
        cao->call(argc + 1, argv - 1, this, this);
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
    const void* ptr;
    if (n <= MAX_NOVEC) {
        // short data *overwrites* the Vector in this struct
        auto& p = noVec();
        p.flag = 1; // never possible in a vector (assuming little-endian!)
        p.size = n;
        ptr = p.bytes;
    } else {
        ins(0, n);
        ptr = getPtr(0);
    }
    if (p != 0)
        memcpy((void*) ptr, p, n);
    else
        memset((void*) ptr, 0, n);
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

void* BytesObj::operator new (size_t sz, size_t len) {
    // this deals with two ways to store the data, either in a Vector or inline
    if (len > MAX_NOVEC)
        return Object::operator new (sz);
    // for storing data inline, i.e. replacing Vector, see BytesObj::BytesObj
    // the size must be large enough to make the dummy Vector constructor happy
    return Object::operator new (sz, sizeof (SeqObj) + sizeof (NoVec) - sz);
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
        auto& a = argv[0].obj();
        if (&a.type() == &StrObj::info) {
            auto& o = (StrObj&) a;
            p = (const char*) o;
            n = o.len();
        } else if (&a.type() == &BytesObj::info) {
            auto& o = (BytesObj&) a;
            p = (const uint8_t*) o;
            n = o.len();
        } else
            assert(false); // TODO iterables
    }
    return new (n) BytesObj (p, n);
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

Value TupleObj::create (const TypeObj&, int argc, Value argv[]) {
    return new (argc * sizeof (Value)) TupleObj (argc, argv);
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

static const char* types = "bBhHiIlL";
static const uint8_t bits [] = { 3, 3, 4, 4, 4, 4, 5, 5, };

Value ArrayObj::create (const TypeObj&, int argc, Value argv[]) {
    assert(argc == 1 && argv[0].isStr() && strlen(argv[0]) == 1);
    return new ArrayObj (*(const char*) argv[0]);
}

ArrayObj::ArrayObj (char t, size_t sz) : atype (t) {
    auto p = strchr(types, t);
    logBits = p != 0 ? bits[p-types] : 3; // overwrites VecOfValue settings
    if (sz > 0) {
        ins(0, sz);
        del(0, sz);
    }
}

void ArrayObj::mark (void (*gc)(const Object&)) const {
    // do NOT call the base class markVec(gc); !
}

size_t ArrayObj::write (const void* p, size_t n) {
    auto w = width();
    if (n > capacity / w - fill)
        n = capacity / w - fill;
    memcpy(getPtr(fill), p, n * w);
    fill += n;
    return n;
}

Value ArrayObj::get (int idx) const {
    return atype & 0x20 ? getInt(idx) : getIntU(idx);
}

void ArrayObj::set (int idx, Value val) {
    Vector::set(idx, (int) val);
}

void  ArrayObj::insert (int, Value) {
    // TODO
}

Value ArrayObj::pop (int) {
    return Value::nil; // TODO
}

void  ArrayObj::remove (Value) {
    // TODO
}

void  ArrayObj::reverse () {
    // TODO
}

bool ArrayObj::Strategy::need (size_t bytes) {
    if (buffer.fill + bytes > buffer.capacity) {
        if (buffer.fill > 0 && !flush())
            return false;
        auto grow = buffer.fill + bytes - buffer.capacity;
        if (grow > 0) {
            buffer.ins(buffer.fill, grow);
            buffer.del(buffer.fill, grow);
        }
    }
    base = (uint8_t*) buffer.getPtr(0);
    assert(room() >= bytes);
    return true;
}

void ArrayObj::Strategy::put (const void* p, size_t n) {
    assert(n <= room());
    memcpy(base + buffer.fill, p, n);
    buffer.fill += n;
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
            return *(Value*) getPtr(i+1);
        }
    if (mode == Set) {
        ins(n, 2);
        set(n, key);
        return *(Value*) getPtr(n+1);
    }
    // result can't be const, but may not be changed
    assert(Value::invalid.isNil());
    return Value::invalid;
}

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

void ModuleObj::mark (void (*gc)(const Object&)) const {
    DictObj::mark(gc);
    if (init != 0)
        gc(*init);
}

Value ModuleObj::call (int argc, Value argv[]) const {
    assert(init != 0);
    auto cao = new CallArgsObj (*init); // TODO move to call site, i.e. main
    return cao->call(argc, argv, (DictObj*) this, 0); // FIXME const!
}

Value ModuleObj::attr (const char* name, Value& self) const {
    self = Value::nil;
    return at(name);
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

    // TODO more arg cases, i.e. *args and keyword args
    for (int i = 0; i < bytecode.n_pos; ++i)
        if (i < argc)
            fp->fastSlot(i) = argv[i];
        else if (posArgs != 0 && i < posArgs->len())
            fp->fastSlot(i) = posArgs->at(i);

    return fp->isCoro() ? fp : Value::nil; // no result yet
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

static const auto mo_format = MethObj::wrap(&StrObj::format);
static const MethObj m_format = mo_format;

static const LookupObj::Item strMap [] = {
    { "count", &m_count }, // TODO can move to SeqObj lookup, once it chains
    { "encode", &m_encode },
    { "format", &m_format },
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
const LookupObj ArrayObj::attrs (0, 0);
