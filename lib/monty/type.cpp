// type.cpp - basic object types and type system

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "xmonty.h"

namespace Monty { // TODO move into defs.h
#include "defs.h"
}

using namespace Monty;

const None None::noneObj;
const Bool Bool::falseObj;
const Bool Bool::trueObj;

const Value Value::None {None::noneObj};
const Value Value::False{Bool::falseObj};
const Value Value::True {Bool::trueObj};

Value::Value (int arg) : v ((arg << 1) | 1) {
    if ((int) *this != arg)
        *this = new Long (arg);
}

Value::Value (char const* arg) : v (((uintptr_t) arg << 2) | 2) {
    assert((char const*) *this == arg);
}

auto Value::check (Type const& t) const -> bool {
    return isObj() && &obj().type() == &t;
}

void Value::verify (Type const& t) const {
    assert(check(t));
}

bool Value::truthy () const {
    switch (tag()) {
        case Value::Nil: return false;
        case Value::Int: return (int) *this != 0;
        case Value::Str: return *(const char*) *this != 0;
        case Value::Obj: return obj().unop(UnOp::Bool).isTrue();
    }
    assert(false);
}

auto Object::unop (UnOp) const -> Value {
    assert(false);
    return Value{};
}

auto Object::binop (BinOp, Value) const -> Value {
    assert(false);
    return Value{};
}

auto Type::noFactory (const Type&, int, Value[]) -> Value {
    assert(false);
    return Value{};
}

Type const Object::info ("<object>");
auto Object::type () const -> Type const& { return info; }

//CG< builtin-types lib/monty/xmonty.h
const TypeObj         None::info ("<none>");

const TypeObj    Array::info ("array", Array::create, &Array::attrs);
const TypeObj     Bool::info ("bool", Bool::create, &Bool::attrs);
const TypeObj     Long::info ("int", Long::create, &Long::attrs);
const TypeObj     Type::info ("type", Type::create, &Type::attrs);

const TypeObj&         None::type () const { return info; }
const TypeObj&        Array::type () const { return info; }
const TypeObj&         Bool::type () const { return info; }
const TypeObj&         Long::type () const { return info; }
const TypeObj&         Type::type () const { return info; }
//CG>

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// TODO added to satisfy linker

struct Monty::Lookup {
    // TODO
};

Lookup const Bool::attrs;
Lookup const Long::attrs;
Lookup const Type::attrs;
Lookup const Array::attrs;

auto None::unop (UnOp) const -> Value {
    return Value{}; // TODO
}

auto Bool::unop (UnOp) const -> Value {
    return Value{}; // TODO
}

auto Long::unop (UnOp) const -> Value {
    return Value{}; // TODO
}

// TODO change argc/argv to: Chunk<Value> const& args

auto Bool::create (const Type&, int argc, Value argv[]) -> Value {
    return Value{}; // TODO
}

auto Long::create (const Type&, int argc, Value argv[]) -> Value {
    return Value{}; // TODO
}

auto Type::create (const Type&, int argc, Value argv[]) -> Value {
    return Value{}; // TODO
}

auto Array::create (const Type&, int argc, Value argv[]) -> Value {
    return Value{}; // TODO
}
