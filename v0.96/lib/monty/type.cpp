// type.cpp - basic object types and type system

#include "monty.h"
#include "ops.h"
#include <cassert>

using namespace Monty;
using Monty::Q;

None const None::nullObj;
Bool const Bool::falseObj;
Bool const Bool::trueObj;
Tuple const Tuple::emptyObj;

Value const Monty::Null  {None::nullObj};
Value const Monty::False {Bool::falseObj};
Value const Monty::True  {Bool::trueObj};
Value const Monty::Empty {Tuple::emptyObj};

constexpr int QID_RAM_BASE = 32*1024;
constexpr int QID_RAM_LAST = 48*1024;

static VaryVec qstrBaseMap (qstrBase, qstrBaseLen);
static VaryVec qstrRamMap;

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

Value::Value (char const* arg) : v ((uintptr_t) arg * 4 + 2) {
    if (Vec::inPool(arg)) // don't store pointers into movable vector space
#if 0
        *this = new struct Str (arg); // TODO should try Q::find first
#else
        v = Q::make(arg) * 4 + 2;
#endif
    else
        assert((char const*) *this == arg); // watch out for address truncation
}

Value::Value (E exc, Value arg1, Value arg2) {
    Vector v;
    v.insert(0, 2);
    v[0] = arg1;
    v[1] = arg2;
    auto nargs = arg1.isNil() ? 0 : arg2.isNil() ? 1 : 2;
    *this = Exception::create(exc, {v, nargs, 0});
    Interp::exception(*this);
}

Value::operator char const* () const {
    if (!isStr())
        return asType<struct Str>();
    auto p = v >> 2;
    if (p < QID_RAM_LAST)
        return Q::str(p);
    return (char const*) p;
}

auto Value::asObj () const -> Object& {
    switch (tag()) {
        case Value::Nil: return (Object&) None::nullObj; // drop const
        case Value::Int: return *new struct Int (*this);
        case Value::Str: return *new struct Str (*this);
        case Value::Obj: break;
    }
    return obj();
}

auto Value::asInt () const -> int64_t {
    if (isInt())
        return (int) *this;
    return asType<struct Int>();
}

bool Value::truthy () const {
    switch (tag()) {
        case Value::Nil: break;
        case Value::Int: return (int) *this != 0;
        case Value::Str: return *(char const*) *this != 0;
        case Value::Obj: return obj().unop(UnOp::Boln).isTrue();
    }
    return false;
}

auto Value::operator== (Value rhs) const -> bool {
    if (v == rhs.v)
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
                case UnOp::Int:  // fall through
                case UnOp::Pos:  // fall through
                case UnOp::Hash: return *this;
                case UnOp::Abs:  if (n > 0) return *this;
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

    return asObj().unop(op);
}

auto Value::binOp (BinOp op, Value rhs) const -> Value {
    // TODO the inverted optimisations will fail if a ResumableObj is involved
    switch (op) {
        case BinOp::In:        return rhs.binOp(BinOp::Contains, *this);
        case BinOp::More:      return rhs.binOp(BinOp::Less, *this);
        case BinOp::LessEqual: return rhs.binOp(BinOp::Less, *this).invert();
        case BinOp::MoreEqual: return binOp(BinOp::Less, rhs).invert();
        case BinOp::NotEqual:  return binOp(BinOp::Equal, rhs).invert();
        default:               break;
    }

    if (tag() == rhs.tag())
        switch (tag()) {
            case Int: {
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
                break;
            }
            case Str: {
                break;
            }
            default:
                assert(isObj());
                switch (op) {
                    case BinOp::Equal:
                        if (ifType<None>() != 0) // FIXME special-cased!
                            return asBool(id() == rhs.id());
                        // fall through
                    default:
                        break;
                }
                break;
        }

    return asObj().binop(op, rhs);
}

auto Value::check (Type const& t) const -> bool {
    return isObj() && &obj().type() == &t;
}

void Value::verify (Type const& t) const {
    auto f = check(t);
    if (!f) {
        dump("verify?");
        Value v = t;
        v.dump(t.name);
    }
    assert(f);
}

void VaryVec::atAdj (uint32_t idx, uint32_t len) {
    assert(idx < fill);
    auto olen = atLen(idx);
    if (len == olen)
        return;
    auto ofill = fill;
    fill = pos(fill);
    if (len > olen)
        ByteVec::insert(pos(idx+1), len - olen);
    else
        ByteVec::remove(pos(idx) + len, olen - len);
    fill = ofill;

    for (uint32_t i = idx + 1; i <= fill; ++i)
        pos(i) += len - olen;
}

void VaryVec::atSet (uint32_t idx, void const* ptr, uint32_t len) {
    atAdj(idx, len);
    memcpy(begin() + pos(idx), ptr, len);
}

void VaryVec::insert (uint32_t idx, uint32_t num) {
    assert(idx <= fill);
    if (cap() == 0) {
        ByteVec::insert(0, 2);
        pos(0) = 2;
        fill = 0;
    }

    auto ofill = fill;
    fill = pos(fill);
    ByteVec::insert(2 * idx, 2 * num);
    fill = ofill + num;

    for (uint32_t i = 0; i <= fill; ++i)
        pos(i) += 2 * num;
    for (uint32_t i = 0; i < num; ++i)
        pos(idx+i) = pos(idx+num);
}

void VaryVec::remove (uint32_t idx, uint32_t num) {
    assert(idx + num <= fill);
    auto diff = pos(idx+num) - pos(idx);

    auto ofill = fill;
    fill = pos(fill);
    ByteVec::remove(pos(idx), diff);
    ByteVec::remove(2 * idx, 2 * num);
    fill = ofill - num;

    for (uint32_t i = 0; i <= fill; ++i)
        pos(i) -= 2 * num;
    for (uint32_t i = idx; i <= fill; ++i)
        pos(i) -= diff;
}

auto Object::call (ArgVec const&) const -> Value {
    Value v = this; v.dump("call?"); assert(false);
    return {};
}

auto Object::unop (UnOp) const -> Value {
    Value v = this; v.dump("unop?"); assert(false);
    return {};
}

auto Object::binop (BinOp, Value) const -> Value {
    Value v = this; v.dump("binop?"); assert(false);
    return {};
}

auto Object::attr (char const* name, Value& self) const -> Value {
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
    auto r = ks->asRange(len());
    if (r.by != 1)
        return {E::NotImplementedError, "assign to extended slice", k};
    return store(r, v.asObj());
}

auto Bool::unop (UnOp op) const -> Value {
    switch (op) {
        case UnOp::Not:  return Value::asBool(this != &trueObj);
        case UnOp::Int:  // fall through
        case UnOp::Hash: return this == &trueObj;
        case UnOp::Boln: return *this;
        default:         break;
    }
    return Object::unop(op);
}

auto Bool::create (ArgVec const& args, Type const*) -> Value {
    if (args.num == 1)
        return args[0].unOp(UnOp::Boln);
    assert(args.num == 0);
    return False;
}

auto Int::make (int64_t i) -> Value {
    Value v = (int) i;
    if ((int) v != i)
        return new Int (i);
    return v;
}

auto Int::conv (char const* s) -> Value {
    return make(strtoll(s, nullptr, 10));
}

auto Int::unop (UnOp op) const -> Value {
    // TODO use templates to share code with Value::unOp ?
    switch (op) {
        case UnOp::Int:  // fall through
        case UnOp::Pos:  return *this;
        case UnOp::Hash: return Q::hash(&i64, sizeof i64);
        case UnOp::Abs:  if (i64 > 0) return *this; // else fall through
        case UnOp::Neg:  return make(-i64);
        case UnOp::Inv:  return make(~i64);
        case UnOp::Not:  return Value::asBool(i64 == 0);
        case UnOp::Boln: return Value::asBool(i64 != 0);
    }
    assert(false);
    return {};
}

auto Int::binop (BinOp op, Value rhs) const -> Value {
    // TODO use templates to share code with Value::binOp ?
    auto r64 = rhs.asInt();
    switch (op) {
        case BinOp::Less:
            return Value::asBool(i64 < r64);
        case BinOp::Equal:
            return Value::asBool(i64 == r64);
        case BinOp::Or: case BinOp::InplaceOr:
            return make(i64 | r64);
        case BinOp::Xor: case BinOp::InplaceXor:
            return make(i64 ^ r64);
        case BinOp::And: case BinOp::InplaceAnd:
            return make(i64 & r64);
        case BinOp::Lshift: case BinOp::InplaceLshift:
            return make(i64 << r64);
        case BinOp::Rshift: case BinOp::InplaceRshift:
            return make(i64 >> r64);
        case BinOp::Add: case BinOp::InplaceAdd:
            return make(i64 + r64);
        case BinOp::Subtract: case BinOp::InplaceSubtract:
            return make(i64 - r64);
        case BinOp::Multiply: case BinOp::InplaceMultiply:
            return make(i64 * r64);
        case BinOp::TrueDivide: case BinOp::InplaceTrueDivide:
            // TODO needs floats, fall through
        case BinOp::InplaceFloorDivide: case BinOp::FloorDivide:
            if (r64 == 0)
                return E::ZeroDivisionError;
            return i64 / r64;
        case BinOp::InplaceModulo: case BinOp::Modulo:
            if (r64 == 0)
                return E::ZeroDivisionError;
            return i64 % r64;
        default:
            break;
    }
    (void) rhs; assert(false);
    return {}; // TODO
}

auto Int::create (ArgVec const& args, Type const*) -> Value {
    assert(args.num == 1);
    auto v = args[0];
    switch (v.tag()) {
        case Value::Nil: // fall through
        case Value::Int: return v;
        case Value::Str: return Int::conv(v);
        case Value::Obj: return v.unOp(UnOp::Int);
    }
    return {};
}

auto Iterator::next() -> Value {
    if (ipos < 0)
        return iobj.next();
    if (ipos >= (int) iobj.len())
        return E::StopIteration;
    // TODO duplicate code, see opForIter in pyvm.h
    if (&iobj.type() == &Dict::info || &iobj.type() == &Set::info)
        return ((List&) iobj)[ipos++]; // avoid keyed access
    return iobj.getAt(ipos++);
}

Bytes::Bytes (void const* ptr, uint32_t len) {
    insert(0, len);
    memcpy(begin(), ptr, len);
}

auto Bytes::unop (UnOp op) const -> Value {
    switch (op) {
        case UnOp::Boln: return Value::asBool(size());
        case UnOp::Hash: return Q::hash(begin(), size());
        default:         break;
    }
    return Object::unop(op);
}

auto Bytes::binop (BinOp op, Value rhs) const -> Value {
    auto& val = rhs.asType<Bytes>();
    assert(size() == val.size());
    switch (op) {
        case BinOp::Equal:
            return Value::asBool(size() == val.size() &&
                                    memcmp(begin(), val.begin(), size()) == 0);
        default:
            break;
    }
    return Object::binop(op, rhs);
}

auto Bytes::getAt (Value k) const -> Value {
    if (!k.isInt())
        return sliceGetter(k);
    return (*this)[k];
}

auto Bytes::copy (Range const& r) const -> Value {
    auto n = r.len();
    auto v = new Bytes;
    v->insert(0, n);
    for (uint32_t i = 0; i < n; ++i)
        (*v)[i] = (*this)[r.getAt(i)];
    return v;
}

auto Bytes::create (ArgVec const& args, Type const*) -> Value {
    assert(args.num == 1);
    Value v = args[0];
    if (v.isInt()) {
        auto o = new Bytes ();
        o->insert(0, v);
        return o;
    }
    const void* p = 0;
    uint32_t n = 0;
    if (v.isStr()) {
        p = (char const*) v;
        n = strlen((char const*) p);
    } else {
        auto ps = v.ifType<Str>();
        auto pb = v.ifType<Bytes>();
        if (ps != 0) {
            p = (char const*) *ps;
            n = strlen((char const*) p); // TODO
        } else if (pb != 0) {
            p = pb->begin();
            n = pb->size();
        } else
            assert(false); // TODO iterables
    }
    return new Bytes (p, n);
}

Str::Str (char const* s, int n) {
    assert(n >= 0 || s != nullptr);
    if (n < 0)
        n = strlen(s);
    insert(0, n);
    adj(n+1);
    assert((int) cap() > n);
    if (s != nullptr)
        memcpy(begin(), s, n);
    else
        memset(begin(), 0, n);
    begin()[n] = 0;
}

auto Str::unop (UnOp op) const -> Value {
    switch (op) {
        case UnOp::Int:  return Int::conv((char const*) begin());
        default:         break;
    }
    return Bytes::unop(op);
}

auto Str::binop (BinOp op, Value rhs) const -> Value {
    auto l = (char const*) begin();
    char const* r = nullptr;
    if (rhs.isStr())
        r = rhs;
    else {
        auto o = rhs.ifType<Str>();
        if (o != nullptr)
            r = *o;
    }
    switch (op) {
        case BinOp::Equal:
            return Value::asBool(r != 0 && strcmp(l, r) == 0);
        case BinOp::Add: {
            assert(r != nullptr);
            auto nl = strlen(l), nr = strlen(r);
            auto o = new struct Str (nullptr, nl + nr);
            memcpy((char*) o->begin(), l, nl);
            memcpy((char*) o->begin() + nl, r, nr);
            return o;
        }
        default:
            break;
    }
    return Bytes::binop(op, rhs);
}

auto Str::getAt (Value k) const -> Value {
    if (!k.isInt())
        return sliceGetter(k);
    int idx = k;
    if (idx < 0)
        idx += size();
    return new Str ((char const*) begin() + idx, 1); // TODO utf-8
}

auto Str::create (ArgVec const& args, Type const*) -> Value {
    assert(args.num == 1 && args[0].isStr());
    return new Str (args[0]);
}

auto Range::len () const -> uint32_t {
    assert(by != 0);
    auto n = (to - from + by + (by > 0 ? -1 : 1)) / by;
    return n < 0 ? 0 : n;
}

auto Range::getAt (Value k) const -> Value {
    return from + k * by;
}

auto Range::create (ArgVec const& args, Type const*) -> Value {
    assert(1 <= args.num && args.num <= 3);
    int a = args.num > 1 ? (int) args[0] : 0;
    int b = args.num == 1 ? args[0] : args[1];
    int c = args.num > 2 ? (int) args[2] : 1;
    return new Range (a, b, c);
}

auto Slice::asRange (int sz) const -> Range {
    int from = off.isInt() ? (int) off : 0;
    int to = num.isInt() ? (int) num : sz;
    int by = step.isInt() ? (int) step : 1;
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
    assert(1 <= args.num && args.num <= 3);
    Value a = args.num > 1 ? args[0] : Null;
    Value b = args.num == 1 ? args[0] : args[1];
    Value c = args.num > 2 ? args[2] : Null;
    return new Slice (a, b, c);
}

auto Lookup::operator[] (char const* key) const -> Value {
    for (uint32_t i = 0; i < count; ++i)
        if (strcmp(key, items[i].k) == 0)
            return items[i].v;
    return {};
}

auto Lookup::getAt (Value k) const -> Value {
    assert(k.isStr());
    return (*this)[k];
}

void Lookup::marker () const {
    for (uint32_t i = 0; i < count; ++i)
        items[i].v.marker();
}

Tuple::Tuple (ArgVec const& args) : fill (args.num) {
    memcpy((Value*) data(), args.begin(), args.num * sizeof (Value));
}

auto Tuple::getAt (Value k) const -> Value {
    if (!k.isInt())
        return sliceGetter(k);
    return data()[k];
}

auto Tuple::copy (Range const& r) const -> Value {
    int n = r.len();
    Vector avec; // TODO messy way to create tuple via sized vec with no data
    avec.insert(0, n);
    auto v = Tuple::create({avec, n, 0});
    auto p = (Value*) (&v.asType<Tuple>() + 1);
    for (int i = 0; i < n; ++i)
        p[i] = getAt(r.getAt(i));
    return v;
}

void Tuple::marker () const {
    for (uint32_t i = 0; i < fill; ++i)
        data()[i].marker();
}

auto Tuple::create (ArgVec const& args, Type const*) -> Value {
    if (args.num == 0)
        return Empty; // there's one unique empty tuple
    return new (args.num * sizeof (Value)) Tuple (args);
}