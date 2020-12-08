// type.cpp - basic object types and type system

#include "monty.h"
//#include "ops.h"
#include <cassert>

using namespace monty;
//using monty::Q;

None const None::nullObj;
Bool const Bool::falseObj;
Bool const Bool::trueObj;
//Tuple const Tuple::emptyObj;

Value const monty::Null  {None::nullObj};
Value const monty::False {Bool::falseObj};
Value const monty::True  {Bool::trueObj};
//Value const monty::Empty {Tuple::emptyObj};

#if 0
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
#endif

Value::Value (char const* arg) : v ((uintptr_t) arg * 4 + 2) {
    if (Vec::inPool(arg)) // don't store pointers into movable vector space
#if 0
        *this = new struct Str (arg); // TODO should try Q::find first
#else
        ;//XXX v = Q::make(arg) * 4 + 2;
#endif
    else
        assert((char const*) *this == arg); // watch out for address truncation
}

#if 0
Value::Value (E exc, Value arg1, Value arg2) {
    Vector v;
    v.insert(0, 2);
    v[0] = arg1;
    v[1] = arg2;
    auto nargs = arg1.isNil() ? 0 : arg2.isNil() ? 1 : 2;
    *this = Exception::create(exc, {v, nargs, 0});
    Interp::exception(*this);
}
#endif

#if 0
Value::operator char const* () const {
    if (!isStr())
        return asType<struct Str>();
    auto p = v >> 2;
    if (p < QID_RAM_LAST)
        return Q::str(p);
    return (char const*) p;
}
#endif

auto Value::asObj () const -> Object& {
    switch (tag()) {
        case Value::Nil: return (Object&) None::nullObj; // drop const
        case Value::Int: break; //XXX return *new struct Int (*this);
        case Value::Str: break; //XXX return *new struct Str (*this);
        case Value::Obj: break;
    }
    return obj();
}

auto Value::asInt () const -> int64_t {
    if (isInt())
        return (int) *this;
    return 0; //XXX return asType<struct Int>();
}

bool Value::truthy () const {
    switch (tag()) {
        case Value::Nil: break;
        case Value::Int: return (int) *this != 0;
        case Value::Str: return *(char const*) *this != 0;
        case Value::Obj: break; //XXX return obj().unop(UnOp::Boln).isTrue();
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
            case Obj: break; //XXX return obj().binop(BinOp::Equal, rhs).truthy();
        }
    return false;
}

#if 0
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
#endif

auto Value::check (Type const& t) const -> bool {
    return isObj() && &obj().type() == &t;
}

void Value::verify (Type const& t) const {
    auto f = check(t);
    if (!f) {
        dump("verify?");
        //XXX Value v = t;
        //XXX v.dump(t.name);
    }
    assert(f);
}

#if 0
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
#endif

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
    (void) name; return {};//XXX return type().getAt(name);
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

List::List (ArgVec const& args) {
    insert(0, args.num);
    for (int i = 0; i < args.num; ++i)
        (*this)[i] = args[i];
}

auto List::pop (int idx) -> Value {
    auto n = relPos(idx);
    assert(size() > n);
    Value v = (*this)[n];
    remove(n);
    return v;
}

void List::append (Value v) {
    auto n = size();
    insert(n);
    (*this)[n] = v;
}

auto List::getAt (Value k) const -> Value {
    if (!k.isInt())
        return sliceGetter(k);
    auto n = relPos(k);
    assert(n < size());
    return (*this)[n];
}

auto List::setAt (Value k, Value v) -> Value {
    if (!k.isInt())
        return sliceSetter(k, v);
    auto n = relPos(k);
    assert(n < size());
    return (*this)[n] = v;
}

#if 0
auto List::copy (Range const& r) const -> Value {
    auto n = r.len();
    auto v = new List;
    v->insert(0, n);
    for (uint32_t i = 0; i < n; ++i)
        (*v)[i] = (*this)[r.getAt(i)];
    return v;
}

auto List::store (Range const& r, Object const& v) -> Value {
    assert(r.by == 1);
    int olen = r.len();
    int nlen = v.len();
    if (nlen < olen)
        remove(r.from + nlen, olen - nlen);
    else if (nlen > olen)
        insert(r.from + olen, nlen - olen);
    for (int i = 0; i < nlen; ++i)
        (*this)[r.getAt(i)] = v.getAt(i);
    return {};
}
#endif

auto List::create (ArgVec const& args, Type const*) -> Value {
    return new List (args);
}
