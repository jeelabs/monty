// array.cpp - vectors, arrays, and other derived types

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "xmonty.h"

using namespace Monty;

Vector::Vector (size_t bits) {
    while (bits > (1U << extra))
        ++extra;
}

auto Vector::getPtr (int idx) const -> uint8_t* {
    if (idx < 0)
        idx += fill;
    return ptr() + widthOf(idx);
}

auto Vector::getInt (int idx) const -> int {
    auto p = getPtr(idx);
    switch (extra) {
        case 3: return *(int8_t*) p;
        case 4: return *(int16_t*) p;
        case 5: return *(int32_t*) p;
    }
    return 0;
}

auto Vector::getIntU (int idx) const -> uint32_t {
    if (idx < 0)
        idx += fill;
    auto p = getPtr(idx);
    switch (extra) {
        case 0: return (*p >> (idx&7)) & 0x1;
        case 1: return (*p >> 2*(idx&3)) & 0x3;
        case 2: return (*p >> 4*(idx&1)) & 0xF;
        case 3: return *p;
        case 4: return *(uint16_t*) p;
        case 5: return *(uint32_t*) p;
    }
    return 0;
}

void Vector::set (int idx, const void* ptr) {
    if (idx < 0)
        idx += fill;
    if (widthOf(idx) >= cap()) {
        auto n = (cap() << 3) >> extra;
        ins(n, idx + 1 - n);
    }
    auto p = getPtr(idx);
    switch (extra) {
        case 0: *p = (*p & ~(0x1 << (idx&7))) |
                        ((*(const uint8_t*) ptr & 0x1) << (idx&7)); return;
        case 1: *p = (*p & ~(0x3 << 2*(idx&3))) |
                        ((*(const uint8_t*) ptr & 0x3) << 2*(idx&3)); return;
        case 2: *p = (*p & ~(0xF << 4*(idx&1))) |
                        ((*(const uint8_t*) ptr & 0xF) << 4*(idx&1)); return;
        case 3: *p = *(const uint8_t*) ptr; return;
        case 4: *(uint16_t*) p = *(const uint16_t*) ptr; return;
        case 5: *(uint32_t*) p = *(const uint32_t*) ptr; return;
    }
    memcpy(getPtr(idx), ptr, widthOf(1));
}

void Vector::set (int idx, int val) {
    assert(1U << extra <= 8 * sizeof val);
    set(idx, &val); // TODO assumes little-endian byte order
}

void Vector::ins (size_t idx, int num) {
    if (fill < idx) {
        num += idx - fill;
        idx = fill;
    }
    if (num <= 0)
        return;
    auto needed = widthOf(fill + num);
    if (needed > cap()) {
        resize(needed);
        assert(ptr() != nullptr && cap() >= needed);
    }
    auto p = getPtr(idx);
    assert(extra >= 3); // TODO
    assert (fill >= idx);
    memmove(getPtr(idx + num), p, widthOf(fill - idx));
    memset(p, 0, widthOf(num));
    fill += num;
}

void Vector::del (size_t idx, int num) {
    if (num <= 0)
        return;
    fill -= num;
    assert(extra >= 3); // TODO
    memmove(getPtr(idx), getPtr(idx + num), widthOf(fill - idx));
}

template< char C, typename T >
struct ArrayOf : Array {
    auto get (int idx) const -> Val override {
        auto& chk = (ChunkOf<T>&) chunk;
        return chk[idx];
    }
    void set (int idx, Val val) override {
        auto& chk = (ChunkOf<T>&) chunk;
        ins(idx + 1, 0); // grow if needed
        chk[idx] = val;
    }
    void ins (size_t idx, size_t num =1) override {
        auto& chk = (ChunkOf<T>&) chunk;
        chk.insert(idx, num);
    }
    void del (size_t idx, size_t num =1) override {
        auto& chk = (ChunkOf<T>&) chunk;
        chk.remove(idx, num);
    }
};

void Monty::markVals (ChunkOf<Val> const& chunk) {
    for (size_t i = 0; i < chunk.length(); ++i)
        if (chunk[i].isObj())
            mark(chunk[i].obj());
}

struct ArrayOfVal : ArrayOf<'V',Val> {
    void marker () const override {
        Array::marker();
        markVals((ChunkOf<Val> const&) chunk);
    }
};

struct Chunk {
    union {
        ChunkOf<Val> v;
        ChunkOf<Chunk> c;
    } &u;
    char t;
};

static void markChunks (ChunkOf<Chunk> const& chunk) {
    for (size_t i = 0; i < chunk.length(); ++i)
        switch (chunk[i].t) {
            case 'V': markVals(chunk[i].u.v.vec); break;
            case 'C': markChunks(chunk[i].u.c.vec); break;
        }
}

struct ArrayOfChunk : Array { // TODO figure out how to use ArrayOf<'C',Chunk>
    auto get (int idx) const -> Val override {
        auto& chk = (ChunkOf<Chunk>&) chunk;
        auto* a = create(chk[idx].t);
        assert(a != nullptr);
        // a->chunk = chk[idx]; TODO can't seem to get past the ref init error
        memcpy(&a->chunk, &chk[idx], sizeof (Chunk));
        return a;
    }
    void set (int idx, Val val) override {
        auto& chk = (ChunkOf<Chunk>&) chunk;
        auto& a = val.asType<Array>();
        // chk[idx] = a.chunk; TODO can't seem to get past the ref init error
        memcpy(&chk[idx], &a.chunk, sizeof (Chunk));
    }
    void ins (size_t idx, size_t num =1) override {
        // TODO same code as in ArrayOf<T>
        auto& chk = (ChunkOf<Chunk>&) chunk;
        chk.insert(idx, num);
    }
    void del (size_t idx, size_t num =1) override {
        // TODO same code as in ArrayOf<T>
        auto& chk = (ChunkOf<Chunk>&) chunk;
        chk.remove(idx, num);
    }
    void marker () const override {
        Array::marker();
        markChunks((ChunkOf<Chunk>&) chunk);
    }
};

auto Array::create (char c) -> Array* {
    Array* p = nullptr;
    switch (c) {
        case 'b': p = new ArrayOf<'b',int8_t>; break;
        case 'B': p = new ArrayOf<'B',uint8_t>; break;
        case 'h': p = new ArrayOf<'h',int16_t>; break;
        case 'H': p = new ArrayOf<'H',uint16_t>; break;
        case 'i': p = new ArrayOf<'i',int16_t>; break;
        case 'I': p = new ArrayOf<'I',uint16_t>; break;
        case 'l': p = new ArrayOf<'l',int32_t>; break;
        case 'L': p = new ArrayOf<'L',uint32_t>; break;

        case 'V': p = new ArrayOfVal; break;
        case 'C': p = new ArrayOfChunk; break;
        
        //case 'P': // 1b: Packed
        //case 'T': // 2b: Tiny
        //case 'N': // 4b: Nibble
    }
    
assert(p != nullptr);
p->chunk.vec.resize(100);
((VecOf<Vec>&) p->chunk.vec).move(1,2,3);

    return p;
}
