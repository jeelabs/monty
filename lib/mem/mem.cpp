#include <assert.h>
#include <stdint.h>
#include <unistd.h>

#include "mem.h"

struct ObjSlot {
    ObjSlot* next () const { return (ObjSlot*) (flag & ~1); }
    bool isFree () const { return vt == 0; }
    bool isLast () const { return chain == 0; }
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
    for (auto slot = objLow; slot != 0; slot = slot->chain)
        if (slot->isMarked())
            slot->clearMark();
        else if (!slot->isFree()) {
            auto q = &slot->obj;
            delete q; // weird: must be a ptr *variable* for stm32 builds
            assert(slot->isFree());
        }
}

static void mergeFreeSlots (ObjSlot* slot) {
    while (true) {
        auto nextSlot = slot->chain;
        assert(nextSlot != 0);
        if (!nextSlot->isFree() || nextSlot->isLast())
            break;
        slot->chain = nextSlot->chain;
    }
}

void* Mem::Obj::operator new (size_t sz) {
    auto need = roundUp<void*>(sz + sizeof (ObjSlot::chain));

    for (auto slot = objLow; !slot->isLast(); slot = slot->next())
        if (slot->isFree()) {
            mergeFreeSlots(slot);
            auto space = (uintptr_t) slot->chain - (uintptr_t) slot;
            if (space >= need)
                return &slot->obj;
        }

    auto prev = objLow;
    objLow = (ObjSlot*) ((uintptr_t) prev - need);
    objLow->chain = prev;

    // new objects are always at least pointer-aligned
    assert((uintptr_t) &objLow->obj % sizeof (void*) == 0);
    return &objLow->obj;
}

void Mem::Obj::operator delete (void* p) {
    auto slot = obj2slot((Mem::Obj*) p);
    assert(slot != 0);

    slot->vt = 0; // mark this object as free and make it unusable

    mergeFreeSlots(slot);

    // try to raise objLow, this will cascade when freeing during a sweep
    if (slot == objLow)
        objLow = objLow->chain;
}

bool Mem::Obj::isAllocated () const {
    auto p = (const void*) this;
    return objLow <= p && p < limit;
}
