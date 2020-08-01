// type.cpp - basic object types and type system

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "xmonty.h"

using namespace Monty;

const NoneObj NoneObj::noneObj;
const BoolObj BoolObj::falseObj;
const BoolObj BoolObj::trueObj;

const Value Value::None =  NoneObj::noneObj;
const Value Value::False = BoolObj::falseObj;
const Value Value::True  = BoolObj::trueObj;

Value::Value (int arg) : v ((arg << 1) | 1) {
    if ((int) *this != arg)
        *this = new IntObj (arg);
}

Value::Value (char const* arg) : v (((uintptr_t) arg << 2) | 2) {
    assert((char const*) *this == arg);
}

auto Value::check (TypeObj const& t) const -> bool {
    return isObj() && &obj().type() == &t;
}

void Value::verify (TypeObj const& t) const {
    assert(check(t));
}

Vector::Vector (size_t bits) {
    info = 0;
    while (bits > (1U << info))
        ++info;
}

auto Vector::getInt (int idx) const -> int {
    auto p = getPtr(idx);
    switch (info) {
        case 3: return *(int8_t*) p;
        case 4: return *(int16_t*) p;
        case 5: return *(int32_t*) p;
    }
    return 0;
}

auto Vector::getIntU (int idx) const -> uint32_t {
    if (idx < 0)
        idx += fill;
    auto p = (uint8_t*) getPtr(idx);
    switch (info) {
        case 0: return (*p >> (idx&7)) & 0x1;
        case 1: return (*p >> 2*(idx&3)) & 0x3;
        case 2: return (*p >> 4*(idx&1)) & 0xF;
        case 3: return *p;
        case 4: return *(uint16_t*) p;
        case 5: return *(uint32_t*) p;
    }
    return 0;
}

auto Vector::getPtr (int idx) const -> void* {
    if (idx < 0)
        idx += fill;
    return ptr() + widthOf(idx);
}

void Vector::set (int idx, int val) {
    assert(1U << info <= 8 * sizeof val);
    set(idx, &val); // TODO assumes little-endian byte order
}

void Vector::set (int idx, const void* ptr) {
    if (idx < 0)
        idx += fill;
    if (widthOf(idx) >= cap()) {
        auto n = (cap() << 3) >> info;
        ins(n, idx + 1 - n);
    }
    auto p = (uint8_t*) getPtr(idx);
    switch (info) {
        case 0: *p = (*p & ~(0x1 << (idx&7))) |
                        ((*(const uint8_t*) ptr & 0x1) << (idx&7)); return;
        case 1: *p = (*p & ~(0x3 << 2*(idx&3))) |
                        ((*(const uint8_t*) ptr & 0x3) << 2*(idx&3)); return;
        case 2: *p = (*p & ~(0xF << 4*(idx&1))) |
                        ((*(const uint8_t*) ptr & 0xF) << 4*(idx&1)); return;
        case 3: *p = *(const uint8_t*) ptr; return;
        case 4: *(uint16_t*) p = *(const uint16_t*) ptr; return;
        case 5: *(uint32_t*) p = *(const uint32_t*) ptr; return;
    }
    memcpy(getPtr(idx), ptr, widthOf(1));
}

void Vector::ins (int idx, int num) {
    if (num <= 0)
        return;
    auto needed = widthOf(fill + num);
    if (needed > cap()) {
        resize(needed);
        assert(ptr() != 0 && cap() >= needed);
    }
    auto p = getPtr(idx);
    assert(info >= 3); // TODO
    memmove(getPtr(idx + num), p, widthOf(fill - idx));
    memset(p, 0, widthOf(num));
    fill += num;
}

void Vector::del (int idx, int num) {
    if (num <= 0)
        return;
    fill -= num;
    assert(info >= 3); // TODO
    memmove(getPtr(idx), getPtr(idx + num), widthOf(fill - idx));
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// TODO added to satisfy linker

struct Monty::LookupObj {
    // TODO
};

LookupObj const BoolObj::attrs;
LookupObj const IntObj::attrs;
LookupObj const TypeObj::attrs;

auto Object::unop (UnOp) const -> Value {
    assert(false);
    return Value ();
}

auto NoneObj::unop (UnOp) const -> Value {
    return Value (); // TODO
}

auto BoolObj::unop (UnOp) const -> Value {
    return Value (); // TODO
}

auto IntObj::unop (UnOp) const -> Value {
    return Value (); // TODO
}

auto Object::binop (BinOp, Value) const -> Value {
    assert(false);
    return Value ();
}

auto BoolObj::create (const TypeObj&, int argc, Value argv[]) -> Value {
    return Value (); // TODO
}

auto IntObj::create (const TypeObj&, int argc, Value argv[]) -> Value {
    return Value (); // TODO
}

auto TypeObj::create (const TypeObj&, int argc, Value argv[]) -> Value {
    return Value (); // TODO
}

auto TypeObj::noFactory (const TypeObj&, int, Value[]) -> Value {
    assert(false);
    return Value ();
}

TypeObj const Object::info ("<object>");
auto Object::type () const -> TypeObj const& { return info; }

//CG< builtin-types lib/monty/xmonty.h
const TypeObj      NoneObj::info ("<none>");

const TypeObj  BoolObj::info ("bool", BoolObj::create, &BoolObj::attrs);
const TypeObj   IntObj::info ("int", IntObj::create, &IntObj::attrs);
const TypeObj  TypeObj::info ("type", TypeObj::create, &TypeObj::attrs);

const TypeObj&      NoneObj::type () const { return info; }
const TypeObj&      BoolObj::type () const { return info; }
const TypeObj&       IntObj::type () const { return info; }
const TypeObj&      TypeObj::type () const { return info; }
//CG>
