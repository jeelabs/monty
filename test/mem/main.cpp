#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "xmonty.h"

#include <unity.h>

using namespace Monty;

constexpr auto MEMSZ = 1024;
uintptr_t memory [MEMSZ];
int created, destroyed, marked, failed;
size_t memAvail;

struct MarkObj : Obj {
    MarkObj (Obj* o =0) : other (o) { ++created; }
    ~MarkObj () override            { ++destroyed; }

private:
    void marker () const override   { ++marked; mark(other); }

    Obj* other;
};

void setUp () {
    setup(memory, sizeof memory);
    memAvail = avail();
    created = destroyed = marked = failed = 0;
    panicOutOfMemory = []() { ++failed; };
}

void tearDown () {
    sweep();
    TEST_ASSERT_EQUAL(created, destroyed);
    compact();
    TEST_ASSERT_EQUAL(memAvail, avail());
}

void smokeTest () {
    TEST_ASSERT_EQUAL(42, 40 + 2);
}

void memTypeSizes () {
    TEST_ASSERT_EQUAL(sizeof (void*), sizeof (Obj));
    TEST_ASSERT_EQUAL(2 * sizeof (void*), sizeof (Vec));
}

void initMem () {
    TEST_ASSERT_LESS_THAN(sizeof memory, avail());
    TEST_ASSERT_GREATER_THAN(sizeof memory - 50, avail());
}

void newObj () {
    MarkObj o1; // on the stack
    TEST_ASSERT(!o1.isCollectable());
    TEST_ASSERT_EQUAL(sizeof (MarkObj), sizeof o1);
    TEST_ASSERT_EQUAL(memAvail, avail());

    auto p1 = new MarkObj; // allocated in pool
    TEST_ASSERT_NOT_NULL(p1);
    TEST_ASSERT(p1->isCollectable());

    auto avail1 = avail();
    TEST_ASSERT_LESS_THAN(memAvail, avail1);

    auto p2 = new MarkObj; // second object in pool
    TEST_ASSERT(p2->isCollectable());
    TEST_ASSERT_NOT_EQUAL(p1, p2);

    auto avail2 = avail();
    TEST_ASSERT_LESS_THAN(avail1, avail2);
    TEST_ASSERT_EQUAL(memAvail - avail1, avail1 - avail2);

    auto p3 = new (0) MarkObj; // same as without the extra size
    TEST_ASSERT(p3->isCollectable());

    auto avail3 = avail();
    TEST_ASSERT_LESS_THAN(avail2, avail3);
    TEST_ASSERT_EQUAL(avail1 - avail2, avail2 - avail3);

    auto p4 = new (20) MarkObj; // extra space at end of object
    TEST_ASSERT(p4->isCollectable());

    auto avail4 = avail();
    TEST_ASSERT_LESS_THAN(avail3, avail4);
    TEST_ASSERT_GREATER_THAN(avail1 - avail2, avail3 - avail4);
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

void reuseObjs () {
    auto p1 = new MarkObj;          // [ p1 ]
    delete p1;                      // [ ]
    TEST_ASSERT_EQUAL(memAvail, avail());

    auto p2 = new MarkObj;          // [ p2 ]
    /* p3: */ new MarkObj;          // [ p3 p2 ]
    delete p2;                      // [ p3 gap ]
    TEST_ASSERT_EQUAL_PTR(p1, p2);
    TEST_ASSERT_LESS_THAN(memAvail, avail());

    auto p4 = new MarkObj;          // [ p3 p4 ]
    TEST_ASSERT_EQUAL_PTR(p1, p4);

    sweep();                   // [ ]
    TEST_ASSERT_EQUAL(memAvail, avail());
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
    auto p1 = new MarkObj;
    auto p2 = new MarkObj;
    auto p3 = new MarkObj;          // [ p3 p2 p1 ]

    delete p2;                      // [ p3 gap p1 ]
    delete p1;                      // [ p3 gap gap ]

    auto p4 = new (1) MarkObj;      // [ p3 p4 ]
    TEST_ASSERT_EQUAL_PTR(p2, p4);

    delete p4;                      // [ p3 gap ]
    delete p3;                      // [ ]
    TEST_ASSERT_EQUAL(memAvail, avail());
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

void outOfObjs () {
    constexpr auto N = 100 * sizeof (uintptr_t);
    for (int i = 0; i < 12; ++i)
        new (N) MarkObj;
    TEST_ASSERT_LESS_THAN(N, avail());
    TEST_ASSERT_EQUAL(12, created);
    TEST_ASSERT_EQUAL(3, failed);

    destroyed += failed; // avoid assertion in tearDown()
}

void newVec () {
    {
        Vec v1;
        TEST_ASSERT_EQUAL(2 * sizeof (void*), sizeof v1);
        TEST_ASSERT_EQUAL_PTR(0, v1.ptr());
        TEST_ASSERT_EQUAL(0, v1.cap());
        TEST_ASSERT_EQUAL(memAvail, avail());
    }
    TEST_ASSERT_EQUAL(memAvail, avail());
}

void resizeVec () {
    {
        Vec v1;
        auto f = v1.resize(0);
        TEST_ASSERT_TRUE(f);
        TEST_ASSERT_EQUAL_PTR(0, v1.ptr());
        TEST_ASSERT_EQUAL(0, v1.cap());
        TEST_ASSERT_EQUAL(memAvail, avail());

        f = v1.resize(1);
        TEST_ASSERT_TRUE(f);
        TEST_ASSERT_NOT_EQUAL(0, v1.ptr());
        TEST_ASSERT_EQUAL(sizeof (void*), v1.cap());
        TEST_ASSERT_LESS_THAN(memAvail, avail());

        f = v1.resize(sizeof (void*));
        TEST_ASSERT_TRUE(f);
        TEST_ASSERT_EQUAL(sizeof (void*), v1.cap());

        f = v1.resize(sizeof (void*) + 1);
        TEST_ASSERT_TRUE(f);
        TEST_ASSERT_EQUAL(3 * sizeof (void*), v1.cap());

        f = v1.resize(1);
        TEST_ASSERT_TRUE(f);
        TEST_ASSERT_EQUAL(sizeof (void*), v1.cap());
    }
    TEST_ASSERT_EQUAL(memAvail, avail());
}

void reuseVecs () {
    Vec v1;
    v1.resize(100);                 // [ v1 ]
    Vec v2;
    v2.resize(20);                  // [ v1 v2 ]

    auto a = avail();
    TEST_ASSERT_LESS_THAN(memAvail, a);
    TEST_ASSERT_GREATER_THAN(v1.ptr(), v2.ptr());

    v1.resize(0);                   // [ gap v2 ]
    TEST_ASSERT_EQUAL(a, avail());

    Vec v3;
    v3.resize(20);                  // [ v3 gap v2 ]
    TEST_ASSERT_EQUAL(a, avail());

    Vec v4;
    v4.resize(20);                  // [ v3 v4 gap v2 ]
    TEST_ASSERT_EQUAL(a, avail());

    v3.resize(0);                   // [ gap v4 gap v2 ]
    v4.resize(0);                   // [ gap gap v2 ]
    TEST_ASSERT_EQUAL(a, avail());

    Vec v5;
    v5.resize(100);                 // [ v5 v2 ]
    TEST_ASSERT_EQUAL(a, avail());

    v5.resize(40);                  // [ v5 gap v2 ]
    TEST_ASSERT_EQUAL(a, avail());

    v5.resize(80);                  // [ v5 gap v2 ]
    TEST_ASSERT_EQUAL(a, avail());

    v5.resize(100);                 // [ v5 v2 ]
    TEST_ASSERT_EQUAL(a, avail());

    v5.resize(20);                  // [ v5 gap v2 ]
    v1.resize(1);                   // [ v5 v1 gap v2 ]
    TEST_ASSERT_EQUAL(a, avail());

    v1.resize(0);                   // [ v5 gap v2 ]
    v5.resize(0);                   // [ gap v2 ]

    v2.resize(0);                   // [ gap ]
    auto b = avail();
    TEST_ASSERT_GREATER_THAN(a, b);
    TEST_ASSERT_LESS_THAN(memAvail, b);

    v1.resize(1);                   // [ v1 ]
    TEST_ASSERT_GREATER_THAN(b, avail());

    v1.resize(0);                   // [ ]
    TEST_ASSERT_EQUAL(memAvail, avail());
}

void compactVecs () {
    Vec v1;
    v1.resize(20);                 // [ v1 ]
    Vec v2;
    v2.resize(20);                 // [ v1 v2 ]

    auto a = avail();

    Vec v3;
    v3.resize(20);                 // [ v1 v2 v3 ]

    auto b = avail();

    Vec v4;
    v4.resize(20);                 // [ v1 v2 v3 v4 ]
    Vec v5;
    v5.resize(20);                 // [ v1 v2 v3 v4 v5 ]

    auto c = avail();
    TEST_ASSERT_LESS_THAN(b, c);

    compact();                      // [ v1 v2 v3 v4 v5 ]
    TEST_ASSERT_EQUAL(c, avail());

    v2.resize(0);                   // [ v1 gap v3 v4 v5 ]
    v4.resize(0);                   // [ v1 gap v3 gap v5 ]
    TEST_ASSERT_EQUAL(c, avail());

    compact();                      // [ v1 v3 v5 ]
    TEST_ASSERT_EQUAL(b, avail());

    v1.resize(0);                   // [ gap v3 v5 ]
    TEST_ASSERT_EQUAL(b, avail());

    compact();                      // [ v3 v5 ]
    TEST_ASSERT_EQUAL(a, avail());
}

void vecData () {
    {
        Vec v1;                     // fill with 0xFF's
        v1.resize(1000);
        TEST_ASSERT_GREATER_OR_EQUAL(1000, v1.cap());

        memset(v1.ptr(), 0xFF, v1.cap());
        TEST_ASSERT_EQUAL(0xFF, v1.ptr()[0]);
        TEST_ASSERT_EQUAL(0xFF, v1.ptr()[v1.cap()-1]);
    }

    Vec v2;
    v2.resize(20);                  // [ v2 ]
    auto p2 = v2.ptr();
    auto n = v2.cap();
    TEST_ASSERT_NOT_NULL(p2);
    TEST_ASSERT_GREATER_OR_EQUAL(20, n);

    p2[0] = 1;
    p2[n-1] = 2;
    TEST_ASSERT_EQUAL(1, p2[0]);
    TEST_ASSERT_EQUAL(0, p2[1]);
    TEST_ASSERT_EQUAL(0, p2[n-2]);
    TEST_ASSERT_EQUAL(2, p2[n-1]);

    v2.resize(40);                  // [ v2 ]
    TEST_ASSERT_EQUAL_PTR(p2, v2.ptr());
    TEST_ASSERT_EQUAL(1, p2[0]);
    TEST_ASSERT_EQUAL(2, p2[n-1]);
    TEST_ASSERT_EQUAL(0, p2[n]);
    TEST_ASSERT_EQUAL(0, p2[ v2.cap()-1]);

    v2.ptr()[n] = 11;
    v2.ptr()[v2.cap()-1] = 22;
    TEST_ASSERT_EQUAL(11, v2.ptr()[n]);
    TEST_ASSERT_EQUAL(22, v2.ptr()[v2.cap()-1]);

    Vec v3;
    v3.resize(20);                  // [ v2 v3 ]
    auto p3 = v3.ptr();
    p3[0] = 3;
    p3[n-1] = 4;
    TEST_ASSERT_EQUAL(3, p3[0]);
    TEST_ASSERT_EQUAL(4, p3[n-1]);

    Vec v4;
    v4.resize(20);                  // [ v2 v3 v4 ]
    auto p4 = v4.ptr();
    p4[0] = 5;
    p4[n-1] = 6;
    TEST_ASSERT_EQUAL(5, p4[0]);
    TEST_ASSERT_EQUAL(6, p4[n-1]);

    v3.resize(40);                  // [ v2 gap v4 v3 ]
    TEST_ASSERT_NOT_EQUAL(p3, v3.ptr());
    TEST_ASSERT_EQUAL(3, v3.ptr()[0]);
    TEST_ASSERT_EQUAL(4, v3.ptr()[n-1]);
    TEST_ASSERT_EQUAL(0, v3.ptr()[n]);
    TEST_ASSERT_EQUAL(0, v3.ptr()[v3.cap()-1]);

    v3.ptr()[n] = 33;
    v3.ptr()[v3.cap()-1] = 44;
    TEST_ASSERT_EQUAL(33, v3.ptr()[n]);
    TEST_ASSERT_EQUAL(44, v3.ptr()[v3.cap()-1]);

    auto a = avail();
    compact();                      // [ v2 v4 v3 ]
    TEST_ASSERT_GREATER_THAN(a, avail());

    TEST_ASSERT_EQUAL_PTR(p2, v2.ptr());
    TEST_ASSERT_EQUAL_PTR(p3, v4.ptr());
    TEST_ASSERT_EQUAL_PTR(p4, v3.ptr());

    TEST_ASSERT_EQUAL(1, p2[0]);
    TEST_ASSERT_EQUAL(22, p2[ v2.cap()-1]);

    TEST_ASSERT_EQUAL(3, v3.ptr()[0]);
    TEST_ASSERT_EQUAL(44, v3.ptr()[v3.cap()-1]);

    TEST_ASSERT_EQUAL(5, v4.ptr()[0]);
    TEST_ASSERT_EQUAL(6, v4.ptr()[n-1]);

    v2.resize(0);                   // [ gap v4 v3 ]
    v4.resize(0);                   // [ gap gap v3 ]

    compact();                      // [ v3 ]
    TEST_ASSERT_LESS_THAN(memAvail, avail());

    TEST_ASSERT_EQUAL_PTR(p2, v3.ptr());

    TEST_ASSERT_EQUAL(3, v3.ptr()[0]);
    TEST_ASSERT_EQUAL(44, v3.ptr()[v3.cap()-1]);
}

void outOfVecs () {
    Vec v1;
    auto f = v1.resize(999);
    TEST_ASSERT_TRUE(f);
    TEST_ASSERT_EQUAL(0, failed);

    auto p = v1.ptr();
    auto n = v1.cap();
    auto a = avail();
    TEST_ASSERT_NOT_EQUAL(0, p);
    TEST_ASSERT_GREATER_THAN(999, n);
    TEST_ASSERT_LESS_THAN(memAvail, a);

    f = v1.resize(sizeof memory); // fail, vector should be the old one

    TEST_ASSERT_FALSE(f);
    TEST_ASSERT_GREATER_THAN(0, failed);
    TEST_ASSERT_EQUAL_PTR(p, v1.ptr());
    TEST_ASSERT_EQUAL(n, v1.cap());
    TEST_ASSERT_EQUAL(a, avail());
}

void memRomOrRam () {
#if !NATIVE
    struct RomObj {
        virtual void blah () {} // virtual is romable
    };
    static const RomObj romObj;
    static       RomObj ramObj;

    struct DataObj {
        virtual ~DataObj () {} // but not if virtual destructor (!)
    };
    static const DataObj dataObj;

    struct BssObj { // with plain destructor, it needs run-time init (?)
        ~BssObj () {}
    };
    static const BssObj bssObj;

    //extern int _etext [];
    extern int _sdata [];
    extern int _edata [];
    extern int _sbss [];
    extern int _ebss [];

    auto rom = (void*) &romObj;
    TEST_ASSERT(rom < _sdata);                  // in flash, i.e. .rodata

    auto ram = (void*) &ramObj;                 // not const
    TEST_ASSERT(_sdata <= ram && ram < _edata); // pre-inited, i.e. in .data

    auto data = (void*) &dataObj;
    TEST_ASSERT(_sdata <= data && data < _edata);

    auto bss = (void*) &bssObj;
    TEST_ASSERT(_sbss <= bss && bss < _ebss);
#endif

    Object stackObj;
    auto heapObj = new Object;

    auto heap = (void*) heapObj;
    TEST_ASSERT(memory <= heap && heap < memory + MEMSZ);

    auto stack = (void*) &stackObj;
    TEST_ASSERT(heap <= stack);
}

int main () {
    UNITY_BEGIN();

    RUN_TEST(smokeTest);
    RUN_TEST(memTypeSizes);

    RUN_TEST(initMem);
    RUN_TEST(newObj);
    RUN_TEST(markObj);
    RUN_TEST(markThrough);
    RUN_TEST(reuseObjs);
    RUN_TEST(mergeNext);
    RUN_TEST(mergePrevious);
    RUN_TEST(mergeMulti);
    RUN_TEST(outOfObjs);

    RUN_TEST(newVec);
    RUN_TEST(resizeVec);
    RUN_TEST(reuseVecs);
    RUN_TEST(compactVecs);
    RUN_TEST(vecData);
    RUN_TEST(outOfVecs);

    RUN_TEST(memRomOrRam);

    UNITY_END();
}
