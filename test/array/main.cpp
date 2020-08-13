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

void vecOfTypeSizes () {
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

void vecOfCopyMove () {
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

void arrayTypeSizes () {
    TEST_ASSERT_EQUAL(2 * sizeof (void*), sizeof (Tuple));
    TEST_ASSERT_EQUAL(4 * sizeof (void*), sizeof (List));
    TEST_ASSERT_EQUAL(4 * sizeof (void*), sizeof (Set));
    TEST_ASSERT_EQUAL(5 * sizeof (void*), sizeof (Dict));
    TEST_ASSERT_EQUAL(7 * sizeof (void*), sizeof (Type));
    TEST_ASSERT_EQUAL(7 * sizeof (void*), sizeof (Class));
    TEST_ASSERT_EQUAL(5 * sizeof (void*), sizeof (Inst));
}

static void listInsDel () {
    List l;
    TEST_ASSERT_EQUAL(0, l.size());

    l.insert(0, 5);
    TEST_ASSERT_EQUAL(5, l.size());

    for (auto e : l)
        TEST_ASSERT(e.isNil());

    for (size_t i = 0; i < 5; ++i)
        l[i] = 10 + i;

    for (auto& e : l) {
        auto i = &e - &l[0];
        TEST_ASSERT_EQUAL(10 + i, e);
    }

    l.insert(2, 3);
    TEST_ASSERT_EQUAL(8, l.size());

    static int m1 [] { 10, 11, 0, 0, 0, 12, 13, 14, };
    for (auto& e : m1) {
        auto i = &e - m1;
        TEST_ASSERT_EQUAL(e, l[i]);
    }

    l.remove(1, 5);
    TEST_ASSERT_EQUAL(3, l.size());

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
    TEST_ASSERT_EQUAL(0, s.size());

    for (int i = 20; i < 25; ++i)
        s.has(i) = true;
    TEST_ASSERT_EQUAL(5, s.size());     // 20 21 22 23 24

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
    TEST_ASSERT_EQUAL(3, s.size());     // 20 21 22

    TEST_ASSERT_FALSE(s.has(19));
    TEST_ASSERT_TRUE(s.has(20));
    TEST_ASSERT_TRUE(s.has(22));
    TEST_ASSERT_FALSE(s.has(23));

    for (int i = 20; i < 23; ++i)
        TEST_ASSERT_TRUE(s.has(i));

    for (int i = 19; i < 22; ++i)
        s.has(i) = true;
    TEST_ASSERT_EQUAL(4, s.size());     // 19 20 21 22

    TEST_ASSERT_FALSE(s.has(18));
    TEST_ASSERT_TRUE(s.has(19));
    TEST_ASSERT_TRUE(s.has(22));
    TEST_ASSERT_FALSE(s.has(23));

    for (int i = 19; i < 23; ++i)
        TEST_ASSERT_TRUE(s.has(i));

    s.has("abc") = true;
    s.has("def") = true;
    TEST_ASSERT_EQUAL(6, s.size());     // 19 20 21 22 "abc" "def"

    TEST_ASSERT_FALSE(s.has(""));
    TEST_ASSERT_TRUE(s.has("abc"));
    TEST_ASSERT_TRUE(s.has("def"));
    TEST_ASSERT_FALSE(s.has("ghi"));

    s.has("abc") = true; // no effect
    s.has("ghi") = false; // no effect
    TEST_ASSERT_EQUAL(6, s.size());     // 19 20 21 22 "abc" "def"

    s.has("def") = false;
    TEST_ASSERT_EQUAL(5, s.size());     // 19 20 21 22 "abc"
    TEST_ASSERT_TRUE(s.has("abc"));
    TEST_ASSERT_FALSE(s.has("def"));
    TEST_ASSERT_FALSE(s.has("ghi"));
}

static void dictInsDel () {
    Dict d;
    TEST_ASSERT_EQUAL(0, d.size());

    for (int i = 0; i < 5; ++i)
        d.at(10+i) = 30+i;
    TEST_ASSERT_EQUAL(5, d.size());     // 10:30 11:31 12:32 13:33 14:34

    TEST_ASSERT_TRUE(d.has(12));
    TEST_ASSERT_FALSE(d.has(15));

    Value v;
    v = d.at(12);
    TEST_ASSERT(!v.isNil() && v.isInt()); // exists
    v = d.at(15);
    TEST_ASSERT(v.isNil()); // doesn't exist

    for (int i = 0; i < 5; ++i) {
        Value e = d.at(10+i);
        TEST_ASSERT_EQUAL(30+i, e);
    }

    d.at(15) = 35;
    TEST_ASSERT_EQUAL(6, d.size());     // 10:30 11:31 12:32 13:33 14:34 15:35
    TEST_ASSERT_TRUE(d.has(15));

    for (int i = 0; i < 6; ++i) {
        Value e = d.at(10+i);
        TEST_ASSERT_EQUAL(30+i, e);
    }

    d.at(12) = 42;
    TEST_ASSERT_EQUAL(6, d.size());     // 10:30 11:31 12:42 13:33 14:34 15:35
    TEST_ASSERT_TRUE(d.has(12));
    TEST_ASSERT_EQUAL(42, (Value) d.at(12));

    d.at(11) = Value {};
    TEST_ASSERT_EQUAL(5, d.size());     // 10:30 12:42 13:33 14:34 15:35
    TEST_ASSERT_FALSE(d.has(11));

    static int m1 [] { 1030, 1242, 1333, 1434, 1535, };
    for (auto e : m1) {
        int k = e / 100, v = e % 100;
        TEST_ASSERT_EQUAL(v, (Value) d.at(k));
    }

#if 0
    auto p = d.begin(); // a sneaky way to access the underlying VecOf<Value>
    for (size_t i = 0; i < 2 * d.size(); ++i)
        printf("%d, ", (int) p[i]);
    printf("\n");
#endif
}

int main () {
    UNITY_BEGIN();

    RUN_TEST(smokeTest);

    RUN_TEST(vecOfTypeSizes);
    RUN_TEST(vecOfInited);
    RUN_TEST(vecOfMoveAndWipe);
    RUN_TEST(vecOfCopyMove);

    RUN_TEST(arrayTypeSizes);
    RUN_TEST(listInsDel);
    RUN_TEST(setInsDel);
    RUN_TEST(dictInsDel);

    UNITY_END();
}
