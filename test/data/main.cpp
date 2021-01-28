#include "monty.h"
#include <unity.h>

#ifndef UNITY_SUPPORT_64
#undef TEST_ASSERT_EQUAL_INT64
#define TEST_ASSERT_EQUAL_INT64(x,y) TEST_ASSERT((int64_t) (x) == (int64_t) (y))
#endif

using namespace monty;

uint8_t memory [3*1024];
uint32_t memAvail;

void setUp () {
    gcSetup(memory, sizeof memory);
    memAvail = gcAvail();
}

void tearDown () {
    sweep();
    compact();
    TEST_ASSERT_EQUAL(memAvail, gcAvail());
}

void smokeTest () {
    TEST_ASSERT_EQUAL(42, 40 + 2);
}

void dataSizes () {
    //TEST_ASSERT_EQUAL(sizeof (ByteVec), sizeof (VaryVec));
    TEST_ASSERT_EQUAL(sizeof (void*), sizeof (Value));
    TEST_ASSERT_EQUAL(sizeof (void*), sizeof (Object));
    TEST_ASSERT_EQUAL(sizeof (void*), sizeof (None));
    TEST_ASSERT_EQUAL(sizeof (void*), sizeof (Bool));
    //TEST_ASSERT_EQUAL(3 * sizeof (void*), sizeof (Iterator));
    //TEST_ASSERT_EQUAL(2 * sizeof (void*) + 8, sizeof (Bytes));
    //TEST_ASSERT_EQUAL(2 * sizeof (void*) + 8, sizeof (Str));
    //TEST_ASSERT_EQUAL(3 * sizeof (void*), sizeof (Lookup));
    //TEST_ASSERT_EQUAL(2 * sizeof (void*), sizeof (Tuple));
    //TEST_ASSERT_EQUAL(2 * sizeof (void*), sizeof (Exception));
    //TEST_ASSERT_EQUAL(2 * sizeof (void*), sizeof (Exception::Extra));

    // on 32b arch, packed = 12 bytes, which will fit in 2 GC slots i.s.o. 3
    // on 64b arch, packed is the same as unpacked as void* is also 8 bytes
    struct Packed { void* p; int64_t i; } __attribute__((packed));
    struct Normal { void* p; int64_t i; };
    TEST_ASSERT(sizeof (Packed) < sizeof (Normal) || 8 * sizeof (void*) == 64);

    //TEST_ASSERT_GREATER_OR_EQUAL(sizeof (Packed), sizeof (Int));
    //TEST_ASSERT_LESS_OR_EQUAL(sizeof (Normal), sizeof (Int));

    // TODO incorrect formulas (size rounded up), but it works on 32b & 64b ...
    //TEST_ASSERT_EQUAL(2 * sizeof (void*) + 8, sizeof (Range));
    //TEST_ASSERT_EQUAL(4 * sizeof (void*), sizeof (Slice));
}

void bigIntTests () {
#if 1
    TEST_IGNORE();
#else
    // check that ints over ± 30 bits properly switch to Int objects
    static int64_t tests [] = { 29, 30, 31, 32, 63 };

    for (auto e : tests) {
        int64_t pos = (1ULL << e) - 1;
        int64_t neg = - (1ULL << e);
        TEST_ASSERT(pos > 0); // make sure there was no overflow
        TEST_ASSERT(neg < 0); // make sure there was no underflow

        Value v = Int::make(pos);
        TEST_ASSERT_EQUAL_INT64(pos, v.asInt());
        TEST_ASSERT(e <= 30 ? v.isInt() : !v.isInt());

        Value w = Int::make(neg);
        TEST_ASSERT_EQUAL_INT64(neg, w.asInt());
        TEST_ASSERT(e <= 30 ? w.isInt() : !w.isInt());
    }
#endif
}

void varyVecTests () {
#if 1
    TEST_IGNORE();
#else
    VaryVec v;
    TEST_ASSERT_EQUAL(0, v.size());

    v.insert(0);
    TEST_ASSERT_EQUAL(1, v.size());
    TEST_ASSERT_EQUAL(0, v.atLen(0));

    v.atSet(0, "abc", 4);
    TEST_ASSERT_EQUAL(1, v.size());
    TEST_ASSERT_EQUAL(4, v.atLen(0));
    TEST_ASSERT_EQUAL_STRING("abc", v.atGet(0));

    v.insert(0);
    TEST_ASSERT_EQUAL(2, v.size());
    TEST_ASSERT_EQUAL(0, v.atLen(0));
    TEST_ASSERT_EQUAL(4, v.atLen(1));
    TEST_ASSERT_EQUAL_STRING("abc", v.atGet(1));

    v.atSet(0, "defg", 5);
    TEST_ASSERT_EQUAL(5, v.atLen(0));
    TEST_ASSERT_EQUAL(4, v.atLen(1));
    TEST_ASSERT_EQUAL_STRING("defg", v.atGet(0));
    TEST_ASSERT_EQUAL_STRING("abc", v.atGet(1));

    v.atSet(0, "hi", 3);
    TEST_ASSERT_EQUAL(3, v.atLen(0));
    TEST_ASSERT_EQUAL(4, v.atLen(1));
    TEST_ASSERT_EQUAL_STRING("hi", v.atGet(0));
    TEST_ASSERT_EQUAL_STRING("abc", v.atGet(1));

    v.atSet(0, nullptr, 0);
    TEST_ASSERT_EQUAL(0, v.atLen(0));
    TEST_ASSERT_EQUAL(4, v.atLen(1));
    TEST_ASSERT_EQUAL_STRING("abc", v.atGet(1));

    v.remove(0);
    TEST_ASSERT_EQUAL(1, v.size());
    TEST_ASSERT_EQUAL(4, v.atLen(0));
    TEST_ASSERT_EQUAL_STRING("abc", v.atGet(0));

    v.insert(1, 3);
    TEST_ASSERT_EQUAL(4, v.size());
    TEST_ASSERT_EQUAL(4, v.atLen(0));
    TEST_ASSERT_EQUAL_STRING("abc", v.atGet(0));
    TEST_ASSERT_EQUAL(0, v.atLen(1));
    TEST_ASSERT_EQUAL(0, v.atLen(2));
    TEST_ASSERT_EQUAL(0, v.atLen(3));
    v.atSet(3, "four", 5);
    TEST_ASSERT_EQUAL_STRING("four", v.atGet(3));
    v.atSet(2, "three", 6);
    TEST_ASSERT_EQUAL_STRING("three", v.atGet(2));
    v.atSet(1, "two", 4);
    TEST_ASSERT_EQUAL_STRING("two", v.atGet(1));
    v.atSet(0, "one", 4);
    TEST_ASSERT_EQUAL_STRING("one", v.atGet(0));
    TEST_ASSERT_EQUAL_STRING("two", v.atGet(1));
    TEST_ASSERT_EQUAL_STRING("three", v.atGet(2));
    TEST_ASSERT_EQUAL_STRING("four", v.atGet(3));

    v.remove(1, 2);
    TEST_ASSERT_EQUAL(2, v.size());
    TEST_ASSERT_EQUAL(4, v.atLen(0));
    TEST_ASSERT_EQUAL(5, v.atLen(1));
    TEST_ASSERT_EQUAL_STRING("one", v.atGet(0));
    TEST_ASSERT_EQUAL_STRING("four", v.atGet(1));
#endif
}

int main () {
    UNITY_BEGIN();

    RUN_TEST(smokeTest);
    RUN_TEST(dataSizes);
    RUN_TEST(bigIntTests);
    RUN_TEST(varyVecTests);

    UNITY_END();
}
