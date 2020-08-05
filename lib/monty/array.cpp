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

// this is a "delegating constructor", i.e. it calls on another constructor
Chunk::Chunk (Value v) : Chunk ('A', v.asType<Array>()) {} // TODO is 'A' correct?

Chunk::operator Value () const {
    auto p = Array::create(typ); // TODO what about Vec ownership & lifetime?
    assert(p != nullptr);
    return *p;
}

template< char C, typename T >
struct SegmentOf : Segment, ChunkOf<T> {
    using CoT = ChunkOf<T>;

    SegmentOf (Vec& v) : CoT (C, v) {}

    auto typ () const -> char override { return C; }
    auto vec () const -> Vec& override { return CoT::asVec(); }

    auto get (int i) const -> Value  override { return CoT::operator[](i); }
    void set (int i, Value v)        override { CoT::operator[](i) = v; }
    void ins (size_t i, size_t n =1) override { CoT::insert(i, n); }
    void del (size_t i, size_t n =1) override { CoT::remove(i, n); }
};

void Array::marker () const {
    switch (segment.typ()) {
        case 'V': mark(((SegmentOf<'V',Value> const&) segment)); break;
        case 'C': mark(((SegmentOf<'C',Chunk> const&) segment)); break;
    }
}

auto Segment::create (Vec& v, char c) -> Segment& {
    Segment* p = nullptr;
    switch (c) {
        case 'b': p = new SegmentOf<'b',int8_t>   (v); break;
        case 'B': p = new SegmentOf<'B',uint8_t>  (v); break;
        case 'h': p = new SegmentOf<'h',int16_t>  (v); break;
        case 'H': p = new SegmentOf<'H',uint16_t> (v); break;
        case 'i': p = new SegmentOf<'i',int16_t>  (v); break;
        case 'I': p = new SegmentOf<'I',uint16_t> (v); break;
        case 'l': p = new SegmentOf<'l',int32_t>  (v); break;
        case 'L': p = new SegmentOf<'L',uint32_t> (v); break;
        case 'V': p = new SegmentOf<'V',Value>    (v); break;
        case 'C': p = new SegmentOf<'C',Chunk>    (v); break;
        
        //case 'P': // 1b: Packed
        //case 'T': // 2b: Tiny
        //case 'N': // 4b: Nibble
    }
    assert(p != nullptr);
    return *p;
}


auto Array::create (char c) -> Array* {
    auto p = new Array (c);

assert(p != nullptr);
p->segment.vec().resize(100);
//p->segment.chunk.asVec<Value>().move(1,2,3);
//p->segment.chunk.asVec<Chunk>().move(1,2,3);

    return p;
}
