// gc.cpp - objects and vectors with garbage collection

#define VERBOSE_GC 0 // show garbage collector activity

#include "monty.h"
#include <cassert>

using namespace monty;

#if VERBOSE_GC
#define D(x) { x; }
static void V (Object const& o, char const* s) {
    Value(o).dump(s);
}
#else
#define D(...)
#define V(...)
#endif

template auto Obj<Object>::operator new (size_t bytes) -> void*;
template void Obj<Object>::operator delete (void*);

template auto Obj<Object>::isInPool (void const* p) -> bool;
template void Obj<Object>::sweep ();
template void Obj<Object>::dumpAll ();

struct ObjSlot {
    auto next () const -> ObjSlot* { return (ObjSlot*) (flag & ~1); }
    auto isFree () const -> bool   { return vt == nullptr; }
    auto isLast () const -> bool   { return chain == nullptr; }
    auto isMarked () const -> bool { return (flag & 1) != 0; }
    void setMark ()                { flag |= 1; }
    void clearMark ()              { flag &= ~1; }

    // field order is essential, vt must be last
    union {
        ObjSlot* chain;
        uintptr_t flag; // bit 0 set for marked objects
    };
    void* vt; // null for deleted objects
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

GCStats monty::gcStats;

template< typename T >
static auto roundUp (uint32_t n) -> uint32_t {
    constexpr auto mask = sizeof (T) - 1;
    static_assert ((sizeof (T) & mask) == 0, "must be power of 2");
    return (n + mask) & ~mask;
}

template< typename T >
static auto multipleOf (uint32_t n) -> uint32_t {
    return roundUp<T>(n) / sizeof (T);
}

static auto obj2slot (Object const& o) -> ObjSlot* {
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
//void* (*panicOutOfMemory)() = []() { assert(false); return nullptr; };
static void* defaultOutOfMemoryHandler () { assert(false); return nullptr; }

namespace monty {
    void* (*panicOutOfMemory)() = defaultOutOfMemoryHandler;

    template< typename T >
    auto Obj<T>::isInPool (void const* p) -> bool {
        return objLow < p && p < limit;
    }

    template< typename T >
    auto Obj<T>::operator new (size_t sz) -> void* {
        auto needs = multipleOf<ObjSlot>(sz + PTR_SZ);

        ++gcStats.toa;
        ++gcStats.coa;
        if (gcStats.moa < gcStats.coa)
            gcStats.moa = gcStats.coa;
        gcStats.tob += needs * OS_SZ;
        gcStats.cob += needs * OS_SZ;
        if (gcStats.mob < gcStats.cob)
            gcStats.mob = gcStats.cob;

        // traverse object pool, merge free slots, loop until first fit
        for (auto slot = objLow; !slot->isLast(); slot = slot->next())
            if (slot->isFree()) {
                mergeFreeObjs(*slot);

                int slack = slot->chain - slot - needs;
                if (slack >= 0) {
                    if (slack > 0) { // put object at end of free space
                        slot->chain -= needs;
                        slot += slack;
                        slot->chain = slot + needs;
                    }
                    return &slot->vt;
                }
            }

        if (objLow - needs < (void*) vecHigh)
            return panicOutOfMemory(); // give up

        objLow -= needs;
        objLow->chain = objLow + needs;

        // new objects are always at least ObjSlot-aligned, i.e. 8-/16-byte
        assert((uintptr_t) &objLow->vt % OS_SZ == 0);
        return &objLow->vt;
    }

    template< typename T >
    void Obj<T>::operator delete (void* p) {
        assert(p != nullptr);
        auto slot = obj2slot(*(Object*) p);
        assert(slot != nullptr);

        --gcStats.coa;
        gcStats.cob -= (slot->chain - slot) * OS_SZ;

        slot->vt = nullptr; // mark this object as free and make it unusable

        mergeFreeObjs(*slot);

        // try to raise objLow, this will cascade when freeing during a sweep
        if (slot == objLow)
            objLow = objLow->chain;
    }

    template< typename T >
    void Obj<T>::sweep () {
        D( printf("\tsweeping ...\n"); )
        ++gcStats.sweeps;
        for (auto slot = objLow; slot != nullptr; slot = slot->chain)
            if (slot->isMarked())
                slot->clearMark();
            else if (!slot->isFree()) {
                auto q = (Object*) &slot->vt;
                V(*q, "\t delete");
                delete q;
                assert(slot->isFree());
            }
    }

    template< typename T >
    void Obj<T>::dumpAll () {
        printf("objects: %p .. %p\n", objLow, limit);
        for (auto slot = objLow; slot != nullptr; slot = slot->chain) {
            if (slot->chain == 0)
                break;
            int bytes = (slot->chain - slot) * sizeof *slot;
            printf("od: %p %6d b :", slot, bytes);
            if (slot->isFree())
                printf(" free\n");
            else {
                Value x {(Object const&) slot->vt};
                x.dump("");
            }
        }
    }

    auto Vec::isInPool (void const* p) -> bool {
        return start < p && p < vecHigh;
    }

    auto Vec::slots () const -> uint32_t {
        return multipleOf<VecSlot>(_capa);
    }

    auto Vec::findSpace (uint32_t needs) -> void* {
        auto slot = (VecSlot*) start;               // scan all vectors
        while (slot < vecHigh)
            if (!slot->isFree())                    // skip used slots
                slot += slot->vec->slots();
            else if (mergeVecs(*slot))              // no more free slots
                break;
            else if (slot + needs > slot->next)     // won't fit
                slot = slot->next;
            else {                                  // fits, may need to split
                splitFreeVec(slot[needs], slot->next);
                break;                              // found existing space
            }
        if (slot == vecHigh) {
            if ((uintptr_t) (vecHigh + needs) > (uintptr_t) objLow)
                return panicOutOfMemory(); // no space, and no room to expand
            vecHigh += needs;
        }
        slot->vec = this;
        return slot;
    }

    // many tricky cases, to merge/reuse free slots as much as possible
    auto Vec::adj (uint32_t sz) -> bool {
        if (!isResizable())
            return false;
        auto capas = slots();
        auto needs = sz > 0 ? multipleOf<VecSlot>(sz + PTR_SZ) : 0;
        if (capas != needs) {
            if (needs > capas)
                gcStats.tvb += (needs - capas) * VS_SZ;
            gcStats.cvb += (needs - capas) * VS_SZ;
            if (gcStats.mvb < gcStats.cvb)
                gcStats.mvb = gcStats.cvb;

            auto slot = _data != nullptr ? (VecSlot*) (_data - PTR_SZ) : nullptr;
            if (slot == nullptr) {                  // new alloc
                ++gcStats.tva;
                ++gcStats.cva;
                if (gcStats.mva < gcStats.cva)
                    gcStats.mva = gcStats.cva;

                slot = (VecSlot*) findSpace(needs);
                if (slot == nullptr)                // no room
                    return false;
                _data = slot->buf;
            } else if (needs == 0) {                // delete
                --gcStats.cva;

                slot->vec = nullptr;
                slot->next = slot + capas;
                mergeVecs(*slot);
                _data = nullptr;
            } else {                                // resize
                auto tail = slot + capas;
                if (tail < vecHigh && tail->isFree())
                    mergeVecs(*tail);
                if (tail == vecHigh) {              // easy resize
                    if ((uintptr_t) (slot + needs) > (uintptr_t) objLow)
                        return panicOutOfMemory(), false;
                    vecHigh += needs - capas;
                } else if (needs < capas)           // split, free at end
                    splitFreeVec(slot[needs], slot + capas);
                else if (!tail->isFree() || slot + needs > tail->next) {
                    // realloc, i.e. del + new
                    auto nslot = (VecSlot*) findSpace(needs);
                    if (nslot == nullptr)           // no room
                        return false;
                    memcpy(nslot->buf, _data, _capa); // copy data over
                    _data = nslot->buf;
                    slot->vec = nullptr;
                    slot->next = slot + capas;
                } else                              // use (part of) next free
                    splitFreeVec(slot[needs], tail->next);
            }
            // clear newly added bytes
            auto obytes = _capa;
            _capa = needs > 0 ? needs * VS_SZ - PTR_SZ : 0;
            if (_capa > obytes)                      // clear added bytes
                memset(_data + obytes, 0, _capa - obytes);
        }
        assert((uintptr_t) _data % VS_SZ == 0);
        return true;
    }

    void Vec::compact () {
        D( printf("\tcompacting ...\n"); )
        ++gcStats.compacts;
        auto newHigh = (VecSlot*) start;
        uint32_t n;
        for (auto slot = newHigh; slot < vecHigh; slot += n)
            if (slot->isFree())
                n = slot->next - slot;
            else {
                n = slot->vec->slots();
                if (newHigh < slot) {
                    slot->vec->_data = newHigh->buf;
                    memmove(newHigh, slot, n * VS_SZ);
                }
                newHigh += n;
            }
        vecHigh = newHigh;
    }

    void Vec::dumpAll () {
        printf("vectors: %p .. %p\n", start, vecHigh);
        uint32_t n;
        for (auto slot = (VecSlot*) start; slot < vecHigh; slot += n) {
            if (slot->isFree())
                n = slot->next - slot;
            else
                n = slot->vec->slots();
            printf("vd: %p %6d b :", slot, (int) (n * sizeof *slot));
            if (slot->isFree())
                printf(" free\n");
            else
                printf(" at %p\n", slot->vec);
        }
    }

    void gcSetup (void* base, uint32_t size) {
        assert(size > 2 * VS_SZ);

        // to get alignment right, simply increase base and decrease size a bit
        // as a result, the allocated data itself sits on an xxxSlot boundary
        // this way no extra alignment is needed when setting up a memory pool

        while ((uintptr_t) base % VS_SZ != PTR_SZ) {
            base = (uint8_t*) base + 1;
            --size;
        }
        size -= size % VS_SZ;

        start = (uintptr_t*) base;
        limit = (uintptr_t*) ((uintptr_t) base + size);

        assert(start < limit); // need room for at least the objLow setup

        vecHigh = (VecSlot*) start;

        objLow = (ObjSlot*) limit - 1;
        objLow->chain = nullptr;
        objLow->vt = nullptr;

        assert((uintptr_t) &vecHigh->next % VS_SZ == 0);
        assert((uintptr_t) &objLow->vt % OS_SZ == 0);

        D( printf("setup: start %p limit %p\n", start, limit); )
    }

    auto gcMax () -> uint32_t {
        return (uintptr_t) objLow - (uintptr_t) vecHigh;
    }

    auto gcCheck () -> bool {
        ++gcStats.checks;
        auto total = (uintptr_t) limit - (uintptr_t) start;
        return gcMax() < total / 4; // TODO crude
    }

    void mark (Object const& obj) {
        if (obj.isCollectable()) {
            auto p = obj2slot(obj);
            if (p != nullptr) {
                if (p->isMarked())
                    return;
                V(obj, "\t mark");
                p->setMark();
            }
        }
        obj.marker();
    }

    void gcReport () {
        printf("gc: max %d b, %d checks, %d sweeps, %d compacts\n",
                gcMax(), gcStats.checks, gcStats.sweeps, gcStats.compacts);
        printf("gc: total %6d objs %8d b, %6d vecs %8d b\n",
                gcStats.toa, gcStats.tob, gcStats.tva, gcStats.tvb);
        printf("gc:  curr %6d objs %8d b, %6d vecs %8d b\n",
                gcStats.coa, gcStats.cob, gcStats.cva, gcStats.cvb);
        printf("gc:   max %6d objs %8d b, %6d vecs %8d b\n",
                gcStats.moa, gcStats.mob, gcStats.mva, gcStats.mvb);
    }
}
