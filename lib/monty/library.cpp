// library.cpp - runtime library functions for several datatypes

#include "monty.h"
#include <cassert>

using namespace monty;

static auto f_bytes_count (ArgVec const& args) -> Value {
    if (args.size() != 2)
        return {E::TypeError, "wrong arg count", 2};
    return 123;
}

void monty::libInstall () {
    printf("in libInstall\n");

    Bytes::info.at(Q( 74,"count")) = f_bytes_count;
}
