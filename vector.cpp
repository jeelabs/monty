// Implementation of a mutable vector class, with "VecOf" using type templates.

#include <assert.h>
#include <string.h>

#include "monty.h"

#if NATIVE
#include <stdio.h>
#else
#include <jee.h>
#endif

extern void initBoard ();

Vector::Vector (size_t bits) {
    while (bits > (1U << logBits))
        ++logBits;
initBoard(); // early init needed, before main() runs!
//printf(":  Vector %p\n", this);
checkVecs();
}

Vector::~Vector () {
checkVecs();
assert(capacity < 500);
assert(data == 0 || data->v == this);
//printf(": ~Vector %p\n", this);
    assert(capacity > 0 || data == 0);
    alloc(0);
}

int Vector::getInt (int idx) const {
checkVecs();
    auto p = getPtr(idx);
assert(capacity < 500);
assert(data == 0 || data->v == this);
    switch (logBits) {
        case 3: return *(int8_t*) p;
        case 4: return *(int16_t*) p;
        case 5: return *(int32_t*) p;
    }
    assert(false); // TODO
    return 0;
}

uint32_t Vector::getIntU (int idx) const {
checkVecs();
    auto p = getPtr(idx);
assert(capacity < 500);
assert(data == 0 || data->v == this);
    switch (logBits) {
        case 3: return *(uint8_t*) p;
        case 4: return *(uint16_t*) p;
        case 5: return *(uint32_t*) p;
    }
    assert(false); // TODO
    return 0;
}

void* Vector::getPtr (int idx) const {
checkVecs();
    assert(logBits >= 3); // TODO
assert(capacity < 500);
assert(data == 0 || data->v == this);
    if (idx < 0)
        idx += fill;
    assert(data != 0);
    return data->d + idx * width();
}

void Vector::set (int idx, int val) {
checkVecs();
    assert(1 <= width() && width() <= (int) sizeof val); // TODO
assert(capacity < 500);
assert(data == 0 || data->v == this);
    set(idx, &val); // TODO assumes little-endian byte order
}

void Vector::set (int idx, const void* ptr) {
checkVecs();
    assert(logBits >= 3); // TODO
assert(capacity < 500);
assert(data == 0 || data->v == this);
    if (idx * width() >= (int) capacity) {
        auto n = capacity / width();
        ins(n, idx + 1 - n);
    }
    memcpy(getPtr(idx), ptr, width());
}

void Vector::ins (int idx, int num) {
checkVecs();
    assert(logBits >= 3); // TODO
//printf("ins %d idx %d cap %d\n", idx, num, capacity);
assert(capacity < 500);
assert(data == 0 || data->v == this);
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
checkVecs();
    assert(logBits >= 3); // TODO
assert(capacity < 500);
assert(data == 0 || data->v == this);
    if (num <= 0)
        return;
    fill -= num;
    memmove(getPtr(idx), getPtr(idx + num), (fill - idx) * width());
}
