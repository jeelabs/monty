#include <cstdlib>

#include "xmonty.h"

#include <unity.h>

using namespace Monty;

uintptr_t memory [1024];
int created, destroyed, marked, failed;

struct MarkObj : Obj {
    MarkObj (MarkObj* o =0) : other (o) { ++created; }
    ~MarkObj () override { ++destroyed; }

    void mark () const override { ++marked; Monty::mark(other); }

    MarkObj* other;
};

void setUp () {
    init(memory, sizeof memory);
    created = destroyed = marked = failed = 0;
    Monty::panicOutOfMemory = []() { ++failed; };
}

// void tearDown () {}

void smokeTest () {
    TEST_ASSERT_EQUAL(42, 40 + 2);
}

void initMem () {
    TEST_ASSERT_LESS_THAN(sizeof memory, avail());
    TEST_ASSERT_GREATER_THAN(sizeof memory - 50, avail());
}

void newObj () {
    auto avail1 = avail();

    Obj o1; // on the stack
    TEST_ASSERT(!o1.inObjPool());
    TEST_ASSERT_EQUAL(sizeof (void*), sizeof o1);
    TEST_ASSERT_EQUAL(avail1, avail());

    auto p1 = new Obj; // allocated in pool
    TEST_ASSERT_NOT_NULL(p1);
    TEST_ASSERT(p1->inObjPool());

    auto avail2 = avail();
    TEST_ASSERT_LESS_THAN(avail1, avail2);

    auto p2 = new Obj; // second object in pool
    TEST_ASSERT(p2->inObjPool());
    TEST_ASSERT_NOT_EQUAL(p1, p2);

    auto avail3 = avail();
    TEST_ASSERT_LESS_THAN(avail2, avail3);
    TEST_ASSERT_EQUAL(avail1 - avail2, avail2 - avail3);

    auto p3 = new (0) Obj; // same as without the extra size
    TEST_ASSERT(p3->inObjPool());

    auto avail4 = avail();
    TEST_ASSERT_LESS_THAN(avail3, avail4);
    TEST_ASSERT_EQUAL(avail2 - avail3, avail3 - avail4);

    auto p4 = new (1) Obj; // extra space at end of object
    TEST_ASSERT(p4->inObjPool());

    auto avail5 = avail();
    TEST_ASSERT_LESS_THAN(avail4, avail5);
    TEST_ASSERT_GREATER_THAN(avail2 - avail3, avail4 - avail5);
}

void markObj () {
    auto p1 = new MarkObj;
    TEST_ASSERT_EQUAL(1, created);

    auto p2 = new MarkObj (p1);
    TEST_ASSERT_EQUAL(2, created);

    mark(p2);
    TEST_ASSERT_EQUAL(2, marked);

    mark(p2);
    TEST_ASSERT_EQUAL(2, marked);

    sweep();
    TEST_ASSERT_EQUAL(0, destroyed);

    mark(p2);
    TEST_ASSERT_EQUAL(4, marked); // now everything is marked again

    sweep();
    TEST_ASSERT_EQUAL(0, destroyed);
    sweep();
    TEST_ASSERT_EQUAL(2, destroyed);
}

void markThrough () {
    auto p1 = new MarkObj;
    MarkObj o1 (p1); // not allocated, but still traversed when marking
    auto p2 = new MarkObj (&o1);
    TEST_ASSERT_EQUAL(3, created);

    mark(p2);
    TEST_ASSERT_EQUAL(3, marked);

    mark(p2);
    TEST_ASSERT_EQUAL(3, marked);

    sweep();
    TEST_ASSERT_EQUAL(0, destroyed);
    sweep();
    TEST_ASSERT_EQUAL(2, destroyed);
}

void reuseObjMem () {
    auto avail1 = avail();
    auto p1 = new MarkObj;          // [ p1 ]
    delete p1;                      // [ ]
    TEST_ASSERT_EQUAL(avail1, avail());

    auto p2 = new MarkObj;          // [ p2 ]
    /* p3: */ new MarkObj;          // [ p3 p2 ]
    delete p2;                      // [ p3 gap ]
    TEST_ASSERT_EQUAL_PTR(p1, p2);
    TEST_ASSERT_LESS_THAN(avail1, avail());

    auto p4 = new MarkObj;          // [ p3 p4 ]
    TEST_ASSERT_EQUAL_PTR(p1, p4);

    sweep();                   // [ ]
    TEST_ASSERT_EQUAL(avail1, avail());
}

void mergeNext () {
    auto p1 = new MarkObj;
    auto p2 = new MarkObj;
    /* p3: */ new MarkObj;          // [ p3 p2 p1 ]

    delete p1;
    delete p2;                      // [ p3 gap ]

    auto p4 = new (1) MarkObj;      // [ p3 p4 ]
    TEST_ASSERT_EQUAL_PTR(p2, p4);
}

void mergePrevious () {
    auto avail1 = avail();
    auto p1 = new MarkObj;
    auto p2 = new MarkObj;
    auto p3 = new MarkObj;          // [ p3 p2 p1 ]

    delete p2;                      // [ p3 gap p1 ]
    delete p1;                      // [ p3 gap gap ]

    auto p4 = new (1) MarkObj;      // [ p3 p4 ]
    TEST_ASSERT_EQUAL_PTR(p2, p4);

    delete p4;                      // [ p3 gap ]
    delete p3;                      // [ ]
    TEST_ASSERT_EQUAL(avail1, avail());
}

void mergeMulti () {
    auto p1 = new (100) MarkObj;
    auto p2 = new (100) MarkObj;
    auto p3 = new (100) MarkObj;
    /* p4: */ new (100) MarkObj;    // [ p4 p3 p2 p1 ]

    delete p3;                      // [ p4 gap p2 p1 ]
    delete p2;                      // [ p4 gap gap p1 ]
    delete p1;                      // [ p4 gap gap gap ]

    auto p5 = new (300) MarkObj;    // [ p4 p5 ]
    TEST_ASSERT_EQUAL_PTR(p3, p5);
}

void outOfObjMem () {
    constexpr auto N = 100 * sizeof (uintptr_t);
    for (int i = 0; i < 12; ++i)
        new (N) MarkObj;
    TEST_ASSERT_LESS_THAN(N, avail());
    TEST_ASSERT_EQUAL_PTR(12, created);
    TEST_ASSERT_GREATER_THAN(0, failed);
}

void newVec () {
    auto avail1 = avail();
    {
        Vec v1;
        TEST_ASSERT_EQUAL(2 * sizeof (void*), sizeof v1);
        TEST_ASSERT_EQUAL_PTR(0, v1.ptr());
        TEST_ASSERT_EQUAL(0, v1.cap());
        TEST_ASSERT_EQUAL(avail1, avail());
    }
    TEST_ASSERT_EQUAL(avail1, avail());
}

void resizeVec () {
    auto avail1 = avail();
    {
        Vec v1;
        v1.resize(0);
        TEST_ASSERT_EQUAL_PTR(0, v1.ptr());
        TEST_ASSERT_EQUAL(0, v1.cap());
        TEST_ASSERT_EQUAL(avail1, avail());

        v1.resize(1);
        TEST_ASSERT_NOT_EQUAL(0, v1.ptr());
        TEST_ASSERT_EQUAL(sizeof (void*), v1.cap());
        TEST_ASSERT_LESS_THAN(avail1, avail());

        v1.resize(sizeof (void*));
        TEST_ASSERT_EQUAL(sizeof (void*), v1.cap());

        v1.resize(sizeof (void*) + 1);
        TEST_ASSERT_EQUAL(3 * sizeof (void*), v1.cap());

        v1.resize(1);
        TEST_ASSERT_EQUAL(sizeof (void*), v1.cap());
    }
    TEST_ASSERT_EQUAL(avail1, avail());
}

void reuseVecMem () {
    auto avail1 = avail();

    Vec v1;
    v1.resize(100);                 // [ v1 ]
    Vec v2;
    v2.resize(10);                  // [ v1 v2 ]

    auto a = avail();
    TEST_ASSERT_LESS_THAN(avail1, a);
    TEST_ASSERT_GREATER_THAN(v1.ptr(), v2.ptr());

    v1.resize(0);                   // [ gap v2 ]
    TEST_ASSERT_EQUAL(a, avail());

    Vec v3;
    v3.resize(10);                  // [ v3 gap v2 ]
    TEST_ASSERT_EQUAL(a, avail());

    Vec v4;
    v4.resize(10);                  // [ v3 v4 gap v2 ]
    TEST_ASSERT_EQUAL(a, avail());

    v3.resize(0);                   // [ gap v4 gap v2 ]
    v4.resize(0);                   // [ gap gap v2 ]
    TEST_ASSERT_EQUAL(a, avail());

    Vec v5;
    v5.resize(100);                 // [ v5 v2 ]
    TEST_ASSERT_EQUAL(a, avail());

    v5.resize(10);                  // [ v5 gap v2 ]
    v1.resize(1);                   // [ v5 v1 gap v2 ]
    TEST_ASSERT_EQUAL(a, avail());

    v1.resize(0);                   // [ v5 gap v2 ]
    v5.resize(0);                   // [ gap v2 ]

    v2.resize(0);                   // [ gap ]
    auto b = avail();
    TEST_ASSERT_GREATER_THAN(a, b);
    TEST_ASSERT_LESS_THAN(avail1, b);

    v1.resize(1);                   // [ v1 ]
    TEST_ASSERT_GREATER_THAN(b, avail());

    v1.resize(0);                   // [ ]
    TEST_ASSERT_EQUAL(avail1, avail());
}

void outOfVecMem () {
    auto avail1 = avail();

    Vec v1;
    v1.resize(999);
    TEST_ASSERT_EQUAL(0, failed);

    auto p = v1.ptr();
    auto n = v1.cap();
    auto a = avail();
    TEST_ASSERT_NOT_EQUAL(0, p);
    TEST_ASSERT_GREATER_THAN(999, n);
    TEST_ASSERT_LESS_THAN(avail1, a);

    auto f = v1.resize(sizeof memory); // fail, vector should be the old one

    TEST_ASSERT_FALSE(f);
    TEST_ASSERT_GREATER_THAN(0, failed);
    TEST_ASSERT_EQUAL_PTR(p, v1.ptr());
    TEST_ASSERT_EQUAL(n, v1.cap());
    TEST_ASSERT_EQUAL(a, avail());
}

int main () {
    UNITY_BEGIN();

    RUN_TEST(smokeTest);
    RUN_TEST(initMem);
    RUN_TEST(newObj);
    RUN_TEST(markObj);
    RUN_TEST(markThrough);
    RUN_TEST(reuseObjMem);
    RUN_TEST(mergeNext);
    RUN_TEST(mergePrevious);
    RUN_TEST(mergeMulti);
    RUN_TEST(outOfObjMem);

    RUN_TEST(newVec);
    RUN_TEST(resizeVec);
    RUN_TEST(reuseVecMem);
    RUN_TEST(outOfVecMem);

    UNITY_END();
    return 0;
}
