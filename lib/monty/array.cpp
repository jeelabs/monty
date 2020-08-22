// array.cpp - arrays, dicts, and other derived types

#include "monty.h"
#include <cassert>

using namespace Monty;

void Monty::mark (Vector const& vec) {
    for (auto e : vec)
        e.marker();
}

struct Accessor {
    virtual auto get (ByteVec&, size_t) const -> Value  = 0;
    virtual void set (ByteVec&, size_t, Value) const = 0;
    virtual void ins (ByteVec&, size_t, size_t) const = 0;
    virtual void del (ByteVec&, size_t, size_t) const = 0;
};

template< typename T >
static auto asVecOf (ByteVec& a) -> VecOf<T>& {
    return (VecOf<T>&) a;
}

template< typename T >
struct AccessAs : Accessor {
    auto get (ByteVec& vec, size_t pos) const -> Value override {
        return asVecOf<T>(vec)[pos];
    }
    void set (ByteVec& vec, size_t pos, Value val) const override {
        asVecOf<T>(vec)[pos] = val;
    }
    void ins (ByteVec& vec, size_t pos, size_t num) const override {
        asVecOf<T>(vec).insert(pos, num);
    }
    void del (ByteVec& vec, size_t pos, size_t num) const override {
        asVecOf<T>(vec).remove(pos, num);
    }
};

constexpr auto arrayModes = "oPTNbBhHiIlLqQ"
#if USE_FLOAT
                            "f"
#endif
#if USE_DOUBLE
                            "d"
#endif
;

// permanent per-type accessors, these are re-used for every Array instance
// the cost per get/set/ins/del is one table index step, just as with vtables

static AccessAs<Value>    const accessor_o;
static AccessAs<int8_t>   const accessor_b;
static AccessAs<uint8_t>  const accessor_B;
static AccessAs<int16_t>  const accessor_h;
static AccessAs<uint16_t> const accessor_H;
static AccessAs<int32_t>  const accessor_l;
static AccessAs<uint32_t> const accessor_L;
static AccessAs<int64_t>  const accessor_q;
static AccessAs<uint64_t> const accessor_Q;
#if USE_FLOAT
static AccessAs<float>    const accessor_f;
#endif
#if USE_DOUBLE
static AccessAs<double>   const accessor_d;
#endif

// must be in same order as arrayModes
static Accessor const* accessors [] = {
    &accessor_o,
    &accessor_B, // TODO P
    &accessor_B, // TODO T
    &accessor_B, // TODO N
    &accessor_b,
    &accessor_B,
    &accessor_h,
    &accessor_H,
    &accessor_h, // i, same as h
    &accessor_H, // I, same as H
    &accessor_l,
    &accessor_L,
    &accessor_q,
    &accessor_Q,
#if USE_FLOAT
    &accessor_f,
#endif
#if USE_DOUBLE
    &accessor_d,
#endif
};

Array::Array (char type, size_t len) {
    auto p = strchr(arrayModes, type);
    auto s = p != nullptr ? p - arrayModes : 0; // use Value if unknown type
    accessors[s]->ins(*this, 0, len);
    fill |= s << 28;
}

auto Array::mode () const -> char {
    return arrayModes[fill >> 28];
}

auto Array::len () const -> size_t {
    return size() & 0x0FFFFFFF;
}

auto Array::getAt (Value k) const -> Value {
    assert(k.isInt());
    auto n = k; // TODO relPos(k);
    return accessors[sel()]->get(const_cast<Array&>(*this), n);
}

auto Array::setAt (Value k, Value v) -> Value {
    assert(k.isInt());
    auto n = k; // TODO relPos(k);
    accessors[sel()]->set(*this, n, v);
    return {};
}

List::List (ArgVec const& args) {
    insert(0, args.num);
    for (int i = 0; i < args.num; ++i)
        (*this)[i] = args[i];
}

auto List::pop (int idx) -> Value {
    assert(len() > 0);
    if (idx < 0)
        idx += size();
    Value v = (*this)[idx];
    remove(idx);
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
    return n < fill ? (*this)[n] : Value {};
}

auto List::setAt (Value k, Value v) -> Value {
    assert(k.isInt());
    return (*this)[relPos(k)] = v;
}

auto Set::find (Value v) const -> size_t {
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
    for (size_t i = 0; i < 2 * fill; ++i) // note: twice the fill
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

    at("__name__") = args[1];
    at("__bases__") = Tuple::create({args.vec, args.num - 2, args.off + 2});

    args[0].obj().call({args.vec, args.num - 2, args.off + 2});

    auto ctx = Interp::context;
    assert(ctx != nullptr);
    ctx->frame().locals = this;
}

Inst::Inst (ArgVec const& args, Class const& cls) : Dict (&cls) {
    auto ctx = Interp::context;
    assert(ctx != nullptr);

    Value self;
    Value init = attr("__init__", self);
    if (!init.isNil()) {
        // stuff "self" before the args passed in TODO is this always ok ???
        assert(ctx == &args.vec && args.off > 0);
        args[-1] = this;
        init.obj().call({args.vec, args.num + 1, args.off - 1});
    }

    ctx->frame().result = this;
}
