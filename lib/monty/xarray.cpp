// array.cpp - arrays, dicts, and other derived types

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "xmonty.h"

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

List::List (Context& ctx, int argc, int args) {
    insert(0, argc);
    for (int i = 0; i < argc; ++i)
        (*this)[i] = ctx[args+i];
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

auto Set::has (Value v) const -> bool {
    return find(v) < size();
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
    adj(2*n);
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

auto Type::call (Context& ctx, int argc, int args) const -> Value {
    return factory(ctx, argc, args, this);
}

auto Type::noFactory (Context&, int, int, const Type*) -> Value {
    assert(false);
    return {};
}

Class::Class (Context& ctx, int argc, int args)
        : Type (ctx[args+1], Inst::create) {
    assert(argc >= 2);
    at("__name__") = ctx[args+1];
    auto& init = ctx[args].asType<Callable>();
    init.call(ctx, argc - 2, args + 2); // TODO *in-arg* ctx vs *curr* ctx!
    ctx.locals = this;
    ctx.result = this;
}

Inst::Inst (Context& ctx, int argc, int args, Class const& cls) {
    chain = &cls;
    Value self;
    Value init = attr("__init__", self);
    if (!init.isNil()) {
        ctx[args-1] = this; // TODO is this alwats ok ???
        auto& co = init.asType<Callable>();
        co.call(ctx, argc + 1, args - 1);
    }
    ctx.result = this;
}
