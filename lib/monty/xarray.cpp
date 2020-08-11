// array.cpp - arrays, dicts, and other derived types

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "xmonty.h"

using namespace Monty;

auto Array::getAt (Value k) const -> Value {
    assert(k.isInt());
    return (*this)[k];
}

auto Array::setAt (Value k, Value v) -> Value {
    assert(k.isInt());
    (*this)[k] = v;
    return {};
}

Tuple::Tuple (size_t n, Value const* vals) : num (n) {
    memcpy((Value*) data(), vals, n * sizeof *vals);
}

auto Tuple::getAt (Value k) const -> Value {
    assert(k.isInt());
    return data()[k];
}

List::List (size_t n, Value const* vals) {
    items.adj(n);
}

auto List::getAt (Value k) const -> Value {
    assert(k.isInt());
    return (*this)[k];
}

auto List::setAt (Value k, Value v) -> Value {
    assert(k.isInt());
    return (*this)[k] = v;
}

auto Set::find (Value v) const -> size_t {
    for (auto& e : items)
        if (v == e)
            return &e - &items[0];
    return len();
}

auto Set::Proxy::operator= (bool f) -> bool {
    auto n = s.len();
    auto pos = s.find(v);
    if (pos < n && !f)
        s.items.remove(pos);
    else if (pos == n && f) {
        s.items.insert(pos);
        s.items[pos] = v;
    }
    return pos < n;
}

auto Set::has (Value v) const -> bool {
    return find(v) < len();
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

Dict::Dict (size_t n) {
    items.adj(2*n);
}

// dict invariant: items layout is: N keys, then N values, with N == d.len()
auto Dict::Proxy::operator= (Value v) -> Value {
    Value w;
    auto n = d.len();
    auto pos = d.find(k);
    if (v.isNil()) {
        if (pos < n) {
            d.items.fill = 2*n;     // don't wipe existing vals
            d.items.remove(n+pos);  // remove value
            d.items.remove(pos);    // remove key
            d.items.fill = --n;     // set length to new key count
        }
    } else {
        if (pos == n) { // move all values up and create new gaps
            d.items.fill = 2*n;     // don't wipe existing vals
            d.items.insert(2*n);    // create slot for new value
            d.items.insert(n);      // same for key, moves all vals one up
            d.items.fill = ++n;     // set length to new key count
            d.items[pos] = k;       // store the key
        } else
            w = d.items[n+pos];
        assert(d.items.cap() >= 2*n);
        d.items[n+pos] = v;
    }
    return w;
}

auto Dict::at (Value k) const -> Value {
    auto n = len();
    auto pos = find(k);
    return pos < n ? items[n+pos] : Value {};
}

auto Type::noFactory (CofV const&, const Type*) -> Value {
    assert(false);
    return {};
}
