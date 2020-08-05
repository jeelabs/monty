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
    setup(memory, sizeof memory);
    memAvail = avail();

    TEST_ASSERT_EQUAL(0, v.cap());
    v.resize(25 * sizeof (int));

    for (int i = 0; i < 20; ++i)
        TEST_ASSERT_EQUAL(0, v[i]);

    TEST_ASSERT_EQUAL(0, v[10]);
    auto p = v.ptr();
    for (int i = 0; i < 10; ++i)
        p[i] = 11 * i;
}

void tearDown () {
    TEST_ASSERT_GREATER_THAN(0, v.cap());
    v.resize(0);
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
    TEST_ASSERT_EQUAL(2 * sizeof (void*), sizeof (VecOf<int>));
    TEST_ASSERT_EQUAL(2 * sizeof (void*), sizeof (VecOf<Vec>));
    TEST_ASSERT_EQUAL(2 * sizeof (void*), sizeof (VecOf<Chunk>));
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
}

void chunkTypeSizes () {
    TEST_ASSERT_EQUAL(3 * sizeof (void*), sizeof (Chunk));
    TEST_ASSERT_EQUAL(3 * sizeof (void*), sizeof (ChunkOf<int>));
    TEST_ASSERT_EQUAL(3 * sizeof (void*), sizeof (ChunkOf<Vec>));
    TEST_ASSERT_EQUAL(3 * sizeof (void*), sizeof (ChunkOf<Chunk>));
}

void chunkOfItems () {
    ChunkOf<int> c (v);

    static int m1 [] = { 0, 11, 22, 33, 44, 55, 66, 77, 88, 99, 0, };
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
}

void chunkOfInsAndDel () {
    ChunkOf<int> c (v);

    TEST_ASSERT_NOT_NULL(&c); // TODO placeholder
#if 0
    for (int i = 0; i < 11; ++i)
        printf("%d, ", v[i]);
    printf("\n");
#endif
}

void segmentTypeSizes () {
    TEST_ASSERT_EQUAL(4 * sizeof (void*), sizeof (Segment));
    TEST_ASSERT_EQUAL(4 * sizeof (void*), sizeof (SegmentOf<'l',int32_t>));
    TEST_ASSERT_EQUAL(4 * sizeof (void*), sizeof (SegmentOf<'V',Vec>));
    TEST_ASSERT_EQUAL(4 * sizeof (void*), sizeof (SegmentOf<'S',Segment>));
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
    auto a = new Array ('B');
    TEST_ASSERT_NOT_NULL(a);

    //a->segment.vec().resize(100);
    //a->segment.chunk.asVec<Value>().move(1,2,3);
    //a->segment.chunk.asVec<Chunk>().move(1,2,3);
}

auto main () -> int {
    UNITY_BEGIN();

    RUN_TEST(smokeTest);
    RUN_TEST(vecTypeSizes);
    RUN_TEST(vecOfInited);
    RUN_TEST(vecOfMoveAndWipe);

    RUN_TEST(chunkTypeSizes);
    RUN_TEST(chunkOfItems);
    RUN_TEST(chunkOfInsAndDel);

    RUN_TEST(segmentTypeSizes);

    RUN_TEST(vecOfVal);

    UNITY_END();
    return 0;
}
