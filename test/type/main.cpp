#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "xmonty.h"

#include <unity.h>

using namespace Monty;

uintptr_t memory [1024];
size_t memAvail;

void setUp () {
    setup(memory, sizeof memory);
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
    TEST_ASSERT_EQUAL(sizeof (void*), sizeof (Val));
    TEST_ASSERT_EQUAL(sizeof (void*), sizeof (Object));
    TEST_ASSERT_EQUAL(sizeof (void*), sizeof (None));
    TEST_ASSERT_EQUAL(sizeof (void*), sizeof (Bool));

    //struct LongAlign { void* p; int64_t i; };
    //TEST_ASSERT_EQUAL(sizeof (LongAlign), sizeof (Long));

    // on 32b arch, packed = 12 bytes, which will fit in 2 GC slots i.s.o. 3
    // on 64b arch, packed is the same as unpacked as void* is also 8 bytes
    struct LongPacked { void* p; int64_t i; } __attribute__((packed));
    TEST_ASSERT_EQUAL(sizeof (LongPacked), sizeof (Long));

    TEST_ASSERT_EQUAL(3 * sizeof (void*), sizeof (Type)); // TODO will change
}

auto main () -> int {
    UNITY_BEGIN();

    RUN_TEST(smokeTest);
    RUN_TEST(objTypeSizes);

    UNITY_END();
    return 0;
}
