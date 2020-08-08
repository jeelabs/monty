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
    //*this = v.asType<Array>().segment;
    return *this;
}

// cannot be an abstract class because the size is needed for VecOf<Segment>
auto Segment::typ () const -> char     { assert(false); }
auto Segment::len () const -> size_t   { assert(false); }
auto Segment::get (int) const -> Value { assert(false); }
void Segment::set (int, Value)         { assert(false); }
void Segment::ins (size_t, size_t)     { assert(false); }
void Segment::del (size_t, size_t)     { assert(false); }

// all SegmentOf<T> instances must have the same size as the Segment base class
static_assert (sizeof (SegmentOf<'b',int8_t>)  == sizeof (Segment), "int8_t?");
static_assert (sizeof (SegmentOf<'V',Value>)   == sizeof (Segment), "Value?");
static_assert (sizeof (SegmentOf<'S',Segment>) == sizeof (Segment), "Segment?");

auto Segment::make (char c, Vec& v) -> Segment {
    switch (c) {
        case 'b': return SegmentOf<'b',int8_t>   (v);
        case 'B': return SegmentOf<'B',uint8_t>  (v);
        case 'h': return SegmentOf<'h',int16_t>  (v);
        case 'H': return SegmentOf<'H',uint16_t> (v);
        case 'i': return SegmentOf<'i',int16_t>  (v);
        case 'I': return SegmentOf<'I',uint16_t> (v);
        case 'l': return SegmentOf<'l',int32_t>  (v);
        case 'L': return SegmentOf<'L',uint32_t> (v);
        case 'V': return SegmentOf<'V',Value>    (v);
        case 'S': return SegmentOf<'S',Segment>  (v);
        
        //case 'P': // 1b: Packed
        //case 'T': // 2b: Tiny
        //case 'N': // 4b: Nibble
    }
    assert(false);
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
