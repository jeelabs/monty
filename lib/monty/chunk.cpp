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
        if (chk.hasVals())
            mark((ChunkOf<Val> const&) chk);
        else if (chk.hasChunks())
            mark((ChunkOf<Chunk> const&) chk);
    }
}

void Monty::mark (ChunkOf<Val> const& chunk) {
    for (size_t i = 0; i < chunk.length(); ++i)
        if (chunk[i].isObj())
            mark(chunk[i].obj());
}
