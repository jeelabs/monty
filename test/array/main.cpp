#include "monty.h"
#include <unity.h>

#ifndef UNITY_SUPPORT_64
#undef TEST_ASSERT_EQUAL_INT64
#define TEST_ASSERT_EQUAL_INT64(x,y) TEST_ASSERT((int64_t) (x) == (int64_t) (y))
#endif

using namespace monty;

uint8_t memory [3*1024];
uint32_t memAvail;
VecOf<int> v;

void setUp () {
    gcSetup(memory, sizeof memory);
    memAvail = gcAvail();

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
    TEST_ASSERT_EQUAL(memAvail, gcAvail());
}

void smokeTest () {
    TEST_ASSERT_EQUAL(42, 40 + 2);
}

void vecOfTypeSizes () {
    TEST_ASSERT_EQUAL(2 * sizeof (void*), sizeof (Vec));
    TEST_ASSERT_EQUAL(1 * sizeof (void*) + 8, sizeof (VecOf<int>));
    TEST_ASSERT_EQUAL(1 * sizeof (void*) + 8, sizeof (VecOf<Vec>));
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
}

void arrayTypeSizes () {
    TEST_ASSERT_EQUAL(2 * sizeof (void*) + 8, sizeof (Array));
    TEST_ASSERT_EQUAL(2 * sizeof (void*) + 8, sizeof (List));
    TEST_ASSERT_EQUAL(2 * sizeof (void*) + 8, sizeof (Set));
    TEST_ASSERT_EQUAL(3 * sizeof (void*) + 8, sizeof (Dict));
    TEST_ASSERT_EQUAL(5 * sizeof (void*) + 8, sizeof (Type));
    TEST_ASSERT_EQUAL(5 * sizeof (void*) + 8, sizeof (Class));
    TEST_ASSERT_EQUAL(3 * sizeof (void*) + 8, sizeof (Inst));
}

static void arrayInsDel () {
    static struct { char typ; int64_t min, max; int log; } tests [] = {
    //    typ                     min  max                log
        { 'P',                      0, 1,                   0 },
        { 'T',                      0, 3,                   1 },
        { 'N',                      0, 15,                  2 },
        { 'b',                   -128, 127,                 3 },
        { 'B',                      0, 255,                 3 },
        { 'h',                 -32768, 32767,               4 },
        { 'H',                      0, 65535,               4 },
        { 'i',                 -32768, 32767,               4 },
        { 'I',                      0, 65535,               4 },
        { 'l',            -2147483648, 2147483647,          5 },
        { 'L',                      0, 4294967295,          5 },
        { 'q', -9223372036854775807-1, 9223372036854775807, 6 },
    };
    for (auto e : tests) {
        //printf("e %c min %lld max %lld log %d\n", e.typ, e.min, e.max, e.log);

        Array a (e.typ);
        TEST_ASSERT_EQUAL(0, a.len());

        constexpr auto N = 24;
        a.insert(0, N);
        TEST_ASSERT_EQUAL(N, a.len());

        int bytes = ((N << e.log) + 7) >> 3;
        TEST_ASSERT_GREATER_OR_EQUAL(bytes, a.cap());
        TEST_ASSERT_LESS_OR_EQUAL(bytes + 2 * sizeof (void*), a.cap());

        for (uint32_t i = 0; i < a.len(); ++i)
            TEST_ASSERT_EQUAL(0, (int) a.getAt(i));

        a.setAt(0, Int::make(e.min));
        a.setAt(1, Int::make(e.min - 1));
        a.setAt(N-2, Int::make(e.max + 1));
        a.setAt(N-1, Int::make(e.max));

        TEST_ASSERT_EQUAL_INT64(e.min, a.getAt(0).asInt());
        TEST_ASSERT_EQUAL_INT64(e.max, a.getAt(N-1).asInt());

        // can't test for overflow using int64_t when storing ± 63-bit ints
        if (e.max != 9223372036854775807) {
            // TODO not present in pio's version of Unity ...
            //TEST_ASSERT_NOT_EQUAL_INT64(e.min - 1, a.getAt(1));
            //TEST_ASSERT_NOT_EQUAL_INT64(e.max + 1, a.getAt(N-2));
            TEST_ASSERT(e.min - 1 != a.getAt(1).asInt());
            TEST_ASSERT(e.max + 1 != a.getAt(N-2).asInt());
        }

        a.remove(8, 8);
        TEST_ASSERT_EQUAL(N-8, a.len());
        TEST_ASSERT_EQUAL_INT64(e.min, a.getAt(0).asInt());
        TEST_ASSERT_EQUAL_INT64(e.max, a.getAt(N-8-1).asInt());

        a.insert(0, 8);
        TEST_ASSERT_EQUAL(N, a.len());
        TEST_ASSERT_EQUAL(0, (int) a.getAt(0));
        TEST_ASSERT_EQUAL(0, (int) a.getAt(7));
        TEST_ASSERT_EQUAL_INT64(e.min, a.getAt(8).asInt());
        TEST_ASSERT_EQUAL_INT64(e.max, a.getAt(N-1).asInt());
    }
}

static void listInsDel () {
    List l;
    TEST_ASSERT_EQUAL(0, l.size());

    l.insert(0, 5);
    TEST_ASSERT_EQUAL(5, l.size());

    for (auto e : l)
        TEST_ASSERT(e.isNil());

    for (uint32_t i = 0; i < 5; ++i)
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
    for (uint32_t i = 0; i < 2 * d.size(); ++i)
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
    RUN_TEST(arrayInsDel);
    RUN_TEST(listInsDel);
    RUN_TEST(setInsDel);
    RUN_TEST(dictInsDel);

    UNITY_END();
}
