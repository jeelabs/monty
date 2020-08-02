// type.cpp - basic object types and type system

#include <cassert>
#include <cstdint>
#include <cstdlib>

#include "xmonty.h"

using namespace Monty;

const None None::noneObj;
const Bool Bool::falseObj;
const Bool Bool::trueObj;

const Val Val::None {None::noneObj};
const Val Val::False{Bool::falseObj};
const Val Val::True {Bool::trueObj};

Val::Val (int arg) : v ((arg << 1) | 1) {
    if ((int) *this != arg)
        *this = new Long (arg);
}

Val::Val (char const* arg) : v (((uintptr_t) arg << 2) | 2) {
    assert((char const*) *this == arg);
}

auto Val::check (Type const& t) const -> bool {
    return isObj() && &obj().type() == &t;
}

void Val::verify (Type const& t) const {
    assert(check(t));
}

auto Object::unop (UnOp) const -> Val {
    assert(false);
    return Val{};
}

auto Object::binop (BinOp, Val) const -> Val {
    assert(false);
    return Val{};
}

auto Type::noFactory (const Type&, int, Val[]) -> Val {
    assert(false);
    return Val{};
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

auto None::unop (UnOp) const -> Val {
    return Val{}; // TODO
}

auto Bool::unop (UnOp) const -> Val {
    return Val{}; // TODO
}

auto Long::unop (UnOp) const -> Val {
    return Val{}; // TODO
}

// TODO change argc/argv to: Chunk<Val> const& args

auto Bool::create (const Type&, int argc, Val argv[]) -> Val {
    return Val{}; // TODO
}

auto Long::create (const Type&, int argc, Val argv[]) -> Val {
    return Val{}; // TODO
}

auto Type::create (const Type&, int argc, Val argv[]) -> Val {
    return Val{}; // TODO
}

auto Array::create (const Type&, int argc, Val argv[]) -> Val {
    return Val{}; // TODO
}
