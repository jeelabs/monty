#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "xmonty.h"

#include <unity.h>

using namespace Monty;

uintptr_t memory [1024];
size_t memAvail;
VecOf<int> v;

void setUp () {
    setup(memory);
    memAvail = avail();

    TEST_ASSERT_EQUAL(0, v.cap());
    v.adj(25);

    for (int i = 0; i < 20; ++i)
        TEST_ASSERT_EQUAL(0, v[i]);

    TEST_ASSERT_EQUAL(0, v[10]);
    for (int i = 0; i < 10; ++i)
        v.begin()[i] = 11 * i;
}

void tearDown () {
    TEST_ASSERT_GREATER_THAN(0, v.cap());
    v.adj(0);
    TEST_ASSERT_EQUAL(0, v.cap());

    sweep();
    compact();
    TEST_ASSERT_EQUAL(memAvail, avail());
}

void smokeTest () {
    TEST_ASSERT_EQUAL(42, 40 + 2);
}

void vecTypeSizes () {
    TEST_ASSERT_EQUAL(2 * sizeof (void*), sizeof (Vec));
    TEST_ASSERT_EQUAL(3 * sizeof (void*), sizeof (VecOf<int>));
    TEST_ASSERT_EQUAL(3 * sizeof (void*), sizeof (VecOf<Vec>));
}

void vecOfInited () {
    TEST_ASSERT_GREATER_OR_EQUAL(25, v.cap());
    TEST_ASSERT_LESS_THAN(30, v.cap());

    for (int i = 0; i < 10; ++i)
        TEST_ASSERT_EQUAL(11 * i, v[i]);
    TEST_ASSERT_EQUAL(0, v[10]);
}

void vecOfMoveAndWipe () {
    v.move(2, 3, 4);

    static int m1 [] { 0, 11, 22, 33, 44, 55, 22, 33, 44, 99, 0, };
    for (int i = 0; i < 11; ++i)
        TEST_ASSERT_EQUAL(m1[i], v[i]);

    v.wipe(2, 3);

    static int m2 [] { 0, 11, 0, 0, 0, 55, 22, 33, 44, 99, 0, };
    for (int i = 0; i < 11; ++i)
        TEST_ASSERT_EQUAL(m2[i], v[i]);

    v.move(4, 3, -2);

    static int m3 [] { 0, 11, 0, 55, 22, 55, 22, 33, 44, 99, 0, };
    for (int i = 0; i < 11; ++i)
        TEST_ASSERT_EQUAL(m3[i], v[i]);
}

void vecCopyMove () {
    VecOf<int> v2;
    v2.adj(3);

    v2[0] = 100;
    v2[1] = 101;
    v2[2] = 102;

    TEST_ASSERT_GREATER_OR_EQUAL(3, v2.cap());
    TEST_ASSERT_LESS_THAN(8, v2.cap());

#if 0 // TODO
    //v2 = VecOf<int> {};
    //TEST_ASSERT_EQUAL(0, v2.cap());

    VecOf<int> v3 = VecOf<int> {};
    ...

    VecOf<int> v3 = v2;
    TEST_ASSERT_GREATER_OR_EQUAL(5, v3.cap());
    TEST_ASSERT_LESS_THAN(10, v3.cap());

    v3.adj(15);
#endif
}

void chunkTypeSizes () {
    TEST_ASSERT_EQUAL(3 * sizeof (void*), sizeof (CofV));
}

void chunkOfItems () {
#if 0
    ChunkOf<int> c (v);

    static int m1 [] { 0, 11, 22, 33, 44, 55, 66, 77, 88, 99, 0, };
    for (int i = 0; i < 11; ++i)
        TEST_ASSERT_EQUAL(m1[i], c[i]);

    auto n = c.length();
    TEST_ASSERT_GREATER_OR_EQUAL(25, n);
    TEST_ASSERT_LESS_THAN(30, n);

    c.off = 3;
    TEST_ASSERT_EQUAL(n - 3, c.length());

    for (int i = 3; i < 11; ++i)
        TEST_ASSERT_EQUAL(m1[i], c[i-3]);

    c.len = 5;
    TEST_ASSERT_EQUAL(5, c.length());

    c.len = 100;
    TEST_ASSERT_EQUAL(n - 3, c.length());
#endif
}

void chunkOfInsert () {
#if 0
    ChunkOf<int> c (v);

    auto n = v.cap();
    TEST_ASSERT_EQUAL(n, c.length());

    for (size_t i = 10; i < c.length(); ++i)
        c[i] = 100 + i;

    for (size_t i = 0; i < 10; ++i) {
        TEST_ASSERT_EQUAL(11 * i, v[i]);
        TEST_ASSERT_EQUAL(11 * i, c[i]);
    }
    for (size_t i = 10; i < n; ++i)
        TEST_ASSERT_EQUAL(100 + i, v[i]);

    c.off = 20;
    c.len = 3;
    for (size_t i = 0; i < c.length(); ++i)
        TEST_ASSERT_EQUAL(120 + i, c[i]);

    c.insert(1, 2);
    TEST_ASSERT_EQUAL(n, v.cap());
    TEST_ASSERT_EQUAL(5, c.length());

    static int m1 [] { 120, 0, 0, 121, 122, };
    for (auto& e : m1) {
        auto i = &e - m1;
        TEST_ASSERT_EQUAL(e, v[i+20]);
        TEST_ASSERT_EQUAL(e, c[i]);
    }

    c.insert(4, 5);
    TEST_ASSERT_EQUAL(10, c.length());
    TEST_ASSERT_EQUAL(10, c.len);
    TEST_ASSERT_GREATER_THAN(n, v.cap());
    TEST_ASSERT_GREATER_OR_EQUAL(c.off + c.len, v.cap());

    static int m2 [] { 120, 0, 0, 121, 0, 0, 0, 0, 0, 122, };
    for (auto& e : m2) {
        auto i = &e - m2;
        TEST_ASSERT_EQUAL(e, v[i+20]);
        TEST_ASSERT_EQUAL(e, c[i]);
    }

    Vec v2;
    CofV cov {v2};
    cov.insert(0, 10);
    TEST_ASSERT_EQUAL(10, cov.length());

    for (auto e : cov)
        TEST_ASSERT(e.isNil());
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
}

int main () {
    UNITY_BEGIN();

    RUN_TEST(smokeTest);
    RUN_TEST(vecTypeSizes);
    RUN_TEST(vecOfInited);
    RUN_TEST(vecOfMoveAndWipe);
    RUN_TEST(vecCopyMove);

    RUN_TEST(chunkTypeSizes);
    RUN_TEST(chunkOfItems);
    RUN_TEST(chunkOfInsert);

    RUN_TEST(vecOfVal);

    UNITY_END();
}
