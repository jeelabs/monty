// data.cpp - basic object data types

#include "monty.h"
#include <cassert>

using namespace monty;
//using monty::Q;

None const None::noneObj;
Bool const Bool::falseObj;
Bool const Bool::trueObj;

Value const monty::Null  {None::noneObj};
Value const monty::False {Bool::falseObj};
Value const monty::True  {Bool::trueObj};

Type const Object::info (Q(166,"<object>"));

Lookup const Bool::attrs;
Lookup const Int::attrs;
Lookup const Range::attrs;
Lookup const Slice::attrs;

constexpr int QID_RAM_BASE = 32*1024;
constexpr int QID_RAM_LAST = 48*1024;

static VaryVec qstrBaseMap (qstrBase, qstrBaseLen);
static VaryVec qstrRamMap;

void monty::qstrCleanup () {
    qstrRamMap.clear();
}

auto Q::hash (void const* p, uint32_t n) -> uint32_t {
    // see http://www.cse.yorku.ca/~oz/hash.html
    uint32_t h = 5381;
    for (uint32_t i = 0; i < n; ++i)
        h = ((h<<5) + h) ^ ((uint8_t const*) p)[i];
    return h;
}

auto Q::str (uint16_t i) -> char const* {
    assert(i != 0);
    auto s = i < QID_RAM_BASE ? qstrBaseMap.atGet(i)
                               : qstrRamMap.atGet(i - QID_RAM_BASE);
    return (char const*) s;
}

static auto qstrFind (VaryVec const& v, char const* s, uint8_t h) -> uint16_t {
    if (v.size() > 0) {
        auto p = v.atGet(0);
        auto n = v.atLen(0);
        for (uint32_t i = 0; i < n; ++i)
            if (h == p[i] && strcmp(s, (char const*) v.atGet(i+1)) == 0)
                return i+1;
    }
    return 0;
}

auto Q::find (char const* s) -> uint16_t {
    uint8_t h = hash(s, strlen(s));
    auto i = qstrFind(qstrBaseMap, s, h);
    if (i > 0)
        return i;
    auto j = qstrFind(qstrRamMap, s, h);
    if (j > 0)
        return j + QID_RAM_BASE;
    return 0;
}

auto Q::last () -> uint16_t {
    return qstrRamMap.size() - 1 + QID_RAM_BASE;
}

auto Q::make (char const* s) -> uint16_t {
    auto i = find(s);
    if (i > 0)
        return i;
    auto n = strlen(s);
    auto& v = qstrRamMap; // shorthand
    if (v.size() == 0)
        v.insert(0); // empty hash map
    i = v.atLen(0);
    v.atAdj(0, i + 1);
    v.atGet(0)[i++] = hash(s, n);
    v.insert(i);
    v.atSet(i, s, n+1);
    return i + QID_RAM_BASE;
}

Value::Value (char const* arg) : _v ((uintptr_t) arg * 4 + 2) {
    auto id = Q::find(arg);
    if (id > 0)
        _v = id * 4 + 2;
    else if (Vec::isInPool(arg)) // don't store ptrs into movable vector space
        *this = new struct Str (arg);
    else
        assert((char const*) *this == arg); // watch out for address truncation
}

Value::Value (E exc, char const* arg1, Value arg2) {
    Value v [] {arg1, arg2};
    auto n = *arg1 == 0 ? 0 : arg2.isNil() ? 1 : 2;
    *this = Exception::create(exc, {v, n});
    Stacklet::exception(*this);
}

Value::operator char const* () const {
    if (!isStr())
        return asType<struct Str>();
    auto p = _v >> 2;
    if (p < QID_RAM_LAST)
        return Q::str(p);
    return (char const*) p;
}

auto Value::asObj () const -> Object& {
    switch (tag()) {
        case Nil: return Null.obj();
        case Int: return *new struct Int (*this);
        case Str: return *new struct Str (*this);
        case Obj: break;
    }
    return obj();
}

auto Value::asInt () const -> int64_t {
    return isInt() ? (int) *this : (int64_t) asType<struct Int>();
}

auto Value::asQid () const -> uint16_t {
    if (isStr()) {
        auto p = _v >> 2;
        if (p < QID_RAM_LAST)
            return p;
    }
    return 0;
}

auto Value::asBool (bool f) -> Value {
    return f ? True : False;
}

auto Value::truthy () const -> bool {
    switch (tag()) {
        case Nil: break;
        case Int: return (int) *this != 0;
        case Str: return *(char const*) *this != 0;
        case Obj: return obj().unop(UnOp::Boln).isTrue();
    }
    return false;
}

auto Value::operator== (Value rhs) const -> bool {
    if (_v == rhs._v)
        return true;
    if (tag() == rhs.tag())
        switch (tag()) {
            case Nil: // fall through
            case Int: return false;
            case Str: return strcmp(*this, rhs) == 0;
            case Obj: return obj().binop(BinOp::Equal, rhs).truthy();
        }
    return false;
}

auto Value::unOp (UnOp op) const -> Value {
    switch (tag()) {
        case Int: {
            int n = *this;
            switch (op) {
                case UnOp::Intg: // fall through
                case UnOp::Pos:  // fall through
                case UnOp::Hash: return *this;
                case UnOp::Abso: if (n > 0) return *this;
                                 // else fall through
                case UnOp::Neg:  return Int::make(-n);
                case UnOp::Inv:  return ~n;
                case UnOp::Not:  return asBool(!n);
                case UnOp::Boln: return asBool(n);
            }
            break;
        }
        case Str: {
            char const* s = *this;
            switch (op) {
                case UnOp::Boln: return asBool(*s);
                case UnOp::Hash: return Q::hash(s, strlen(s));
                default:         break;
            }
            break;
        }
        default: break;
    }

    return obj().unop(op);
}

auto Value::binOp (BinOp op, Value rhs) const -> Value {
    switch (op) {
        case BinOp::In:        return rhs.binOp(BinOp::Contains, *this);
        case BinOp::More:      return rhs.binOp(BinOp::Less, *this);
        case BinOp::LessEqual: return rhs.binOp(BinOp::Less, *this).invert();
        case BinOp::MoreEqual: return binOp(BinOp::Less, rhs).invert();
        case BinOp::NotEqual:  return binOp(BinOp::Equal, rhs).invert();
        default:               break;
    }

    if (isInt() && rhs.isInt()) {
        auto l = (int) *this, r = (int) rhs;
        switch (op) {
            case BinOp::Less:
                return asBool(l < r);
            case BinOp::Equal:
                return asBool(l == r);
            case BinOp::Or: case BinOp::InplaceOr:
                return l | r;
            case BinOp::Xor: case BinOp::InplaceXor:
                return l ^ r;
            case BinOp::And: case BinOp::InplaceAnd:
                return l & r;
            case BinOp::Lshift: case BinOp::InplaceLshift:
                return Int::make((int64_t) l << r);
            case BinOp::Rshift: case BinOp::InplaceRshift:
                return l >> r;
            case BinOp::Add: case BinOp::InplaceAdd:
                return l + r;
            case BinOp::Subtract: case BinOp::InplaceSubtract:
                return l - r;
            case BinOp::Multiply: case BinOp::InplaceMultiply:
                return Int::make((int64_t) l * r);
            case BinOp::TrueDivide: case BinOp::InplaceTrueDivide:
                // TODO needs floats, fall through
            case BinOp::FloorDivide: case BinOp::InplaceFloorDivide:
                if (r == 0)
                    return E::ZeroDivisionError;
                return l / r;
            case BinOp::Modulo: case BinOp::InplaceModulo:
                if (r == 0)
                    return E::ZeroDivisionError;
                return l % r;
            default:
                break;
        }
    }

    return asObj().binop(op, rhs);
}

auto Value::check (Type const& t) const -> bool {
    return isObj() && &_o->type() == &t;
}

void Value::verify (Type const& t) const {
    auto f = check(t);
    if (!f) {
        dump("verify?");
        Value v = t;
        v.dump(t._name);
    }
    assert(f);
}

// non-recursive version for debugging, does not affect the VM state
void Value::dump (char const* msg) const {
    if (msg != 0)
        printf("%s ", msg);
    auto qid = asQid();
    if (qid != 0)
        printf("<Q %d \"%s\">", qid, (char const*) *this);
    else
        switch (tag()) {
            case Value::Nil: printf("<N>"); break;
            case Value::Int: printf("<I %d>", (int) *this); break;
            case Value::Str: printf("<S \"%s\">", (char const*) *this); break;
            case Value::Obj: printf("<O %s at %p>",
                                    (char const*) obj().type()._name, &obj());
                            break;
        }
    if (msg != 0)
        printf("\n");
}

auto Object::call (ArgVec const&) const -> Value {
    Value v = this; v.dump("call?"); assert(false);
    return {};
}

auto Object::unop (UnOp op) const -> Value {
    printf("op %d ", op);
    Value v = this; v.dump("unop?"); assert(false);
    return {};
}

auto Object::binop (BinOp op, Value v2) const -> Value {
    printf("op %d ", op); v2.dump("v2");
    Value v = this; v.dump("binop?"); assert(false);
    return {};
}

auto Object::attr (Value name, Value& self) const -> Value {
    self = this;
    return type().getAt(name);
}

auto Object::len () const -> uint32_t {
    Value v = this; v.dump("len?"); assert(false);
    return {};
}

auto Object::getAt (Value) const -> Value {
    Value v = this; v.dump("getAt?"); assert(false);
    return {};
}

auto Object::setAt (Value, Value) -> Value {
    Value v = this; v.dump("setAt?"); assert(false);
    return {};
}

auto Object::iter () const -> Value {
    assert(false);
    return {};
}

auto Object::next () -> Value {
    assert(false);
    return {};
}

auto Object::copy (Range const&) const -> Value {
    assert(false);
    return {};
}

auto Object::store (Range const&, Object const&) -> Value {
    assert(false);
    return {};
}

auto Object::sliceGetter (Value k) const -> Value {
    auto ks = k.ifType<Slice>();
    if (ks == nullptr)
        return {E::TypeError, "index not int or slice", k};
    return copy(ks->asRange(len()));
}

auto Object::sliceSetter (Value k, Value v) -> Value {
    auto ks = k.ifType<Slice>();
    if (ks == nullptr)
        return {E::TypeError, "index not int or slice", k};
    auto const& r = ks->asRange(len());
    if (r._by != 1)
        return {E::NotImplementedError, "assign to extended slice", k};
    return store(r, v.obj());
}

void Object::repr (Buffer& buf) const {
    buf.print("<%s at %p>", (char const*) type()._name, this);
}

auto None::binop (BinOp op, Value rhs) const -> Value {
    if (op == BinOp::Equal)
        return Value::asBool(rhs.isNone());
    assert(false);
    return {}; // TODO
}

void None::repr (Buffer& buf) const {
    buf << "null"; // JSON ...
}

auto Bool::unop (UnOp op) const -> Value {
    switch (op) {
        case UnOp::Not:  return Value::asBool(this != &trueObj);
        case UnOp::Intg: // fall through
        case UnOp::Hash: return this == &trueObj;
        case UnOp::Boln: return *this;
        default:         break;
    }
    return Object::unop(op);
}

auto Bool::create (ArgVec const& args, Type const*) -> Value {
    if (args.size() == 1)
        return args[0].unOp(UnOp::Boln);
    assert(args.size() == 0);
    return False;
}

void Bool::repr (Buffer& buf) const {
    buf << (this == &falseObj ? "false" : "true");
}

auto Int::make (int64_t i) -> Value {
    Value v = (int) i;
    return i == (int) v ? v : new Int (i);
}

auto Int::conv (char const* s) -> Value {
    return make(strtoll(s, nullptr, 10));
}

auto Int::unop (UnOp op) const -> Value {
    // TODO use templates to share code with Value::unOp ?
    switch (op) {
        case UnOp::Intg: // fall through
        case UnOp::Pos:  return *this;
        case UnOp::Hash: return Q::hash(&_i64, sizeof _i64);
        case UnOp::Abso: if (_i64 > 0) return *this; // else fall through
        case UnOp::Neg:  return make(-_i64);
        case UnOp::Inv:  return make(~_i64);
        case UnOp::Not:  return Value::asBool(_i64 == 0);
        case UnOp::Boln: return Value::asBool(_i64 != 0);
    }
    assert(false);
    return {};
}

auto Int::binop (BinOp op, Value rhs) const -> Value {
    // TODO use templates to share code with Value::binOp ?
    auto r64 = rhs.asInt();
    switch (op) {
        case BinOp::Less:
            return Value::asBool(_i64 < r64);
        case BinOp::Equal:
            return Value::asBool(_i64 == r64);
        case BinOp::Or: case BinOp::InplaceOr:
            return make(_i64 | r64);
        case BinOp::Xor: case BinOp::InplaceXor:
            return make(_i64 ^ r64);
        case BinOp::And: case BinOp::InplaceAnd:
            return make(_i64 & r64);
        case BinOp::Lshift: case BinOp::InplaceLshift:
            return make(_i64 << r64);
        case BinOp::Rshift: case BinOp::InplaceRshift:
            return make(_i64 >> r64);
        case BinOp::Add: case BinOp::InplaceAdd:
            return make(_i64 + r64);
        case BinOp::Subtract: case BinOp::InplaceSubtract:
            return make(_i64 - r64);
        case BinOp::Multiply: case BinOp::InplaceMultiply:
            return make(_i64 * r64);
        case BinOp::TrueDivide: case BinOp::InplaceTrueDivide:
            // TODO needs floats, fall through
        case BinOp::InplaceFloorDivide: case BinOp::FloorDivide:
            if (r64 == 0)
                return E::ZeroDivisionError;
            return _i64 / r64;
        case BinOp::InplaceModulo: case BinOp::Modulo:
            if (r64 == 0)
                return E::ZeroDivisionError;
            return _i64 % r64;
        default:
            break;
    }
    assert(false);
    return {}; // TODO
}

auto Int::create (ArgVec const& args, Type const*) -> Value {
    assert(args.size() == 1);
    auto v = args[0];
    switch (v.tag()) {
        case Value::Nil: // fall through
        case Value::Int: return v;
        case Value::Str: return Int::conv(v);
        case Value::Obj: return v.unOp(UnOp::Intg);
    }
    return {};
}

void Int::repr (Buffer& buf) const {
    uint64_t val = _i64;
    if (_i64 < 0) {
        buf.putc('-');
        val = -_i64;
    }

    // need to print in pieces which fit into a std int
    int v1 = val / 1000000000000;
    int v2 = (val / 1000000) % 1000000;
    int v3 = val % 1000000;

    if (v1 > 0)
        buf.print("%d%06d%06d", v1, v2, v3);
    else if (v2 > 0)
        buf.print("%d%06d", v2, v3);
    else
        buf.print("%d", v3);
}

auto RawIter::operator!= (RawIter const&) -> bool {
    if (_val.isNil() && _pos.isOk())
        _val = stepper();
    return _val.isOk();
}

auto RawIter::stepper () -> Value {
    if (_pos.isInt()) {
        uint32_t n = _pos;
        assert(_obj.isObj());
        if (n >= _obj->len())
            return {};
        // TODO better would be: if derived from Tuple, i.e. based on Vector
        _pos = n + 1;
        if (&_obj->type() == &Dict::info || &_obj->type() == &Set::info)
            return ((List&) _obj.obj())[n]; // avoid keyed access
        return _obj->getAt(n);
    }
    auto ctx = Stacklet::current;
    auto v = _pos->next();
    if (v.isOk() || Stacklet::current == ctx)
        return v;
    Stacklet::current = ctx;
    return Stacklet::suspend();
}

auto Range::len () const -> uint32_t {
    assert(_by != 0);
    auto n = (_to - _from + _by + (_by > 0 ? -1 : 1)) / _by;
    return n < 0 ? 0 : n;
}

auto Range::create (ArgVec const& args, Type const*) -> Value {
    assert(1 <= args.size() && args.size() <= 3);
    int a = args.size() > 1 ? (int) args[0] : 0;
    int b = args.size() == 1 ? args[0] : args[1];
    int c = args.size() > 2 ? (int) args[2] : 1;
    return new Range (a, b, c);
}

void Range::repr (Buffer& buf) const {
    buf.print("range(%d,%d,%d)", _from, _to, _by);
}

auto Slice::asRange (int sz) const -> Range {
    int from = _off.isInt() ? (int) _off : 0;
    int to = _num.isInt() ? (int) _num : sz;
    int by = _step.isInt() ? (int) _step : 1;
    if (from < 0)
        from += sz;
    if (to < 0)
        to += sz;
    if (by < 0) {
        auto t = from - 1;
        from = to - 1;
        to = t;
    }
    return {from, to, by};
}

auto Slice::create (ArgVec const& args, Type const*) -> Value {
    assert(1 <= args.size() && args.size() <= 3);
    Value a = args.size() > 1 ? args[0] : Null;
    Value b = args.size() == 1 ? args[0] : args[1];
    Value c = args.size() > 2 ? args[2] : Null;
    return new Slice (a, b, c);
}

void Slice::repr (Buffer& buf) const {
    buf << "slice(" << _off << ',' << _num << ',' << _step << ')';
}
