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

void arrayTypeSizes () {
    TEST_ASSERT_EQUAL(7 * sizeof (void*), sizeof (Array));
    TEST_ASSERT_EQUAL(7 * sizeof (void*), sizeof (Tuple));
    TEST_ASSERT_EQUAL(6 * sizeof (void*), sizeof (List));
    TEST_ASSERT_EQUAL(6 * sizeof (void*), sizeof (Set));
    TEST_ASSERT_EQUAL(7 * sizeof (void*), sizeof (Dict));
    TEST_ASSERT_EQUAL(9 * sizeof (void*), sizeof (Type));
    TEST_ASSERT_EQUAL(9 * sizeof (void*), sizeof (Class));
    TEST_ASSERT_EQUAL(7 * sizeof (void*), sizeof (Instance));
}

static void listInsDel () {
    List v;
    TEST_ASSERT_EQUAL(0, v.len());

    v.ins(0, 5);
    TEST_ASSERT_EQUAL(5, v.len());

    for (auto e : v)
        TEST_ASSERT(e.isNil());

    for (size_t i = 0; i < 5; ++i)
        v[i] = 10 + i;

    for (auto& e : v) {
        auto i = &e - &v[0];
        TEST_ASSERT_EQUAL(10 + i, e);
    }

    v.ins(2, 3);
    TEST_ASSERT_EQUAL(8, v.len());

    static int m1 [] { 10, 11, 0, 0, 0, 12, 13, 14, };
    for (auto& e : m1) {
        auto i = &e - m1;
        TEST_ASSERT_EQUAL(e, v[i]);
    }

    v.del(1, 5);
    TEST_ASSERT_EQUAL(3, v.len());

    static int m2 [] { 10, 13, 14, };
    for (auto& e : m2) {
        auto i = &e - m2;
        TEST_ASSERT_EQUAL(e, v[i]);
    }

    for (auto& e : v) {
        auto i = &e - &v[0];
        TEST_ASSERT_EQUAL(m2[i], e);
    }
}

static void setInsDel () {
    Set v;
    TEST_ASSERT_EQUAL(0, v.len());

    for (int i = 20; i < 25; ++i)   // 20 21 22 23 24
        v.has(i) = true;
    TEST_ASSERT_EQUAL(5, v.len());

    TEST_ASSERT_FALSE(v.has(19));
    TEST_ASSERT_TRUE(v.has(20));
    TEST_ASSERT_TRUE(v.has(24));
    TEST_ASSERT_FALSE(v.has(25));

    for (int i = 20; i < 25; ++i)
        TEST_ASSERT_TRUE(v.has(i));

    for (auto& e : v)
        TEST_ASSERT_TRUE(e); // every item in v is in v, doh!

    for (int i = 23; i < 28; ++i)   // 20 21 22
        v.has(i) = false;
    TEST_ASSERT_EQUAL(3, v.len());

    TEST_ASSERT_FALSE(v.has(19));
    TEST_ASSERT_TRUE(v.has(20));
    TEST_ASSERT_TRUE(v.has(22));
    TEST_ASSERT_FALSE(v.has(23));

    for (int i = 20; i < 23; ++i)
        TEST_ASSERT_TRUE(v.has(i));

    for (int i = 19; i < 22; ++i)   // 19 20 21 22
        v.has(i) = true;
    TEST_ASSERT_EQUAL(4, v.len());

    TEST_ASSERT_FALSE(v.has(18));
    TEST_ASSERT_TRUE(v.has(19));
    TEST_ASSERT_TRUE(v.has(22));
    TEST_ASSERT_FALSE(v.has(23));

    for (int i = 19; i < 23; ++i)
        TEST_ASSERT_TRUE(v.has(i));

    v.has("abc") = true;
    v.has("def") = true;
    TEST_ASSERT_EQUAL(6, v.len());

    TEST_ASSERT_FALSE(v.has(""));
    TEST_ASSERT_TRUE(v.has("abc"));
    TEST_ASSERT_TRUE(v.has("def"));
    TEST_ASSERT_FALSE(v.has("x"));

    v.has("def") = false;
    TEST_ASSERT_EQUAL(5, v.len());
    TEST_ASSERT_TRUE(v.has("abc"));
    TEST_ASSERT_FALSE(v.has("def"));
}

int main () {
    UNITY_BEGIN();

    RUN_TEST(smokeTest);
    RUN_TEST(arrayTypeSizes);
    RUN_TEST(listInsDel);
    RUN_TEST(setInsDel);

    UNITY_END();
}
