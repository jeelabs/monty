// data.cpp - basic object data types

#include "monty.h"

using namespace monty;
using monty::Q;

None const None::nullObj;
Bool const Bool::falseObj;
Bool const Bool::trueObj;

Value const monty::Null  {None::nullObj};
Value const monty::False {Bool::falseObj};
Value const monty::True  {Bool::trueObj};
