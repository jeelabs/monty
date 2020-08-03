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

void moveBytes (void* to, void const* from, size_t len) {
    memmove(to, from, len);
}

void wipeBytes (void* ptr, size_t len) {
    memset(ptr, 0, len);
}

template< typename T >
struct ItemsOf : Array::Items {
    auto get (Chunk const& chk, int idx) const -> Val override {
        auto chunk = (ChunkOf<T>&) chk;
        return chunk[idx];
    }
    void set (Chunk& chk, int idx, Val val) override {
        auto chunk = (ChunkOf<T>&) chk;
        ins(chk, idx + 1, 0); // grow if needed
        chunk[idx] = val;
    }
    void ins (Chunk& chk, size_t idx, int num =1) override {
        //auto chunk = (ChunkOf<T>&) chk;
        // TODO
    }
    void del (Chunk& chk, size_t idx, int num =1) override {
        //auto chunk = (ChunkOf<T>&) chk;
        // TODO
    }
};

struct ItemsOfObj : ItemsOf<Val> {
    void marker (Chunk const& chk) const override {
        auto chunk = (ChunkOf<Val>&) chk;
        for (size_t i = 0; i < chunk.length(); ++i)
            if (chunk[i].isObj())
                mark(chunk[i].obj());
    }
};

#if 0 // TODO how?
struct ItemsOfChunk : ItemsOf<Chunk> {
    void marker (Chunk const& chk) const override {
        auto chunk = (ChunkOf<Chunk>&) chk;
        for (size_t i = 0; i < chunk.length(); ++i)
            mark(chunk[i].obj());
    }
};
#endif

static auto typedItem (char c) -> Array::Items& {
    Array::Items* p = nullptr;
    switch (c) {
        case 'b': p = new ItemsOf<int8_t>; break;
        case 'B': p = new ItemsOf<uint8_t>; break;
        case 'h': p = new ItemsOf<int16_t>; break;
        case 'H': p = new ItemsOf<uint16_t>; break;
        case 'i': p = new ItemsOf<int16_t>; break;
        case 'I': p = new ItemsOf<uint16_t>; break;
        case 'l': p = new ItemsOf<int32_t>; break;
        case 'L': p = new ItemsOf<uint32_t>; break;

        case 'o': p = new ItemsOfObj; break;
        //case 'c': p = new ItemsOfChunk; break;
        
        //case 'P': // 1b: Packed
        //case 'T': // 2b: Tiny
        //case 'N': // 4b: Nibble
    }
    assert(p != nullptr);
    return *p;
}

Array::Array (char type) : items (typedItem(type)), chunk (vec) {
    // TODO
    
    chunk.v.vec.resize(100);
    chunk.v.vec.move(1,2,3);
    chunk.u8.vec.move(1,2,3);
    chunk.c.vec.move(1,2,3);
}
