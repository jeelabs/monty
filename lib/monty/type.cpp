// type.cpp - basic object types and type system

#include <cassert>
#include <cstdint>
#include <cstdlib>

#include "xmonty.h"

using namespace Monty;

const NoneObj NoneObj::noneObj;
const BoolObj BoolObj::falseObj;
const BoolObj BoolObj::trueObj;

const Val Val::None =  NoneObj::noneObj;
const Val Val::False = BoolObj::falseObj;
const Val Val::True  = BoolObj::trueObj;

Val::Val (int arg) : v ((arg << 1) | 1) {
    if ((int) *this != arg)
        *this = new IntObj (arg);
}

Val::Val (char const* arg) : v (((uintptr_t) arg << 2) | 2) {
    assert((char const*) *this == arg);
}

auto Val::check (TypeObj const& t) const -> bool {
    return isObj() && &obj().type() == &t;
}

void Val::verify (TypeObj const& t) const {
    assert(check(t));
}

auto Object::unop (UnOp) const -> Val {
    assert(false);
    return Val ();
}

auto Object::binop (BinOp, Val) const -> Val {
    assert(false);
    return Val ();
}

auto TypeObj::noFactory (const TypeObj&, int, Val[]) -> Val {
    assert(false);
    return Val ();
}

TypeObj const Object::info ("<object>");
auto Object::type () const -> TypeObj const& { return info; }

ArrayObj::ArrayObj (char atype) {
    // TODO
}

//CG< builtin-types lib/monty/xmonty.h
const TypeObj      NoneObj::info ("<none>");

const TypeObj ArrayObj::info ("array", ArrayObj::create, &ArrayObj::attrs);
const TypeObj  BoolObj::info ("bool", BoolObj::create, &BoolObj::attrs);
const TypeObj   IntObj::info ("int", IntObj::create, &IntObj::attrs);
const TypeObj  TypeObj::info ("type", TypeObj::create, &TypeObj::attrs);

const TypeObj&      NoneObj::type () const { return info; }
const TypeObj&     ArrayObj::type () const { return info; }
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
LookupObj const ArrayObj::attrs;

auto NoneObj::unop (UnOp) const -> Val {
    return Val (); // TODO
}

auto BoolObj::unop (UnOp) const -> Val {
    return Val (); // TODO
}

auto IntObj::unop (UnOp) const -> Val {
    return Val (); // TODO
}

auto BoolObj::create (const TypeObj&, int argc, Val argv[]) -> Val {
    return Val (); // TODO
}

auto IntObj::create (const TypeObj&, int argc, Val argv[]) -> Val {
    return Val (); // TODO
}

auto TypeObj::create (const TypeObj&, int argc, Val argv[]) -> Val {
    return Val (); // TODO
}

auto ArrayObj::create (const TypeObj&, int argc, Val argv[]) -> Val {
    return Val (); // TODO
}
