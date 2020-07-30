#include <unistd.h>
#include "mem.h"
#include <unity.h>

uintptr_t memory [1024];
int created, destroyed, marked;

struct MarkObj : Mem::Obj {
    MarkObj (MarkObj* o =0) : other (o) { ++created; }
    ~MarkObj () override { ++destroyed; }

    void mark () const override { ++marked; Mem::mark(other); }

    MarkObj* other;
};

void setUp () {
    Mem::init(memory, sizeof memory);
    created = destroyed = marked = 0;
}

// void tearDown () {}

void smokeTest () {
    TEST_ASSERT_EQUAL(42, 40 + 2);
}

void initMem () {
    TEST_ASSERT_LESS_THAN_size_t(sizeof memory, Mem::avail());
    TEST_ASSERT_GREATER_THAN_size_t(sizeof memory - 50, Mem::avail());
}

void newObj () {
    auto avail1 = Mem::avail();

    Mem::Obj o1; // on the stack
    TEST_ASSERT(!o1.isAllocated());
    TEST_ASSERT_EQUAL(sizeof (void*), sizeof o1);
    TEST_ASSERT_EQUAL(avail1, Mem::avail());

    auto p1 = new Mem::Obj; // allocated in pool
    TEST_ASSERT_NOT_NULL(p1);
    TEST_ASSERT(p1->isAllocated());

    auto avail2 = Mem::avail();
    TEST_ASSERT_LESS_THAN_size_t(avail1, avail2);

    auto p2 = new Mem::Obj; // second object in pool
    TEST_ASSERT(p2->isAllocated());
    TEST_ASSERT_NOT_EQUAL(p1, p2);

    auto avail3 = Mem::avail();
    TEST_ASSERT_LESS_THAN_size_t(avail2, avail3);
    TEST_ASSERT_EQUAL(avail1 - avail2, avail2 - avail3);

    auto p3 = new (0) Mem::Obj; // same as without the extra size
    TEST_ASSERT(p3->isAllocated());

    auto avail4 = Mem::avail();
    TEST_ASSERT_LESS_THAN_size_t(avail3, avail4);
    TEST_ASSERT_EQUAL(avail2 - avail3, avail3 - avail4);

    auto p4 = new (1) Mem::Obj; // extra space at end of object
    TEST_ASSERT(p4->isAllocated());

    auto avail5 = Mem::avail();
    TEST_ASSERT_LESS_THAN_size_t(avail4, avail5);
    TEST_ASSERT_GREATER_THAN_size_t(avail2 - avail3, avail4 - avail5);
}

void markObj () {
    auto p1 = new MarkObj;
    TEST_ASSERT_EQUAL(1, created);

    auto p2 = new MarkObj (p1);
    TEST_ASSERT_EQUAL(2, created);

    Mem::mark(p2);
    TEST_ASSERT_EQUAL(2, marked);

    Mem::mark(p2);
    TEST_ASSERT_EQUAL(2, marked);

    Mem::sweep();
    TEST_ASSERT_EQUAL(0, destroyed);

    Mem::mark(p2);
    TEST_ASSERT_EQUAL(4, marked); // now everything is marked again

    Mem::sweep();
    TEST_ASSERT_EQUAL(0, destroyed);
    Mem::sweep();
    TEST_ASSERT_EQUAL(2, destroyed);
}

void markThrough () {
    auto p1 = new MarkObj;
    MarkObj o1 (p1); // not allocated, but still traversed when marking
    auto p2 = new MarkObj (&o1);
    TEST_ASSERT_EQUAL(3, created);

    Mem::mark(p2);
    TEST_ASSERT_EQUAL(3, marked);

    Mem::mark(p2);
    TEST_ASSERT_EQUAL(3, marked);

    Mem::sweep();
    TEST_ASSERT_EQUAL(0, destroyed);
    Mem::sweep();
    TEST_ASSERT_EQUAL(2, destroyed);
}

void reuseMem () {
    auto avail1 = Mem::avail();
    auto p1 = new MarkObj;          // [ p1 ]
    delete p1;                      // [ ]
    TEST_ASSERT_EQUAL_size_t(avail1, Mem::avail());

    auto p2 = new MarkObj;          // [ p2 ]
    /* p3: */ new MarkObj;          // [ p3 p2 ]
    delete p2;                      // [ p3 gap ]
    TEST_ASSERT_EQUAL_PTR(p1, p2);
    TEST_ASSERT_LESS_THAN_size_t(avail1, Mem::avail());

    auto p4 = new MarkObj;          // [ p3 p4 ]
    TEST_ASSERT_EQUAL_PTR(p1, p4);

    Mem::sweep();                   // [ ]
    TEST_ASSERT_EQUAL_size_t(avail1, Mem::avail());
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
    auto avail1 = Mem::avail();
    auto p1 = new MarkObj;
    auto p2 = new MarkObj;
    auto p3 = new MarkObj;          // [ p3 p2 p1 ]

    delete p2;                      // [ p3 gap p1 ]
    delete p1;                      // [ p3 gap gap ]

    auto p4 = new (1) MarkObj;      // [ p3 p4 ]
    TEST_ASSERT_EQUAL_PTR(p2, p4);

    delete p4;                      // [ p3 gap ]
    delete p3;                      // [ ]
    TEST_ASSERT_EQUAL_size_t(avail1, Mem::avail());
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

    UNITY_END();
    return 0;
}
