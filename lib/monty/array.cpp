// array.cpp - arrays, dicts, and other derived types

#include "monty.h"
#include "ops.h"
#include <cassert>

using namespace Monty;

void Monty::markVec (Vector const& vec) {
    for (auto e : vec)
        e.marker();
}

struct Accessor {
    virtual auto get (ByteVec&, uint32_t) const -> Value  = 0;
    virtual void set (ByteVec&, uint32_t, Value) const = 0;
    virtual void ins (ByteVec&, uint32_t, uint32_t) const = 0;
    virtual void del (ByteVec&, uint32_t, uint32_t) const = 0;
};

template< typename T >
static auto asVecOf (ByteVec& a) -> VecOf<T>& {
    return (VecOf<T>&) a;
}

template< typename T, int L =0 >
struct AccessAs : Accessor {
    auto get (ByteVec& vec, uint32_t pos) const -> Value override {
        if (L)
            return Int::make(asVecOf<T>(vec)[pos]);
        else
            return asVecOf<T>(vec)[pos];
    }
    void set (ByteVec& vec, uint32_t pos, Value val) const override {
        if (L)
            asVecOf<T>(vec)[pos] = val.asInt();
        else
            asVecOf<T>(vec)[pos] = val;
    }
    void ins (ByteVec& vec, uint32_t pos, uint32_t num) const override {
        asVecOf<T>(vec).insert(pos, num);
    }
    void del (ByteVec& vec, uint32_t pos, uint32_t num) const override {
        asVecOf<T>(vec).remove(pos, num);
    }
};

template< int L >                                   // 0 1 2
struct AccessAsBits : Accessor {                    // P T N
    constexpr static auto bits = 1 << L;            // 1 2 4
    constexpr static auto mask = (1 << bits) - 1;   // 1 3 15
    constexpr static auto shft = 3 - L;             // 3 2 1
    constexpr static auto rest = (1 << shft) - 1;   // 7 3 1

    auto get (ByteVec& vec, uint32_t pos) const -> Value override {
        return (vec[pos>>shft] >> bits * (pos & rest)) & mask;
    }
    void set (ByteVec& vec, uint32_t pos, Value val) const override {
        auto b = bits * (pos & rest);
        auto& e = vec[pos>>shft];
        e = (e & ~(mask << b)) | (((int) val & mask) << b);
    }
    void ins (ByteVec& vec, uint32_t pos, uint32_t num) const override {
        assert((pos & rest) == 0 && (num & rest) == 0);
        vec.fill >>= shft;
        vec.insert(pos >> shft, num >> shft);
        vec.fill <<= shft;
    }
    void del (ByteVec& vec, uint32_t pos, uint32_t num) const override {
        assert((pos & rest) == 0 && (num & rest) == 0);
        vec.fill >>= shft;
        vec.remove(pos >> shft, num >> shft);
        vec.fill <<= shft;
    }
};

struct AccessAsVaryBytes : Accessor {
    auto get (ByteVec& vec, uint32_t pos) const -> Value override {
        auto& v = (VaryVec&) vec;
        return new Bytes (v.atGet(pos), v.atLen(pos));
    }
    void set (ByteVec& vec, uint32_t pos, Value val) const override {
        auto& v = (VaryVec&) vec;
        if (val.isNil())
            v.remove(pos);
        else {
            auto& b = val.asType<Bytes>();
            v.atSet(pos, b.begin(), b.size());
        }
    }
    void ins (ByteVec& vec, uint32_t pos, uint32_t num) const override {
        ((VaryVec&) vec).insert(pos, num);
    }
    void del (ByteVec& vec, uint32_t pos, uint32_t num) const override {
        ((VaryVec&) vec).remove(pos, num);
    }
};

struct AccessAsVaryStr : AccessAsVaryBytes {
    auto get (ByteVec& vec, uint32_t pos) const -> Value override {
        return new Str ((char const*) ((VaryVec&) vec).atGet(pos));
    }
    void set (ByteVec& vec, uint32_t pos, Value val) const override {
        auto& v = (VaryVec&) vec;
        if (val.isNil())
            v.remove(pos);
        else {
            char const* s = val.isStr() ? val : val.asType<Str>();
            v.atSet(pos, (char const*) s, strlen(s) + 1);
        }
    }
};

constexpr auto arrayModes = "PTNbBhHiIlLqvV"
#if USE_FLOAT
                            "f"
#endif
#if USE_DOUBLE
                            "d"
#endif
;

// permanent per-type accessors, these are re-used for every Array instance
// the cost per get/set/ins/del is one table index step, just as with vtables

static AccessAsBits<0>      const accessor_P;
static AccessAsBits<1>      const accessor_T;
static AccessAsBits<2>      const accessor_N;
static AccessAs<int8_t>     const accessor_b;
static AccessAs<uint8_t>    const accessor_B;
static AccessAs<int16_t>    const accessor_h;
static AccessAs<uint16_t>   const accessor_H;
static AccessAs<int32_t,1>  const accessor_l;
static AccessAs<uint32_t,1> const accessor_L;
static AccessAs<int64_t,1>  const accessor_q;
static AccessAsVaryBytes    const accessor_v;
static AccessAsVaryStr      const accessor_V;
#if USE_FLOAT
static AccessAs<float>    const accessor_f;
#endif
#if USE_DOUBLE
static AccessAs<double>   const accessor_d;
#endif

// must be in same order as arrayModes
static Accessor const* accessors [] = {
    &accessor_P,
    &accessor_T,
    &accessor_N,
    &accessor_b,
    &accessor_B,
    &accessor_h,
    &accessor_H,
    &accessor_h, // i, same as h
    &accessor_H, // I, same as H
    &accessor_l,
    &accessor_L,
    &accessor_q,
    &accessor_v,
    &accessor_V,
#if USE_FLOAT
    &accessor_f,
#endif
#if USE_DOUBLE
    &accessor_d,
#endif
};

Array::Array (char type, uint32_t len) {
    auto p = strchr(arrayModes, type);
    auto s = p != nullptr ? p - arrayModes : 0; // use Value if unknown type
    accessors[s]->ins(*this, 0, len);
    fill |= s << 27;
}

auto Array::mode () const -> char {
    return arrayModes[fill >> 27];
}

auto Array::len () const -> uint32_t {
    return size() & 0x07FFFFFF;
}

auto Array::getAt (Value k) const -> Value {
    assert(k.isInt());
    auto n = k; // TODO relPos(k);
    return accessors[sel()]->get(const_cast<Array&>(*this), n);
}

auto Array::setAt (Value k, Value v) -> Value {
    assert(k.isInt());
    auto n = k; // TODO relPos(k);
    auto s = sel();
    fill &= 0x07FFFFFF;
    accessors[s]->set(*this, n, v);
    fill |= s << 27;
    return {};
}

void Array::insert (uint32_t idx, uint32_t num) {
    auto s = sel();
    fill &= 0x07FFFFFF;
    accessors[s]->ins(*this, idx, num);
    fill = fill + (s << 27);
}

void Array::remove (uint32_t idx, uint32_t num) {
    auto s = sel();
    fill &= 0x07FFFFFF;
    accessors[s]->del(*this, idx, num);
    fill = fill + (s << 27);
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
    assert(k.isInt());
    auto n = relPos(k);
    assert(n < size());
    return (*this)[n];
}

auto List::setAt (Value k, Value v) -> Value {
    assert(k.isInt());
    auto n = relPos(k);
    assert(n < size());
    return (*this)[n] = v;
}

auto Set::find (Value v) const -> uint32_t {
    for (auto& e : *this)
        if (v == e)
            return &e - begin();
    return size();
}

auto Set::Proxy::operator= (bool f) -> bool {
    auto n = s.size();
    auto pos = s.find(v);
    if (pos < n && !f)
        s.remove(pos);
    else if (pos == n && f) {
        s.insert(pos);
        s[pos] = v;
    }
    return pos < n;
}

auto Set::binop (BinOp op, Value rhs) const -> Value {
    if (op == BinOp::Contains)
        return Value::asBool(find(rhs) < size());
    return Object::binop(op, rhs);
}

auto Set::getAt (Value k) const -> Value {
    assert(k.isInt());
    auto f = (*this)[k];
    return Value::asBool(f);
}

auto Set::setAt (Value k, Value v) -> Value {
    assert(k.isInt());
    auto f = (*this)[k] = v.truthy();
    return Value::asBool(f);
}

// dict invariant: items layout is: N keys, then N values, with N == d.size()
auto Dict::Proxy::operator= (Value v) -> Value {
    Value w;
    auto n = d.size();
    auto pos = d.find(k);
    if (v.isNil()) {
        if (pos < n) {
            d.fill = 2*n;     // don't wipe existing vals
            d.remove(n+pos);  // remove value
            d.remove(pos);    // remove key
            d.fill = --n;     // set length to new key count
        }
    } else {
        if (pos == n) { // move all values up and create new gaps
            d.fill = 2*n;     // don't wipe existing vals
            d.insert(2*n);    // create slot for new value
            d.insert(n);      // same for key, moves all vals one up
            d.fill = ++n;     // set length to new key count
            d[pos] = k;       // store the key
        } else
            w = d[n+pos];
        assert(d.cap() >= 2*n);
        d[n+pos] = v;
    }
    return w;
}

auto Dict::at (Value k) const -> Value {
    auto n = size();
    auto pos = find(k);
    return pos < n ? (*this)[n+pos] :
            chain != nullptr ? chain->getAt(k) : Value {};
}

void Dict::marker () const {
    auto& v = (Vector const&) *this;
    for (uint32_t i = 0; i < 2 * fill; ++i) // note: twice the fill
        v[i].marker();
    mark(chain);
}

auto Type::call (ArgVec const& args) const -> Value {
    return factory(args, this);
}

auto Type::noFactory (ArgVec const&, const Type*) -> Value {
    assert(false);
    return {};
}

Class::Class (ArgVec const& args) : Type (args[1], Inst::create) {
    assert(2 <= args.num && args.num <= 3); // no support for multiple inheritance
    if (args.num > 2)
        chain = &args[2].asType<Class>();

    at(Q( 23,"__name__")) = args[1];
    at(Q(166,"__bases__")) = Tuple::create({args.vec, args.num-2, args.off+2});

    args[0].obj().call({args.vec, args.num - 2, args.off + 2});

    auto ctx = Interp::context;
    assert(ctx != nullptr);
    ctx->frame().locals = this;
}

Inst::Inst (ArgVec const& args, Class const& cls) : Dict (&cls) {
    auto ctx = Interp::context;
    assert(ctx != nullptr); (void) ctx;

    Value self;
    Value init = attr(Q( 17,"__init__"), self);
    if (!init.isNil()) {
        // stuff "self" before the args passed in TODO is this always ok ???
        assert(ctx == &args.vec && args.off > 0);
        args[-1] = this;
        init.obj().call({args.vec, args.num + 1, args.off - 1});
    }
}
