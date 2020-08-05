// chunk.cpp - typed and chunked access to vectors

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "xmonty.h"

using namespace Monty;

void Monty::mark (ChunkOf<Chunk> const& chunk) {
    for (size_t i = 0; i < chunk.length(); ++i) {
        auto& chk = chunk[i];
        if (chk.hasChunks())
            mark((ChunkOf<Chunk> const&) chk);
        else if (chk.hasVals())
            mark((ChunkOf<Value> const&) chk);
    }
}

void Monty::mark (ChunkOf<Value> const& chunk) {
    for (size_t i = 0; i < chunk.length(); ++i)
        if (chunk[i].isObj())
            mark(chunk[i].obj());
}
