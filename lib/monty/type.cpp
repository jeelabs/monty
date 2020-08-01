// type.cpp - basic object types and type system

#include <cassert>
#include <cstdint>
#include <cstdlib>

#include "xmonty.h"

using namespace Monty;

const Value Value::None =  NoneObj::noneObj;
const Value Value::False = BoolObj::falseObj;
const Value Value::True  = BoolObj::trueObj;

auto Value::check (TypeObj const& t) const -> bool {
    return isObj() && &obj().type() == &t;
}

void Value::verify (TypeObj const& t) const {
    assert(check(t));
}
