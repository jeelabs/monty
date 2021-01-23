// call.cpp - functions, methods, contexts, and interpreter state

#include "monty.h"
#include <cassert>

using namespace monty;

auto BoundMeth::call (ArgVec const& args) const -> Value {
    assert(args.num > 0 && this == &args[-1].obj());
    args[-1] = self; // overwrites the entry before first arg
    return meth.call({args.vec, (int) args.num + 1, (int) args.off - 1});
}

Closure::Closure (Object const& f, ArgVec const& args)
        : func (f) {
    insert(0, args.num);
    for (int i = 0; i < args.num; ++i)
        begin()[i] = args[i];
}

auto Closure::call (ArgVec const& args) const -> Value {
    int n = size();
    assert(n > 0);
    Vector v;
    v.insert(0, n + args.num);
    for (int i = 0; i < n; ++i)
        v[i] = begin()[i];
    for (int i = 0; i < args.num; ++i)
        v[n+i] = args[i];
    return func.call({v, n + args.num});
}
