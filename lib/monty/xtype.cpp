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
    return Value{};
}

auto Object::binop (BinOp, Value) const -> Value {
    assert(false);
    return Value{};
}

auto None::unop (UnOp) const -> Value {
    return Value{}; // TODO
}

auto Bool::unop (UnOp) const -> Value {
    return Value{}; // TODO
}

auto Fixed::unop (UnOp) const -> Value {
    return Value{}; // TODO
}

auto Fixed::binop (BinOp, Value) const -> Value {
    return Value{}; // TODO
}

void Lookup::marker () const {
    for (size_t i = 0; i < count; ++i)
        mark(items[i].v);
}

auto Type::noFactory (const Type&, int, Value[]) -> Value {
    assert(false);
    return Value{};
}

Type const Object::info ("<object>");
auto Object::type () const -> Type const& { return info; }

//CG< builtin-types lib/monty/xmonty.h
const TypeObj    BoundMeth::info ("<boundmeth>");
const TypeObj     Callable::info ("<callable>");
const TypeObj      Context::info ("<context>");
const TypeObj     Function::info ("<function>");
const TypeObj       Lookup::info ("<lookup>");
const TypeObj       Module::info ("<module>");
const TypeObj         None::info ("<none>");

const TypeObj    Array::info ("array", Array::create, &Array::attrs);
const TypeObj     Bool::info ("bool", Bool::create, &Bool::attrs);
const TypeObj     Dict::info ("dict", Dict::create, &Dict::attrs);
const TypeObj    Fixed::info ("int", Fixed::create, &Fixed::attrs);
const TypeObj      Set::info ("set", Set::create, &Set::attrs);
const TypeObj     Type::info ("type", Type::create, &Type::attrs);

const TypeObj&    BoundMeth::type () const { return info; }
const TypeObj&     Callable::type () const { return info; }
const TypeObj&      Context::type () const { return info; }
const TypeObj&     Function::type () const { return info; }
const TypeObj&       Lookup::type () const { return info; }
const TypeObj&       Module::type () const { return info; }
const TypeObj&         None::type () const { return info; }
const TypeObj&        Array::type () const { return info; }
const TypeObj&         Bool::type () const { return info; }
const TypeObj&         Dict::type () const { return info; }
const TypeObj&        Fixed::type () const { return info; }
const TypeObj&          Set::type () const { return info; }
const TypeObj&         Type::type () const { return info; }
//CG>

static const LookupObj::Item builtins [] = {
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

LookupObj const builtinDict (builtins, sizeof builtins / sizeof *builtins);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// TODO added to satisfy linker

Lookup const  Bool::attrs {nullptr, 0};
Lookup const Fixed::attrs {nullptr, 0};
Lookup const  Type::attrs {nullptr, 0};
Lookup const Array::attrs {nullptr, 0};
Lookup const   Set::attrs {nullptr, 0};
Lookup const  Dict::attrs {nullptr, 0};

// TODO change argc/argv to: ChunkOf<Value> const& args

auto Bool::create (const Type&, int argc, Value argv[]) -> Value {
    return Value{}; // TODO
}

auto Fixed::create (const Type&, int argc, Value argv[]) -> Value {
    return Value{}; // TODO
}

auto Type::create (const Type&, int argc, Value argv[]) -> Value {
    return Value{}; // TODO
}

auto Array::create (const Type&, int argc, Value argv[]) -> Value {
    // TODO
    return new Array;
}

auto Set::create (const Type&, int argc, Value argv[]) -> Value {
    // TODO
    return new Set;
}

auto Dict::create (const Type&, int argc, Value argv[]) -> Value {
    // TODO
    return new Dict;
}
