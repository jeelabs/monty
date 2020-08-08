#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "xmonty.h"

#include <unity.h>

using namespace Monty;

uintptr_t memory [1024];
size_t memAvail;

void setUp () {
    setup(memory);
    memAvail = avail();
}

void tearDown () {
    sweep();
    compact();
    TEST_ASSERT_EQUAL(memAvail, avail());
}

void smokeTest () {
    TEST_ASSERT_EQUAL(42, 40 + 2);
}

void objTypeSizes () {
    TEST_ASSERT_EQUAL(sizeof (void*), sizeof (Value));
    TEST_ASSERT_EQUAL(sizeof (void*), sizeof (Object));
    TEST_ASSERT_EQUAL(sizeof (void*), sizeof (None));
    TEST_ASSERT_EQUAL(sizeof (void*), sizeof (Bool));

    // on 32b arch, packed = 12 bytes, which will fit in 2 GC slots i.s.o. 3
    // on 64b arch, packed is the same as unpacked as void* is also 8 bytes
    struct Packed { void* p; int64_t i; } __attribute__((packed));
    struct Normal { void* p; int64_t i; };
    TEST_ASSERT(sizeof (Packed) < sizeof (Normal) || 8 * sizeof (void*) == 64);

    TEST_ASSERT_GREATER_OR_EQUAL(sizeof (Packed), sizeof (Fixed));
    TEST_ASSERT_LESS_OR_EQUAL(sizeof (Normal), sizeof (Fixed));

    TEST_ASSERT_EQUAL(3 * sizeof (void*), sizeof (Type)); // TODO will change
}

int main () {
    UNITY_BEGIN();

    RUN_TEST(smokeTest);
    RUN_TEST(objTypeSizes);

    UNITY_END();
}
