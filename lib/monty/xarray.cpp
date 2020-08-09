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

auto Set::find (Value v) const -> int {
    for (auto& e : items)
        if (v == e)
            return &e - &items[0];
    return -1;
}

Set::Proxy::operator bool () const {
    return s.find(v) >= 0;
}

auto Set::Proxy::operator= (Value v) -> bool {
    if (s.find(v) >= 0)
        return true;
    auto i = s.items.length();
    s.items.insert(i);
    s.items[i] = v;
    return false;
}

Dict::Dict (size_t n) {
    // TODO
}

Dict::Proxy::operator Value () const {
    auto i = d.find(v);
    return i >= 0 ? d.items[d.len()+i] : Value {};
}

auto Dict::Proxy::operator= (Value v) -> Value {
    auto i = d.find(v);
    if (i < 0) {
        i = d.items.length();
        d.items.insert(i);
        d.items[i] = v;
        assert(false); // TODO resize, moving vals up
    }
    d.items[d.len()+i] = v;
    return v;
}

auto Type::noFactory (const Type&, ChunkOf<Value> const&) -> Value {
    assert(false);
    return {};
}
