// Implementation of a mutable vector class, with "VecOf" using type templates.

#include <assert.h>
#include <string.h>

#include "monty.h"

extern void initBoard ();

Vector::Vector (size_t bits) {
    while (bits > (1U << logBits))
        ++logBits;
    assert(logBits >= 3); // TODO
}

Vector::~Vector () {
    alloc(0);
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
    auto p = getPtr(idx);
    switch (logBits) {
        case 3: return *(uint8_t*) p;
        case 4: return *(uint16_t*) p;
        case 5: return *(uint32_t*) p;
    }
    return 0;
}

void* Vector::getPtr (int idx) const {
    if (idx < 0)
        idx += fill;
    assert(data != 0);
    return data->d + idx * width();
}

void Vector::set (int idx, int val) {
    assert(1 <= width() && width() <= (int) sizeof val); // TODO
    set(idx, &val); // TODO assumes little-endian byte order
}

void Vector::set (int idx, const void* ptr) {
    if (idx * width() >= (int) capacity) {
        auto n = capacity / width();
        ins(n, idx + 1 - n);
    }
    memcpy(getPtr(idx), ptr, width());
}

void Vector::ins (int idx, int num) {
    if (num <= 0)
        return;
    auto needed = (fill + num) * width();
    if (needed > (int) capacity) {
        alloc(needed);
        assert(data != 0);
        capacity = needed;
    }
    auto p = getPtr(idx);
    memmove(getPtr(idx + num), p, (fill - idx) * width());
    memset(p, 0, num * width());
    fill += num;
}

void Vector::del (int idx, int num) {
    if (num <= 0)
        return;
    fill -= num;
    memmove(getPtr(idx), getPtr(idx + num), (fill - idx) * width());
}
