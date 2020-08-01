// type.cpp - basic object types and type system

#include <cassert>
#include <cstdint>
#include <cstdlib>

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

auto Object::unop (UnOp) const -> Value {
    assert(false);
    return Value ();
}

auto Object::binop (BinOp, Value) const -> Value {
    assert(false);
    return Value ();
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

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// TODO added to satisfy linker

struct Monty::LookupObj {
    // TODO
};

LookupObj const BoolObj::attrs;
LookupObj const IntObj::attrs;
LookupObj const TypeObj::attrs;

auto NoneObj::unop (UnOp) const -> Value {
    return Value (); // TODO
}

auto BoolObj::unop (UnOp) const -> Value {
    return Value (); // TODO
}

auto IntObj::unop (UnOp) const -> Value {
    return Value (); // TODO
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
