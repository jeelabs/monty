// Objects and vectors with garbage collection, implementation.

#include <assert.h>
#include <stdint.h>
#include <unistd.h>

#include "mem.h"

using namespace Monty;

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
        Obj obj;
        void* vt; // null for deleted objects
    };
};

struct VecSlot {
    Vec* vec;
    VecSlot* next;
};

static_assert (sizeof (VecSlot) == sizeof (ObjSlot), "incorrect slot sizes");
static_assert (sizeof (ObjSlot) == 2 * sizeof (void*), "wrong ObjSlot size");
static_assert (sizeof (VecSlot) == 2 * sizeof (void*), "wrong VecSlot size");

static uintptr_t* start;
static uintptr_t* limit;

static ObjSlot* objLow;
static VecSlot* vecHigh;

template< typename T >
static size_t roundUp (size_t n) {
    constexpr auto mask = sizeof (T) - 1;
    static_assert ((sizeof (T) & mask) == 0, "must be power of 2");
    return (n + mask) & ~mask;
}

static ObjSlot* obj2slot (const Obj& o) {
    return o.inObjPool() ? (ObjSlot*) ((uintptr_t) &o - sizeof (void*)) : 0;
}

static VecSlot* vec2slot (const Vec& v) {
    return v.ptr() != 0 ? (VecSlot*) ((uintptr_t) v.ptr() - sizeof (void*)) : 0;
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

namespace Monty {

    bool Obj::inObjPool () const {
        auto p = (const void*) this;
        return objLow <= p && p < limit;
    }

    void* Obj::operator new (size_t sz) {
        auto need = roundUp<ObjSlot>(sz + sizeof (ObjSlot::chain));

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

        // new objects are always at least ObjSlot-aligned, i.e. 8-/16-byte
        assert((uintptr_t) &objLow->obj % sizeof (ObjSlot) == 0);
        return &objLow->obj;
    }

    void Obj::operator delete (void* p) {
        assert(p != 0);
        auto slot = obj2slot(*(Obj*) p);
        assert(slot != 0);

        slot->vt = 0; // mark this object as free and make it unusable

        mergeFreeSlots(slot);

        // try to raise objLow, this will cascade when freeing during a sweep
        if (slot == objLow)
            objLow = objLow->chain;
    }

    void Vec::resize (size_t sz) {
        auto slot = vec2slot(*this);
        if (slot == 0) {            // new alloc
            if (sz == 0)
                return;
            // TODO
        } else if (sz != 0) {       // resize
            // TODO
        } else {                    // delete
            slot->vec = 0;
            slot->next = slot + capa;
            if (vecHigh == slot->next)
                vecHigh = slot;
            data = 0;
            capa = 0;
        }
    }

    void init (uintptr_t* base, size_t size) {
        assert((uintptr_t) base % sizeof (void*) == 0);
        assert(size % sizeof (void*) == 0);

        start = base;
        limit = base + size / sizeof *base;

        // start & limit must not be exact multiples of ObjSlot/VecSlot sizes
        // when they are, simply increase start and/or decrease limit a bit
        // as a result, the allocated data itself sits on an xxxSlot boundary
        // this way no extra alignment is needed when setting up a memory pool
        if ((uintptr_t) start % sizeof (VecSlot) == 0)
            ++start;
        if ((uintptr_t) limit % sizeof (VecSlot) == 0)
            --limit;

        vecHigh = (VecSlot*) start;

        objLow = (ObjSlot*) limit - 1;
        objLow->chain = 0;
        objLow->vt = 0;

        assert((uintptr_t) &vecHigh->next % sizeof (VecSlot) == 0);
        assert((uintptr_t) &objLow->obj % sizeof (ObjSlot) == 0);
    }

    size_t avail () {
        return (uintptr_t) objLow - (uintptr_t) vecHigh;
    }

    void mark (const Obj& obj) {
        auto p = obj2slot(obj);
        if (p != 0) {
            if (p->isMarked())
                return;
            p->setMark();
        }
        obj.mark();
    }

    void sweep () {
        for (auto slot = objLow; slot != 0; slot = slot->chain)
            if (slot->isMarked())
                slot->clearMark();
            else if (!slot->isFree()) {
                auto q = &slot->obj;
                delete q; // weird: must be a ptr *variable* for stm32 builds
                assert(slot->isFree());
            }
    }

    void compact () {
        auto slot = (VecSlot*) start;
        while (slot < vecHigh) {
            // TODO
            ++slot; // wrong
        }
    }

} // namespace Monty
