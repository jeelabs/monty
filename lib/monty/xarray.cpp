// array.cpp - arrays, dicts, and other derived types

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "xmonty.h"

using namespace Monty;

Tuple::Tuple (size_t n, Value const* vals) : num (n) {
    memcpy((Value*) data(), vals, n * sizeof *vals);
}

List::List (size_t n, Value const* vals) {
    items.asVec().adj(n);
}

Set::Set (size_t n, Value const* vals) {
    items.asVec().adj(n);
}

auto Set::find (Value v) const -> size_t {
    for (auto& e : items)
        if (v == e)
            return &e - &items[0];
    return len();
}

Set::Proxy::operator bool () const {
    return s.find(v) < s.len();
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

Dict::Dict (size_t n) {
    items.asVec().adj(2*n);
}

Dict::Proxy::operator Value () const {
    auto n = d.len();
    auto pos = d.find(k);
    return pos < n ? d.items[n+pos] : Value {};
}

// dict invariant: items layout is: N keys, then N values, with N == d.len()
auto Dict::Proxy::operator= (Value v) -> Value {
    Value w;
    auto n = d.len();
    auto pos = d.find(k);
    if (v.isNil()) {
        if (pos < n) {
            d.items.len = 2*n;      // don't wipe existing vals
            d.items.remove(n+pos);  // remove value
            d.items.remove(pos);    // remove key
            d.items.len = --n;      // set length to new key count
        }
    } else {
        if (pos == n) { // move all values up and create new gaps
            d.items.len = 2*n;      // don't wipe existing vals
            d.items.insert(2*n);    // create slot for new value
            d.items.insert(n);      // same for key, moves all vals one up
            d.items.len = ++n;      // set length to new key count
            d.items[pos] = k;       // store the key
        } else
            w = d.items[n+pos];
        assert(d.items.asVec().cap() >= 2*n);
        d.items[n+pos] = v;
    }
    return w;
}

auto Type::noFactory (CofV const&, const Type*) -> Value {
    assert(false);
    return {};
}
