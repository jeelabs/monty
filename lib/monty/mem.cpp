// Objects and vectors with garbage collection, implementation.

#include <cassert>
#include <stdint.h>
#include <stdlib.h>
#include <cstring>
#include <unistd.h>

#include "xmonty.h"

using namespace Monty;

struct ObjSlot {
    auto next () const -> ObjSlot* { return (ObjSlot*) (flag & ~1); }
    auto isFree () const -> bool   { return vt == 0; }
    auto isLast () const -> bool   { return chain == 0; }
    auto isMarked () const -> bool { return (flag & 1) != 0; }
    void setMark ()                { flag |= 1; }
    void clearMark ()              { flag &= ~1; }

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
    auto isFree () const -> bool { return vec == 0; }

    Vec* vec;
    union {
        VecSlot* next;
        uint8_t buf [];
    };
};

constexpr auto PTR_SZ = sizeof (void*);
constexpr auto VS_SZ  = sizeof (VecSlot);
constexpr auto OS_SZ  = sizeof (ObjSlot);

static_assert (OS_SZ == 2 * PTR_SZ, "wrong ObjSlot size");
static_assert (VS_SZ == 2 * PTR_SZ, "wrong VecSlot size");
static_assert (VS_SZ == OS_SZ, "mismatched slot sizes");

static uintptr_t* start;    // start of memory pool, aligned to OS_SZ-PTR_SZ
static uintptr_t* limit;    // limit of memory pool, aligned to OS_SZ-PTR_SZ

static ObjSlot* objLow;     // low water mark of object memory pool
static VecSlot* vecHigh;    // high water mark of vector memory pool

template< typename T >
static auto roundUp (size_t n) -> size_t {
    constexpr auto mask = sizeof (T) - 1;
    static_assert ((sizeof (T) & mask) == 0, "must be power of 2");
    return (n + mask) & ~mask;
}

template< typename T >
static auto multipleOf (size_t n) -> size_t {
    return roundUp<T>(n) / sizeof (T);
}

static auto obj2slot (Obj const& o) -> ObjSlot* {
    return o.inObjPool() ? (ObjSlot*) ((uintptr_t) &o - PTR_SZ) : 0;
}

static void mergeFreeObjs (ObjSlot& slot) {
    while (true) {
        auto nextSlot = slot.chain;
        assert(nextSlot != 0);
        if (!nextSlot->isFree() || nextSlot->isLast())
            break;
        slot.chain = nextSlot->chain;
    }
}

// don't use lambda w/ assert, since Espressif's ESP8266 compiler chokes on it
//void (*panicOutOfMemory)() = []() { assert(false); };
static void defaultOutOfMemoryHandler () { assert(false); }

namespace Monty {

    void (*panicOutOfMemory)() = defaultOutOfMemoryHandler;

    auto Obj::inObjPool () const -> bool {
        auto p = (void const*) this;
        return objLow <= p && p < limit;
    }

    auto Obj::operator new (size_t sz) -> void* {
        auto needs = multipleOf<ObjSlot>(sz + PTR_SZ);

        // traverse object pool, merge free slots, loop until first fit
        for (auto slot = objLow; !slot->isLast(); slot = slot->next())
            if (slot->isFree()) {
                mergeFreeObjs(*slot);
                auto space = slot->chain - slot;
                if (space >= (int) needs)
                    return &slot->obj;
            }

        if (objLow - needs < (void*) vecHigh)
            return panicOutOfMemory(), malloc(sz); // give up, last resort

        objLow -= needs;
        objLow->chain = objLow + needs;

        // new objects are always at least ObjSlot-aligned, i.e. 8-/16-byte
        assert((uintptr_t) &objLow->obj % OS_SZ == 0);
        return &objLow->obj;
    }

    void Obj::operator delete (void* p) {
        assert(p != 0);
        auto slot = obj2slot(*(Obj*) p);
        assert(slot != 0);

        slot->vt = 0; // mark this object as free and make it unusable

        mergeFreeObjs(*slot);

        // try to raise objLow, this will cascade when freeing during a sweep
        if (slot == objLow)
            objLow = objLow->chain;
    }

    // combine this free vec with all following free vecs
    // return true if vecHigh has been lowered, i.e. this free vec is now gone
    static auto mergeVecs (VecSlot& slot) -> bool {
        assert(slot.isFree());
        auto& tail = slot.next;
        while (tail < vecHigh && tail->isFree())
            tail = tail->next;
        if (tail < vecHigh)
            return false;
        vecHigh = &slot;
        return true;
    }
    
    auto Vec::resize (size_t sz) -> bool {
        auto needs = sz > 0 ? multipleOf<VecSlot>(sz + PTR_SZ) : 0;
        if (caps != needs) {
            auto slot = data != 0 ? (VecSlot*) (data - PTR_SZ) : 0;
            if (slot == 0) {                        // new alloc
                slot = (VecSlot*) start;
                while (slot < vecHigh)
                    if (!slot->isFree())
                        slot += slot->vec->caps;
                    else if (mergeVecs(*slot))
                        break;
                    else if (slot + needs > slot->next)
                        slot = slot->next;
                    else {
                        slot->vec = this;
                        if (slot + needs < slot->next) { // split, free at end
                            slot[needs].vec = 0;
                            slot[needs].next = slot->next;
                        }
                        break;
                    }
                if (slot == vecHigh) {
                    if ((uintptr_t) (vecHigh + needs) > (uintptr_t) objLow)
                        return panicOutOfMemory(), false;
                    vecHigh += needs;
                    slot->vec = this;
                }
                data = slot->buf;
            } else if (needs == 0) {             // delete
                slot->vec = 0;
                slot->next = slot + caps;
                mergeVecs(*slot);
                data = 0;
            } else if (slot + caps == vecHigh) {    // easy resize
                if ((uintptr_t) (slot + needs) > (uintptr_t) objLow)
                    return panicOutOfMemory(), false;
                vecHigh += needs - caps;
            } else {
                // TODO merge all free slots after this one
                //  ... then a "grow" might become a "shrink"
                if (needs > caps) {              // grow, i.e. del + new
                    if ((uintptr_t) (vecHigh + needs) > (uintptr_t) objLow)
                        return panicOutOfMemory(), false;
                    slot->vec = 0;
                    slot->next = slot + caps;
                    vecHigh->vec = this;
                    data = (uint8_t*) &vecHigh->next;
                    vecHigh += needs;
                } else {                            // shrink
                    // TODO
                }
            }
            if (needs > caps) {
                auto bytes = (needs - caps) * VS_SZ;
                if (caps == 0) // vector sizes are 4,12,20,28,... for 32b arch
                    bytes -= PTR_SZ;
                memset(data - PTR_SZ + needs * VS_SZ - bytes, 0, bytes);
            }
            caps = needs;
        }
        assert((uintptr_t) data % VS_SZ == 0);
        return true;
    }

    void init (uintptr_t* base, size_t size) {
        assert((uintptr_t) base % PTR_SZ == 0);
        assert(size % PTR_SZ == 0);

        start = base;
        limit = base + size / sizeof *base;

        // start & limit must not be exact multiples of ObjSlot/VecSlot sizes
        // when they are, simply increase start and/or decrease limit a bit
        // as a result, the allocated data itself sits on an xxxSlot boundary
        // this way no extra alignment is needed when setting up a memory pool
        if ((uintptr_t) start % VS_SZ == 0)
            ++start;
        if ((uintptr_t) limit % VS_SZ == 0)
            --limit;

        vecHigh = (VecSlot*) start;

        objLow = (ObjSlot*) limit - 1;
        objLow->chain = 0;
        objLow->vt = 0;

        assert((uintptr_t) &vecHigh->next % VS_SZ == 0);
        assert((uintptr_t) &objLow->obj % OS_SZ == 0);
    }

    auto avail () -> size_t {
        return (uintptr_t) objLow - (uintptr_t) vecHigh;
    }

    void mark (Obj const& obj) {
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
