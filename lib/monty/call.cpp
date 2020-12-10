// call.cpp - functions, methods, and other callables

#include "monty.h"
#include <cassert>

using namespace monty;

#if 0
Callable::Callable (Value callee, Tuple* t, Dict* d, Module* mod)
        : mo (mod != nullptr ? *mod : Interp::context->globals()),
          bc (callee.asType<Bytecode>()), pos (t), kw (d) {
}

auto Callable::call (ArgVec const& args) const -> Value {
    auto ctx = Interp::context;
    auto coro = bc.isGenerator();
    if (coro)
        ctx = new Context;

    ctx->enter(*this);

    int nPos = bc.numArgs(0);
    int nDef = bc.numArgs(1);
    int nKwo = bc.numArgs(2);
    int nc = bc.numCells();

    for (int i = 0; i < nPos + nc; ++i)
        if (i < args.num)
            ctx->fastSlot(i) = args[i];
        else if (pos != nullptr && (uint32_t) i < nDef + pos->fill)
            ctx->fastSlot(i) = (*pos)[i+nDef-nPos];

    if (bc.hasVarArgs())
        ctx->fastSlot(nPos+nKwo) =
            Tuple::create({args.vec, args.num-nPos, args.off+nPos});

    uint8_t const* cellMap = bc.start() - nc;
    for (int i = 0; i < nc; ++i) {
        auto slot = cellMap[i];
        ctx->fastSlot(slot) = new Cell (ctx->fastSlot(slot));
    }

    return coro ? ctx : Value {};
}
#endif

void Callable::marker () const {
#if 0 //XXX
    mo.marker();
    mark(bc);
#endif
    mark(pos);
    mark(kw);
}
