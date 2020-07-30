#include <assert.h>
#include <stdint.h>
#include <unistd.h>

#include "mem.h"

struct ObjSlot {
    ObjSlot* next () const { return (ObjSlot*) (flag & ~1); }
    bool isFree () const { return vt == 0; }
    bool isMarked () const { return (flag & 1) != 0; }
    void setMark () { flag |= 1; }
    void clearMark () { flag &= ~1; }

    // field order is essential, obj must be last
    union {
        ObjSlot* chain;
        uintptr_t flag; // bit 0 set for marked objects
    };
    union {
        Mem::Obj obj;
        void* vt; // null for deleted objects
    };
};

static uintptr_t* start;
static uintptr_t* limit;

static ObjSlot* objLow;

template< typename T >
static size_t roundUp (size_t n) {
    constexpr auto mask = sizeof (T) - 1;
    static_assert ((sizeof (T) & mask) == 0, "must be power of 2");
    return (n + mask) & ~mask;
}

static ObjSlot* obj2slot (const Mem::Obj* p) {
    assert(p != 0);
    return p->isAllocated() ? (ObjSlot*) ((uintptr_t) p - sizeof (void*)) : 0;
}

void Mem::init (uintptr_t* base, size_t size) {
    assert((uintptr_t) base % sizeof (void*) == 0);
    assert(size % sizeof (void*) == 0);

    start = base;
    limit = base + size / sizeof *base;

    objLow = (ObjSlot*) limit - 1;
    objLow->chain = 0;
    objLow->vt = 0;
}

size_t Mem::avail () {
    return (uintptr_t) objLow - (uintptr_t) start;
}

void Mem::mark (const Obj& obj) {
    auto p = obj2slot(&obj);
    if (p != 0) {
        if (p->isMarked())
            return;
        p->setMark();
    }
    obj.mark();
}

void Mem::sweep () {
    for (auto p = objLow; p != 0; p = p->chain)
        if (p->isMarked())
            p->clearMark();
        else if (!p->isFree()) {
            auto q = &p->obj;
            delete q; // weird: must be a ptr *variable* for stm32 builds
            assert(p->isFree());
        }
}

void* Mem::Obj::operator new (size_t sz) {
    auto need = roundUp<void*>(sz + sizeof (ObjSlot::chain));

    for (auto p = objLow; p->chain != 0; p = p->next())
        if (p->isFree()) {
            auto space = (uintptr_t) p->chain - (uintptr_t) p;
            if (space >= need)
                return &p->obj;
        }

    auto prev = objLow;
    objLow = (ObjSlot*) ((uintptr_t) prev - need);
    objLow->chain = prev;

    // new objects are always at least pointer-aligned
    assert((uintptr_t) &objLow->obj % sizeof (void*) == 0);
    return &objLow->obj;
}

void Mem::Obj::operator delete (void* p) {
    // merge with next if also free
    auto mySlot = obj2slot((Mem::Obj*) p);
    assert(mySlot != 0 && mySlot->chain != 0);
    auto nextSlot = mySlot->chain;

    mySlot->vt = 0; // mark this object as free and make it unusable

#if 1
    if (nextSlot->isFree() && nextSlot->chain != 0)
        mySlot->chain = nextSlot->chain;
#endif

    // try to raise objLow, this will cascade when freeing during a sweep
    if (mySlot == objLow)
        objLow = objLow->chain;
}

bool Mem::Obj::isAllocated () const {
    auto p = (const void*) this;
    return objLow <= p && p < limit;
}
