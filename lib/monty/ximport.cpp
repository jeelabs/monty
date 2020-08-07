// import.cpp - importing, loading, and bytecode objects

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "xmonty.h"

using namespace Monty;

struct Monty::Bytecode : Object {
    static Type const info;
    auto type () const -> Type const& override;

    //Bytecode ();

    void marker () const override {} // TODO
};

Type const Bytecode::info ("<bytecode>");
auto Bytecode::type () const -> Type const& { return info; }

auto Callable::frameSize () const -> size_t {
    return 100; // TODO
}

void Callable::marker () const {
    callee.marker();
}

Module::Module (Value v) {
    auto& bc = v.asType<Bytecode>();

    (void) bc; // TODO
}
