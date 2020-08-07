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
        *this = new Fixed (arg);
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
    return Value {};
}

auto Object::binop (BinOp, Value) const -> Value {
    assert(false);
    return Value {};
}

auto None::unop (UnOp) const -> Value {
    return Value {}; // TODO
}

auto Bool::unop (UnOp) const -> Value {
    return Value {}; // TODO
}

auto Fixed::unop (UnOp) const -> Value {
    return Value {}; // TODO
}

auto Fixed::binop (BinOp, Value) const -> Value {
    return Value {}; // TODO
}

void Lookup::marker () const {
    for (size_t i = 0; i < count; ++i)
        mark(items[i].v);
}

auto Type::noFactory (const Type&, ChunkOf<Value> const&) -> Value {
    assert(false);
    return Value {};
}

Type const Object::info ("<object>");
auto Object::type () const -> Type const& { return info; }

//CG< builtin-types lib/monty/xmonty.h
Type const    BoundMeth::info ("<boundmeth>");
Type const     Callable::info ("<callable>");
Type const      Context::info ("<context>");
Type const     Function::info ("<function>");
Type const       Lookup::info ("<lookup>");
Type const       Module::info ("<module>");
Type const         None::info ("<none>");

Type const    Array::info ("array", Array::create, &Array::attrs);
Type const     Bool::info ("bool", Bool::create, &Bool::attrs);
Type const     Dict::info ("dict", Dict::create, &Dict::attrs);
Type const    Fixed::info ("int", Fixed::create, &Fixed::attrs);
Type const      Set::info ("set", Set::create, &Set::attrs);
Type const     Type::info ("type", Type::create, &Type::attrs);

auto    BoundMeth::type () const -> Type const& { return info; }
auto     Callable::type () const -> Type const& { return info; }
auto      Context::type () const -> Type const& { return info; }
auto     Function::type () const -> Type const& { return info; }
auto       Lookup::type () const -> Type const& { return info; }
auto       Module::type () const -> Type const& { return info; }
auto         None::type () const -> Type const& { return info; }
auto        Array::type () const -> Type const& { return info; }
auto         Bool::type () const -> Type const& { return info; }
auto         Dict::type () const -> Type const& { return info; }
auto        Fixed::type () const -> Type const& { return info; }
auto          Set::type () const -> Type const& { return info; }
auto         Type::type () const -> Type const& { return info; }
//CG>

static const Lookup::Item builtins [] = {
    //CG< builtin-emit 1
    { "array", &Array::info },
    { "bool", &Bool::info },
    { "dict", &Dict::info },
    { "int", &Fixed::info },
    { "set", &Set::info },
    { "type", &Type::info },
    //CG>
#if 0
    { "monty", &m_monty },
    { "machine", &m_machine },
#if INCLUDE_NETWORK
    { "network", &m_network },
#endif
#if INCLUDE_SDCARD
    { "sdcard", &m_sdcard },
#endif
#endif
};

Lookup const builtinDict (builtins, sizeof builtins / sizeof *builtins);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// TODO added to satisfy linker

Lookup const  Bool::attrs {nullptr, 0};
Lookup const Fixed::attrs {nullptr, 0};
Lookup const  Type::attrs {nullptr, 0};
Lookup const Array::attrs {nullptr, 0};
Lookup const   Set::attrs {nullptr, 0};
Lookup const  Dict::attrs {nullptr, 0};

auto Bool::create (const Type&, ChunkOf<Value> const& args) -> Value {
    return Value {}; // TODO
}

auto Fixed::create (const Type&, ChunkOf<Value> const& args) -> Value {
    return Value {}; // TODO
}

auto Type::create (const Type&, ChunkOf<Value> const& args) -> Value {
    return Value {}; // TODO
}

auto Array::create (const Type&, ChunkOf<Value> const& args) -> Value {
    // TODO
    return new Array;
}

auto Set::create (const Type&, ChunkOf<Value> const& args) -> Value {
    // TODO
    return new Set;
}

auto Dict::create (const Type&, ChunkOf<Value> const& args) -> Value {
    // TODO
    return new Dict;
}
