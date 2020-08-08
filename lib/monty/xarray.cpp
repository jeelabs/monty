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
    for (auto& e : keys)
        if (v.isEq(e))
            return &e - &keys[0];
    return -1;
}

Set::Proxy::operator bool () const {
    return s.find(v) >= 0;
}

auto Set::Proxy::operator= (Value v) -> bool {
    if (s.find(v) >= 0)
        return true;
    auto i = s.keys.length();
    s.keys.insert(i);
    s[i] = v;
    return false;
}

Dict::Dict (size_t n) {
    // TODO
}

Dict::Proxy::operator Value () const {
    auto i = d.find(v);
    return i >= 0 ? d.vals[i] : Value {};
}

auto Dict::Proxy::operator= (Value v) -> Value {
    auto i = d.find(v);
    if (i < 0) {
        i = d.keys.length();
        d.keys.insert(i);
        d.keys[i] = v;
        d.vals.insert(i);
    }
    d.vals[i] = v;
    return v;
}
