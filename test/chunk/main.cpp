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

#include <cstdio>
void vecOfMoveAndWipe () {
    VecOf<int> v;
    v.resize(100);

    TEST_ASSERT_GREATER_OR_EQUAL(100 / sizeof (int), v.cap());
    TEST_ASSERT_LESS_THAN(120 / sizeof (int), v.cap());

    auto p = v.ptr();
    for (int i = 0; i < 10; ++i)
        p[i] = 11 * i;

    for (int i = 0; i < 10; ++i)
        TEST_ASSERT_EQUAL(11 * i, v[i]);
    TEST_ASSERT_EQUAL(0, v[10]);

    v.move(2, 3, 4);

    static int m1 [] = { 0, 11, 22, 33, 44, 55, 22, 33, 44, 99, 0, };
    for (int i = 0; i < 11; ++i)
        TEST_ASSERT_EQUAL(m1[i], v[i]);

    v.wipe(2, 3);

    static int m2 [] = { 0, 11, 0, 0, 0, 55, 22, 33, 44, 99, 0, };
    for (int i = 0; i < 11; ++i)
        TEST_ASSERT_EQUAL(m2[i], v[i]);

    v.move(4, 3, -2);

    static int m3 [] = { 0, 11, 0, 55, 22, 55, 22, 33, 44, 99, 0, };
    for (int i = 0; i < 11; ++i)
        TEST_ASSERT_EQUAL(m3[i], v[i]);

#if 0
    for (int i = 0; i < 11; ++i)
        printf("%d, ", v[i]);
    printf("\n");
#endif
}

static void vecOfVal () {
#if 0
    VecOf<Val> v;
    TEST_ASSERT_EQUAL(sizeof (Val), v.width());

    v.set(0, 123);
    TEST_ASSERT_EQUAL(1, v.length());
    TEST_ASSERT_EQUAL_INT(123, v.get(0));

    v.set(2, 456);
    TEST_ASSERT_EQUAL(3, v.length());
    TEST_ASSERT_EQUAL_INT(123, v.get(0));
    TEST_ASSERT_EQUAL_INT(0,   v.get(1));
    TEST_ASSERT_EQUAL_INT(456, v.get(2));

    v.ins(1, 2);
    TEST_ASSERT_EQUAL(5, v.length());
    TEST_ASSERT_EQUAL_INT(123, v.get(0));
    TEST_ASSERT_EQUAL_INT(0,   v.get(1));
    TEST_ASSERT_EQUAL_INT(0,   v.get(2));
    TEST_ASSERT_EQUAL_INT(0,   v.get(3));
    TEST_ASSERT_EQUAL_INT(456, v.get(4));

    v.ins(0);
    TEST_ASSERT_EQUAL(6, v.length());
    TEST_ASSERT_EQUAL_INT(0,   v.get(0));
    TEST_ASSERT_EQUAL_INT(123, v.get(1));
#endif
    auto a = Array::create('B');
    TEST_ASSERT_NOT_NULL(a);
}

auto main () -> int {
    UNITY_BEGIN();

    RUN_TEST(smokeTest);
    RUN_TEST(vecOfMoveAndWipe);
    RUN_TEST(vecOfVal);

    UNITY_END();
    return 0;
}
