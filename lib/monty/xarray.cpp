// array.cpp - arrays, dicts, and other derived types

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "xmonty.h"

using namespace Monty;

Tuple::Tuple (size_t n, Value const* vals) {
    // TODO
}

List::List (size_t n, Value const* vals) {
    // TODO
}

Set::Set (size_t n, Value const* vals) {
    // TODO
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
    // TODO
}

Dict::Proxy::operator Value () const {
    auto pos = d.find(k);
    return pos >= 0 ? d.items[d.len()+pos] : Value {};
}

auto Dict::Proxy::operator= (Value v) -> Value {
    Value w;
    auto n = d.len();
    auto pos = d.find(k);
    if (v.isNil()) {
        if (pos < n) {
            d.items.len = 2*n;
            d.items.remove(n+pos);
            d.items.remove(pos);
            d.items.len = --n;
        }
    } else {
        if (pos == n) { // move all values up and create new gaps
            d.items.len = 2*n;      // don't wipe existing vals
            d.items.insert(2*n);    // create slot for new value
            d.items.insert(n);      // same for key, moves all vals one up
            assert(d.len() == 2*(n+1));
            d.items.len = ++n;      // restore length to key count
            d.items[pos] = k;       // store the key
        } else
            w = d.items[n+pos];
        assert(d.items.asVecOf<Value>().cap() >= 2*n);
        d.items[n+pos] = v;
    }
    // invariant: the layout is: N keys, then N values, with N == d.len()
    return w;
}

auto Type::noFactory (const Type&, ChunkOf<Value> const&) -> Value {
    assert(false);
    return {};
}
