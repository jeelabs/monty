// array.cpp - arrays, lists, dicts, and other derived types

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "xmonty.h"

using namespace Monty;

int abc = 123; // avoid "has no symbols" warning TODO remove when there is code

#if 0 // TODO obsolete
    struct Vector : private Vec {
        Vector (size_t bits);

        auto length () const -> size_t { return fill; }
        int width () const { auto b = 1<<extra; return b < 8 ? -b : b/8; }
        size_t widthOf (int num) const { return ((num << extra) + 7) >> 3; }

        auto getPtr (int idx) const -> uint8_t*;
        auto getInt (int idx) const -> int;
        auto getIntU (int idx) const -> uint32_t;

        void set (int idx, void const* ptr);
        void set (int idx, int val);

        void ins (size_t idx, int num =1);
        void del (size_t idx, int num =1);

    protected:
        uint32_t fill{0}; // in elements
    };

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
#endif
