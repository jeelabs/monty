#include <stdint.h>
#include <unistd.h>
#include "mem.h"
#include <unity.h>

using namespace Monty;

uintptr_t memory [1024];
int created, destroyed, marked;

struct MarkObj : Obj {
    MarkObj (MarkObj* o =0) : other (o) { ++created; }
    ~MarkObj () override { ++destroyed; }

    void mark () const override { ++marked; Monty::mark(other); }

    MarkObj* other;
};

void setUp () {
    init(memory, sizeof memory);
    created = destroyed = marked = 0;
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

void reuseMem () {
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

void newVec () {
    auto avail1 = avail();
    {
        Vec v1; // on the stack
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
        Vec v1; // on the stack
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

int main (int argc, char **argv) {
    UNITY_BEGIN();

    RUN_TEST(smokeTest);
    RUN_TEST(initMem);
    RUN_TEST(newObj);
    RUN_TEST(markObj);
    RUN_TEST(markThrough);
    RUN_TEST(reuseMem);
    RUN_TEST(mergeNext);
    RUN_TEST(mergePrevious);
    RUN_TEST(mergeMulti);

    RUN_TEST(newVec);
    RUN_TEST(resizeVec);

    UNITY_END();
    return 0;
}
