// array.cpp - arrays, dicts, and other derived types

#include "monty.h"
#include <cassert>

using namespace Monty;

void Monty::mark (Vector const& vec) {
    for (auto e : vec)
        if (e.isObj())
            mark(e.obj());
}

Tuple::Tuple (size_t n, Value const* vals) : fill (n) {
    memcpy((Value*) data(), vals, n * sizeof *vals);
}

auto Tuple::getAt (Value k) const -> Value {
    assert(k.isInt());
    return data()[k];
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
    auto get (ByteVec& ary, size_t pos) const -> Value override {
        return asVecOf<T>(ary)[pos];
    }
    void set (ByteVec& ary, size_t pos, Value val) const override {
        asVecOf<T>(ary)[pos] = val;
    }
    void ins (ByteVec& ary, size_t pos, size_t num) const override {
        asVecOf<T>(ary).insert(pos, num);
    }
    void del (ByteVec& ary, size_t pos, size_t num) const override {
        asVecOf<T>(ary).remove(pos, num);
    }
};

constexpr auto arrayModes = "PTNbBhHiIlLqQ";

static AccessAs<int8_t>   const as_b;
static AccessAs<uint8_t>  const as_B;
static AccessAs<int16_t>  const as_h;
static AccessAs<uint16_t> const as_H;
static AccessAs<int32_t>  const as_l;
static AccessAs<uint32_t> const as_L;
static AccessAs<int64_t>  const as_q;
static AccessAs<uint64_t> const as_Q;

static Accessor const* accessors [] = {
    &as_B, // TODO P
    &as_B, // TODO T
    &as_B, // TODO N
    &as_b,
    &as_B,
    &as_h,
    &as_H,
    &as_h,
    &as_H,
    &as_l,
    &as_L,
    &as_q,
    &as_Q,
};

Array::Array (char type, size_t len) {
    auto p = strchr(arrayModes, type);
    assert(p != nullptr);
    auto s = p - arrayModes;
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

List::List (Vector const& vec, int argc, int args) {
    insert(0, argc);
    for (int i = 0; i < argc; ++i)
        (*this)[i] = vec[args+i];
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

auto Type::call (Vector const& vec, int argc, int args) const -> Value {
    return factory(vec, argc, args, this);
}

auto Type::noFactory (Vector const&, int, int, const Type*) -> Value {
    assert(false);
    return {};
}

Class::Class (Vector const& vec, int argc, int args)
        : Type (vec[args+1], Inst::create) {
    assert(argc >= 2);
    at("__name__") = vec[args+1];

    auto& init = vec[args];
    init.obj().call(vec, argc - 2, args + 2);

    auto ctx = Interp::context;
    assert(ctx != nullptr);
    ctx->frame().locals = this;
    ctx->frame().result = this;
}

Inst::Inst (Vector const& vec, int argc, int args, Class const& cls)
        : Dict (&cls) {
    auto ctx = Interp::context;
    assert(ctx != nullptr);

    Value self;
    Value init = attr("__init__", self);
    if (!init.isNil()) {
        // stuff "self" before the args passed in TODO is this always ok ???
        (*ctx)[args-1] = this;
        init.obj().call(vec, argc + 1, args - 1);
    }

    ctx->frame().result = this;
}
