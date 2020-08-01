// Implementation of a mutable vector class, with "VecOf" using type templates.

#include <assert.h>
#include <string.h>

#include "monty.h"

Vector::Vector (size_t bits) : logBits (0), capacity (0) {
    while (bits > (1U << logBits))
        ++logBits;
}

int Vector::getInt (int idx) const {
    auto p = getPtr(idx);
    switch (logBits) {
        case 3: return *(int8_t*) p;
        case 4: return *(int16_t*) p;
        case 5: return *(int32_t*) p;
    }
    return 0;
}

uint32_t Vector::getIntU (int idx) const {
    if (idx < 0)
        idx += fill;
    auto p = (uint8_t*) getPtr(idx);
    switch (logBits) {
        case 0: return (*p >> (idx&7)) & 0x1;
        case 1: return (*p >> 2*(idx&3)) & 0x3;
        case 2: return (*p >> 4*(idx&1)) & 0xF;
        case 3: return *p;
        case 4: return *(uint16_t*) p;
        case 5: return *(uint32_t*) p;
    }
    return 0;
}

void* Vector::getPtr (int idx) const {
    if (idx < 0)
        idx += fill;
    return data != 0 ? data->d + widthOf(idx) : 0;
}

void Vector::set (int idx, int val) {
    assert(1U << logBits <= 8 * sizeof val);
    set(idx, &val); // TODO assumes little-endian byte order
}

void Vector::set (int idx, const void* ptr) {
    if (idx < 0)
        idx += fill;
    if (widthOf(idx) >= (int) capacity) {
        auto n = (capacity << 3) >> logBits;
        ins(n, idx + 1 - n);
    }
    auto p = (uint8_t*) getPtr(idx);
    switch (logBits) {
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

void Vector::ins (int idx, int num) {
    if (num <= 0)
        return;
    auto needed = widthOf(fill + num);
    if (needed > capacity) {
        alloc(needed);
        assert(data != 0);
        capacity = needed;
    }
    auto p = getPtr(idx);
    if (logBits >= 3) // TODO
        memmove(getPtr(idx + num), p, widthOf(fill - idx));
    memset(p, 0, widthOf(num));
    fill += num;
}

void Vector::del (int idx, int num) {
    if (num <= 0)
        return;
    fill -= num;
    if (logBits >= 3) // TODO
        memmove(getPtr(idx), getPtr(idx + num), widthOf(fill - idx));
}
