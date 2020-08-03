#include <cstdint>
#include <cstdlib>

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

void arrayTypeSizes () {
    TEST_ASSERT_EQUAL(7 * sizeof (void*), sizeof (Array));
}

static void vecInstance () {
    Vector v (12);
    TEST_ASSERT_EQUAL(2, v.width()); // 12 bits in 2 bytes
    TEST_ASSERT_EQUAL(0, v.length());
}

static void vecSetGrow () {
    Vector v (32);
    TEST_ASSERT_EQUAL(4, v.width());

    v.set(0, 123);
    TEST_ASSERT_EQUAL(1, v.length());
    TEST_ASSERT_EQUAL(123, v.getInt(0));

    v.set(2, 456);
    TEST_ASSERT_EQUAL(3, v.length());
    TEST_ASSERT_EQUAL(123, v.getInt(0));
    TEST_ASSERT_EQUAL(0,   v.getInt(1));
    TEST_ASSERT_EQUAL(456, v.getInt(2));

    v.ins(1, 2);
    TEST_ASSERT_EQUAL(5, v.length());
    TEST_ASSERT_EQUAL(123, v.getInt(0));
    TEST_ASSERT_EQUAL(0,   v.getInt(1));
    TEST_ASSERT_EQUAL(0,   v.getInt(2));
    TEST_ASSERT_EQUAL(0,   v.getInt(3));
    TEST_ASSERT_EQUAL(456, v.getInt(4));

    v.ins(0);
    TEST_ASSERT_EQUAL(6, v.length());
    TEST_ASSERT_EQUAL(0,   v.getInt(0));
    TEST_ASSERT_EQUAL(123, v.getInt(1));
}

auto main () -> int {
    UNITY_BEGIN();

    RUN_TEST(smokeTest);
    RUN_TEST(arrayTypeSizes);
    RUN_TEST(vecInstance);
    RUN_TEST(vecSetGrow);

    UNITY_END();
    return 0;
}
