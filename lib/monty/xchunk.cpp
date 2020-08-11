// chunk.cpp - typed and chunked access to vectors

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "xmonty.h"

using namespace Monty;

void Monty::mark (VofV const& vec) {
    for (size_t i = 0; i < vec.cap(); ++i)
        if (vec[i].isObj())
            mark(vec[i].obj());
}

void Monty::mark (CofV const& chunk) {
    for (size_t i = 0; i < chunk.length(); ++i)
        if (chunk[i].isObj())
            mark(chunk[i].obj());
}
