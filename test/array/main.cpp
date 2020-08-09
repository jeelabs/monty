#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "xmonty.h"

#include <unity.h>
#include <cstdio>

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
    List l;
    TEST_ASSERT_EQUAL(0, l.len());

    l.ins(0, 5);
    TEST_ASSERT_EQUAL(5, l.len());

    for (auto e : l)
        TEST_ASSERT(e.isNil());

    for (size_t i = 0; i < 5; ++i)
        l[i] = 10 + i;

    for (auto& e : l) {
        auto i = &e - &l[0];
        TEST_ASSERT_EQUAL(10 + i, e);
    }

    l.ins(2, 3);
    TEST_ASSERT_EQUAL(8, l.len());

    static int m1 [] { 10, 11, 0, 0, 0, 12, 13, 14, };
    for (auto& e : m1) {
        auto i = &e - m1;
        TEST_ASSERT_EQUAL(e, l[i]);
    }

    l.del(1, 5);
    TEST_ASSERT_EQUAL(3, l.len());

    static int m2 [] { 10, 13, 14, };
    for (auto& e : m2) {
        auto i = &e - m2;
        TEST_ASSERT_EQUAL(e, l[i]);
    }

    for (auto& e : l) {
        auto i = &e - &l[0];
        TEST_ASSERT_EQUAL(m2[i], e);
    }
}

static void setInsDel () {
    Set s;
    TEST_ASSERT_EQUAL(0, s.len());

    for (int i = 20; i < 25; ++i)
        s.has(i) = true;
    TEST_ASSERT_EQUAL(5, s.len());      // 20 21 22 23 24

    TEST_ASSERT_FALSE(s.has(19));
    TEST_ASSERT_TRUE(s.has(20));
    TEST_ASSERT_TRUE(s.has(24));
    TEST_ASSERT_FALSE(s.has(25));

    for (int i = 20; i < 25; ++i)
        TEST_ASSERT_TRUE(s.has(i));

    for (auto e : s)
        TEST_ASSERT(20 <= (int) e && (int) e < 25);

    for (int i = 23; i < 28; ++i)
        s.has(i) = false;
    TEST_ASSERT_EQUAL(3, s.len());      // 20 21 22

    TEST_ASSERT_FALSE(s.has(19));
    TEST_ASSERT_TRUE(s.has(20));
    TEST_ASSERT_TRUE(s.has(22));
    TEST_ASSERT_FALSE(s.has(23));

    for (int i = 20; i < 23; ++i)
        TEST_ASSERT_TRUE(s.has(i));

    for (int i = 19; i < 22; ++i)
        s.has(i) = true;
    TEST_ASSERT_EQUAL(4, s.len());      // 19 20 21 22

    TEST_ASSERT_FALSE(s.has(18));
    TEST_ASSERT_TRUE(s.has(19));
    TEST_ASSERT_TRUE(s.has(22));
    TEST_ASSERT_FALSE(s.has(23));

    for (int i = 19; i < 23; ++i)
        TEST_ASSERT_TRUE(s.has(i));

    s.has("abc") = true;
    s.has("def") = true;
    TEST_ASSERT_EQUAL(6, s.len());      // 19 20 21 22 "abc" "def"

    TEST_ASSERT_FALSE(s.has(""));
    TEST_ASSERT_TRUE(s.has("abc"));
    TEST_ASSERT_TRUE(s.has("def"));
    TEST_ASSERT_FALSE(s.has("ghi"));

    s.has("abc") = true; // no effect
    s.has("ghi") = false; // no effect
    TEST_ASSERT_EQUAL(6, s.len());      // 19 20 21 22 "abc" "def"

    s.has("def") = false;
    TEST_ASSERT_EQUAL(5, s.len());      // 19 20 21 22 "abc"
    TEST_ASSERT_TRUE(s.has("abc"));
    TEST_ASSERT_FALSE(s.has("def"));
    TEST_ASSERT_FALSE(s.has("ghi"));
}

static void dictInsDel () {
    Dict d;
    TEST_ASSERT_EQUAL(0, d.len());

    for (int i = 0; i < 5; ++i)
        d.at(10+i) = 30+i;
    TEST_ASSERT_EQUAL(5, d.len());      // 10:30 11:31 12:32 13:33 14:34
    TEST_ASSERT_FALSE(d.has(15));

    for (int i = 0; i < 5; ++i) {
        Value e = d.at(10+i);
        TEST_ASSERT_EQUAL(30+i, e);
    }

    d.at(15) = 35;
    TEST_ASSERT_EQUAL(6, d.len());      // 10:30 11:31 12:32 13:33 14:34 15:35
    TEST_ASSERT_TRUE(d.has(15));

    for (int i = 0; i < 6; ++i) {
        Value e = d.at(10+i);
        TEST_ASSERT_EQUAL(30+i, e);
    }

    d.at(12) = 42;
    TEST_ASSERT_EQUAL(6, d.len());      // 10:30 11:31 12:42 13:33 14:34 15:35
    TEST_ASSERT_TRUE(d.has(12));
    TEST_ASSERT_EQUAL(42, (Value) d.at(12));

    d.at(11) = Value {};
    TEST_ASSERT_EQUAL(5, d.len());      // 10:30 12:42 13:33 14:34 15:35
    TEST_ASSERT_FALSE(d.has(11));

    static int m1 [] { 1030, 1242, 1333, 1434, 1535, };
    for (auto e : m1) {
        int k = e / 100, v = e % 100;
        TEST_ASSERT_EQUAL(v, (Value) d.at(k));
    }

#if 0
    auto p = d.begin(); // a sneaky way to access the underlying VecOf<Value>
    for (size_t i = 0; i < 2 * d.len(); ++i)
        printf("%d, ", (int) p[i]);
    printf("\n");
#endif
}

int main () {
    UNITY_BEGIN();

    RUN_TEST(smokeTest);
    RUN_TEST(arrayTypeSizes);
    RUN_TEST(listInsDel);
    RUN_TEST(setInsDel);
    RUN_TEST(dictInsDel);

    UNITY_END();
}
