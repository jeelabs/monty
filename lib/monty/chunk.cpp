// chunk.cpp - typed and chunked access to vectors

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "xmonty.h"

using namespace Monty;

Segment::operator Value () const {
    auto p = new Array ('A'); // TODO type 'A'? Vec ownership & lifetime?
    assert(p != nullptr);
    return *p;
}

Segment& Segment::operator= (Value v) {
    *this = v.asType<Array>().segment;
    return *this;
}

// must be defined because the size is needed for VecOf<Segment>
// careful: SegmentOf<T> instances must all have this same size!
auto Segment::typ () const -> char     { assert(false); }
auto Segment::vec () const -> Vec&     { assert(false); }
auto Segment::get (int) const -> Value { assert(false); }
void Segment::set (int, Value)         { assert(false); }
void Segment::ins (size_t, size_t)     { assert(false); }
void Segment::del (size_t, size_t)     { assert(false); }

template< char C, typename T >
struct SegmentOf : Segment, ChunkOf<T> {
    using CoT = ChunkOf<T>;

    SegmentOf (Vec& v) : CoT (v) {}

    auto typ () const -> char override { return C; }
    auto vec () const -> Vec& override { return CoT::asVec(); }

    auto get (int i) const -> Value  override { return CoT::operator[](i); }
    void set (int i, Value v)        override { CoT::operator[](i) = v; }
    void ins (size_t i, size_t n =1) override { CoT::insert(i, n); }
    void del (size_t i, size_t n =1) override { CoT::remove(i, n); }
};

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
        case 'S': p = new SegmentOf<'S',Segment>  (v); break;
        
        //case 'P': // 1b: Packed
        //case 'T': // 2b: Tiny
        //case 'N': // 4b: Nibble
    }
    assert(p != nullptr);
    return *p;
}

void Monty::mark (Segment const& seg) {
    switch (seg.typ()) {
        case 'S':
            mark((ChunkOf<Segment>&) (SegmentOf<'S',Segment> const&) seg);
            break;
        case 'V':
            mark((ChunkOf<Value>&) (SegmentOf<'V',Value> const&) seg);
            break;
    }
}

void Monty::mark (ChunkOf<Segment> const& chunk) {
    for (size_t i = 0; i < chunk.length(); ++i) {
        mark(chunk[i]);
    }
}

void Monty::mark (ChunkOf<Value> const& chunk) {
    for (size_t i = 0; i < chunk.length(); ++i)
        if (chunk[i].isObj())
            mark(chunk[i].obj());
}
