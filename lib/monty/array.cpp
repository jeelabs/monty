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

struct ArrayAny {
    virtual auto get (VecOf<uint8_t> const&, size_t) const -> Value  = 0;
    virtual void set (VecOf<uint8_t>&, size_t, Value) const = 0;
    virtual void ins (VecOf<uint8_t>&, size_t, size_t) const = 0;
    virtual void del (VecOf<uint8_t>&, size_t, size_t) const = 0;
};

template< typename T >
struct ArrayAs : ArrayAny {
    auto get (VecOf<uint8_t> const& vec, size_t pos) const -> Value override {
        auto& v = (VecOf<T>&) vec;
        return v[pos];
    }
    void set (VecOf<uint8_t>& vec, size_t pos, Value val) const override {
        auto& v = (VecOf<T>&) vec;
        v[pos] = val;
    }
    void ins (VecOf<uint8_t>& vec, size_t pos, size_t num) const override {
        auto& v = (VecOf<T>&) vec;
        v.insert(pos, num);
    }
    void del (VecOf<uint8_t>& vec, size_t pos, size_t num) const override {
        auto& v = (VecOf<T>&) vec;
        v.remove(pos, num);
    }
};

constexpr auto arrayChars = "PTNbBhHiIlL";

static ArrayAs<int8_t>   const vec_b;
static ArrayAs<uint8_t>  const vec_B;
static ArrayAs<int16_t>  const vec_h;
static ArrayAs<uint16_t> const vec_H;
static ArrayAs<int32_t>  const vec_l;
static ArrayAs<uint32_t> const vec_L;

static ArrayAny const* arrayTypes [] = {
    &vec_B, // TODO P
    &vec_B, // TODO T
    &vec_B, // TODO N
    &vec_b,
    &vec_B,
    &vec_h,
    &vec_H,
    &vec_h,
    &vec_H,
    &vec_l,
    &vec_L,
};

Array::Array (char type, size_t len) {
    auto p = strchr(arrayChars, type);
    assert(p != nullptr);
    auto s = p - arrayChars;
    arrayTypes[s]->ins(*this, 0, len);
    fill |= s << 28;
}

auto Array::mode () const -> char {
    return arrayChars[fill >> 28];
}

auto Array::len () const -> size_t {
    return size() & 0x0FFFFFFF;
}

auto Array::getAt (Value k) const -> Value {
    assert(k.isInt());
    auto n = k; // TODO relPos(k);
    return arrayTypes[sel()]->get(*this, n);
}

auto Array::setAt (Value k, Value v) -> Value {
    assert(k.isInt());
    auto n = k; // TODO relPos(k);
    arrayTypes[sel()]->set(*this, n, v);
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
