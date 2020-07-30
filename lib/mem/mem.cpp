#include <assert.h>
#include <stdint.h>
#include <unistd.h>

#include "mem.h"

struct ObjSlot {
    bool isMarked () const { return (flag & 1) != 0; }
    void setMark () { flag |= 1; }
    void clearMark () { flag &= ~1; }

    // field order is essential, obj must be last
    union {
        ObjSlot* next;
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

static ObjSlot* obj2slot (const Mem::Obj& o) {
    return o.isAllocated() ? (ObjSlot*) ((uintptr_t) &o - sizeof (void*)) : 0;
}

void Mem::init (uintptr_t* base, size_t size) {
    assert((uintptr_t) base % sizeof (void*) == 0);
    assert(size % sizeof (void*) == 0);

    start = base;
    limit = base + size / sizeof *base;

    objLow = (ObjSlot*) limit - 1;
    objLow->next = 0;
    objLow->vt = 0;
}

size_t Mem::avail () {
    return (uintptr_t) objLow - (uintptr_t) start;
}

void Mem::mark (const Obj& obj) {
    auto p = obj2slot(obj);
    if (p != 0) {
        if (p->isMarked())
            return;
        p->setMark();
    }
    obj.mark();
}

void Mem::sweep () {
    for (auto p = objLow; p != 0; p = p->next)
        if (p->isMarked())
            p->clearMark();
        else if (p->vt != 0) {
            auto q = &p->obj;
            delete q; // weird: must be a ptr *variable* for stm32 builds
        }
}

void* Mem::Obj::operator new (size_t sz) {
    auto need = roundUp<void*>(sz + sizeof (ObjSlot::next));
    auto prev = objLow;
    objLow = (ObjSlot*) ((uintptr_t) prev - need);
    objLow->next = prev;

    assert((uintptr_t) &objLow->obj % sizeof (void*) == 0);
    return &objLow->obj; // new objects are always at least pointer-aligned
}

void Mem::Obj::operator delete (void* p) {
    *(void**) p = 0; // clear the vtable pointer to make this object unusable
}

bool Mem::Obj::isAllocated () const {
    auto p = (const void*) this;
    return objLow <= p && p < limit;
}
