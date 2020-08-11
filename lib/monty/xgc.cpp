// gc.cpp - objects and vectors with garbage collection

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "xmonty.h"

using namespace Monty;

struct ObjSlot {
    auto next () const -> ObjSlot* { return (ObjSlot*) (flag & ~1); }
    auto isFree () const -> bool   { return vt == nullptr; }
    auto isLast () const -> bool   { return chain == nullptr; }
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
    auto isFree () const -> bool { return vec == nullptr; }

    Vec* vec;
    union {
        VecSlot* next;
        uint8_t buf [1];
    };
};

constexpr auto PTR_SZ = sizeof (void*);
constexpr auto VS_SZ  = sizeof (VecSlot);
constexpr auto OS_SZ  = sizeof (ObjSlot);

static_assert (OS_SZ == 2 * PTR_SZ, "wrong ObjSlot size");
static_assert (VS_SZ == 2 * PTR_SZ, "wrong VecSlot size");
static_assert (OS_SZ == VS_SZ, "mismatched slot sizes");

static uintptr_t* start;    // start of memory pool, aligned to VS_SZ-PTR_SZ
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
    return o.isCollectable() ? (ObjSlot*) ((uintptr_t) &o - PTR_SZ) : 0;
}

static void mergeFreeObjs (ObjSlot& slot) {
    while (true) {
        auto nextSlot = slot.chain;
        assert(nextSlot != nullptr);
        if (!nextSlot->isFree() || nextSlot->isLast())
            break;
        slot.chain = nextSlot->chain;
    }
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

// turn a vec slot into a free slot, ending at tail
static void splitFreeVec (VecSlot& slot, VecSlot* tail) {
    if (tail <= &slot)
        return; // no room for a free slot
    slot.vec = nullptr;
    slot.next = tail;
}

// don't use lambda w/ assert, since Espressif's ESP8266 compiler chokes on it
// (hmmm, perhaps the assert macro is trying to obtain a function name ...)
//void (*panicOutOfMemory)() = []() { assert(false); };
static void defaultOutOfMemoryHandler () { assert(false); }

namespace Monty {
    void (*panicOutOfMemory)() = defaultOutOfMemoryHandler;

    auto Obj::isCollectable () const -> bool {
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
        assert(p != nullptr);
        auto slot = obj2slot(*(Obj*) p);
        assert(slot != nullptr);

        slot->vt = nullptr; // mark this object as free and make it unusable

        mergeFreeObjs(*slot);

        // try to raise objLow, this will cascade when freeing during a sweep
        if (slot == objLow)
            objLow = objLow->chain;
    }

    auto Vec::isResizable () const -> bool {
        auto p = (void*) data;
        return p == nullptr || (start < p && p < vecHigh);
    }

    auto Vec::cap () const -> size_t {
        return caps > 0 ? (2 * caps - 1) * PTR_SZ : 0;
    }

    auto Vec::findSpace (size_t needs) -> void* {
        auto slot = (VecSlot*) start;               // scan all vectors
        while (slot < vecHigh)
            if (!slot->isFree())                    // skip used slots
                slot += slot->vec->caps;
            else if (mergeVecs(*slot))              // no more free slots
                break;
            else if (slot + needs > slot->next)     // won't fit
                slot = slot->next;
            else {                                  // fits, may need to split
                splitFreeVec(slot[needs], slot->next);
                break;                              // found existing space
            }
        if (slot == vecHigh) {
            if ((uintptr_t) (vecHigh + needs) > (uintptr_t) objLow) {
                panicOutOfMemory();
                return 0; // no space found, and no room to expand
            }
            vecHigh += needs;
        }
        slot->vec = this;
        return slot;
    }

    // many tricky cases, to merge/reuse free slots as much as possible
    auto Vec::adj (size_t sz) -> bool {
        if (!isResizable())
            return false;
        auto needs = sz > 0 ? multipleOf<VecSlot>(sz + PTR_SZ) : 0;
        if (caps != needs) {
            auto slot = data != nullptr ? (VecSlot*) (data - PTR_SZ) : nullptr;
            if (slot == nullptr) {                  // new alloc
                slot = (VecSlot*) findSpace(needs);
                if (slot == nullptr)                // no room
                    return false;
                data = slot->buf;
            } else if (needs == 0) {                // delete
                slot->vec = nullptr;
                slot->next = slot + caps;
                mergeVecs(*slot);
                data = nullptr;
            } else {                                // resize
                auto tail = slot + caps;
                if (tail < vecHigh && tail->isFree())
                    mergeVecs(*tail);
                if (tail == vecHigh) {              // easy resize
                    if ((uintptr_t) (slot + needs) > (uintptr_t) objLow)
                        return panicOutOfMemory(), false;
                    vecHigh += needs - caps;
                } else if (needs < caps)            // split, free at end
                    splitFreeVec(slot[needs], slot + caps);
                else if (!tail->isFree() || slot + needs > tail->next) {
                    // realloc, i.e. del + new
                    auto nslot = (VecSlot*) findSpace(needs);
                    if (nslot == nullptr)                 // no room
                        return false;
                    memcpy(nslot->buf, data, cap()); // copy data over
                    data = nslot->buf;
                    slot->vec = nullptr;
                    slot->next = slot + caps;
                } else                              // use (part of) next free
                    splitFreeVec(slot[needs], tail->next);
            }
            // clear newly added bytes
            auto obytes = cap();
            caps = needs;
            auto nbytes = cap();
            if (nbytes > obytes)                    // clear added bytes
                memset(data + obytes, 0, nbytes - obytes);
        }
        assert((uintptr_t) data % VS_SZ == 0);
        return true;
    }

    void setup (uintptr_t* base, size_t size) {
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
        assert(start < limit); // need room for at least the objLow setup

        vecHigh = (VecSlot*) start;

        objLow = (ObjSlot*) limit - 1;
        objLow->chain = nullptr;
        objLow->vt = nullptr;

        assert((uintptr_t) &vecHigh->next % VS_SZ == 0);
        assert((uintptr_t) &objLow->obj % OS_SZ == 0);
    }

    auto avail () -> size_t {
        return (uintptr_t) objLow - (uintptr_t) vecHigh;
    }

    void mark (Obj const& obj) {
        auto p = obj2slot(obj);
        if (p != nullptr) {
            if (p->isMarked())
                return;
            p->setMark();
        }
        obj.marker();
    }

    void sweep () {
        for (auto slot = objLow; slot != nullptr; slot = slot->chain)
            if (slot->isMarked())
                slot->clearMark();
            else if (!slot->isFree()) {
                auto q = &slot->obj;
                delete q; // weird: must be a ptr *variable* for stm32 builds
                assert(slot->isFree());
            }
    }

    void compact () {
        auto newHigh = (VecSlot*) start;
        size_t n;
        for (auto slot = newHigh; slot < vecHigh; slot += n)
            if (slot->isFree())
                n = slot->next - slot;
            else {
                n = slot->vec->caps;
                if (newHigh < slot) {
                    slot->vec->data = newHigh->buf;
                    memmove(newHigh, slot, n * VS_SZ);
                }
                newHigh += n;
            }
        vecHigh = newHigh;
    }

} // namespace Monty
