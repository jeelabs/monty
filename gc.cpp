// Memory allocation and garbage collection for objects and vectors.

#define VERBOSE_GC      1 // show detailed memory allocation info & stats

#include "monty.h"

#include <assert.h>
#include <string.h>

#if VERBOSE_GC // 0 = off, 1 = stats, 2 = detailed
#if NATIVE
#include <stdio.h>
#else
#include <jee.h>
#endif
#endif

#if VERBOSE_GC <= 1
#define printf(...)
#endif

#define PREFIX "\t\tgc "

#if NATIVE
constexpr int MEM_BYTES = 24 * 1024;    // 24 Kb total memory
constexpr int MEM_ALIGN = 16;           // 16-byte slot boundaries
#else
constexpr int MEM_BYTES = 12 * 1024;    // 12 Kb total memory
constexpr int MEM_ALIGN = 8;            // 8-byte slot boundaries
#endif

// use 16- or 32-bit int headers: sign bit set for allocated blocks
// the remaining 15 or 31 bits are the slot size, in alignment units
// with 16-bit headers and 8-byte alignment, memory can be up to 256 KB

#if 1
typedef int16_t hdr_t;                  // header precedes pointer to block
constexpr int   HBYT = 2;               // header size in bytes
constexpr hdr_t USED = 0x8000;          // sign bit, set if in use
constexpr hdr_t MASK = 0x7FFF;          // size mask, in alignment units
#else
typedef int32_t hdr_t;                  // header precedes pointer to block
constexpr int   HBYT = 4;               // header size in bytes
constexpr hdr_t USED = 0x80000000;      // sign bit, set if in use
constexpr hdr_t MASK = 0x7FFFFFFF;      // size mask, in alignment units
#endif

static_assert (HBYT == sizeof (hdr_t), "HBYT does not match hdr_t");
static_assert (USED < 0,               "USED is not the sign of hdr_t");
static_assert (MASK == ~USED,          "MASK does not match USED");

constexpr int HPS = MEM_ALIGN / HBYT;   // headers per alignment slot
constexpr int MAX = MEM_BYTES / HBYT;   // memory size as header count

// use a fixed memory pool for now, indexed as headers and aligned to slots
static hdr_t mem [MAX] __attribute__ ((aligned (MEM_ALIGN)));

// convert an alloc pointer to the header reference which precedes it
static hdr_t& p2h (void *p) {
    assert(((uintptr_t) p) % MEM_ALIGN == 0);
    assert(mem < p && p <= mem + MAX);
    return ((hdr_t*) p)[-1];
}

// convert a header pointer to an alloc pointer
static void* h2p (hdr_t* h) { return h + 1; }

// true if this is an allocated block
static bool inUse (hdr_t h) { return h < 0; }

// header size in slots
static size_t h2s (hdr_t h) { return h & MASK; }

// header size in bytes
static size_t h2b (hdr_t h) { return h2s(h) * MEM_ALIGN - HBYT; }

// round bytes upwards to number of slots, leave room for next header
static int b2s (size_t bytes) {
    // with 8-byte alignment, up to 6 bytes will fit in the first slot
    return (bytes + HBYT + MEM_ALIGN - 1) / MEM_ALIGN;
}

// return a reference to the next block
static hdr_t& next (hdr_t* h) {
    return *(h + h2b(*h) / HBYT + 1);
}

// init pool to 1 partial slot, max-1 free slots, 1 allocated header
static void initMem () {
    constexpr auto slots = MAX / HPS; // aka MEM_BYTES / MEM_ALIGN
    printf("ma %d hps %d max %d slots %d mem %p\n",
            MEM_ALIGN, HPS, MAX, slots, mem);
    mem[HPS-1] = slots - 1;
    mem[MAX-1] = USED; // special end marker: used, but size 0
}

// merge any following free blocks into this one
static void coalesce (hdr_t* h) {
    if (!inUse(*h))
        while (!inUse(next(h)))
            *h += next(h); // free headers are positive and can be added
}

static void* release (void* p) {
    if (p != 0) {
        auto& h = p2h(p);
        assert(inUse(h));
        h &= ~USED;
    }
    return 0;
}

static void* allocate (size_t bytes) {
    if (mem[HPS-1] == 0)
        initMem();

    auto sz = b2s(bytes);
    printf("  alloc %d %d\n", (int) bytes, sz);
    for (auto h = &mem[HPS-1]; h2s(*h) > 0; h = &next(h))
        if (!inUse(*h)) {
            printf("    b %d sz %d h %p *h %04x\n", bytes, sz, h, *h);
            coalesce(h);
            if (*h >= sz) {
                auto n = *h;
                *h = sz;
                printf("    h %p next h %p end %p\n", h, &next(h), mem + MAX);
                if (n > sz)
                    next(h) = n - sz;
                *h |= USED;
                auto p = h2p(h);
                printf("    -> %p *h %04x next %04x\n",
                        p, (uint16_t) *h, (uint16_t) next(h));
                //memset(p, 0, bytes);
                return p;
            }
        }

    assert(false);
    return 0; // ouch, ran out of memory
}

static void* resize (void* p, size_t sz) {
    void* q = 0;
    if (sz > 0) {
        q = allocate(sz);
        if (p != 0) {
            auto osz = h2b(p2h(p));
            memcpy(q, p, sz < osz ? sz : osz);
        }
    }
    release(p);
    return q;
}

void* Object::allocator (size_t sz, void* p) {
#if 0
    if (sz != 0)
        return realloc(p, sz);
    free(p);
#else
    if (sz != 0)
        return resize(p, sz);
    release(p);
#endif
    return 0;
}

uint32_t totalAllocs,
         totalBytes,
         currAllocs,
         currBytes,
         maxAllocs,
         maxBytes;

void* Object::operator new (size_t sz) {
    auto p = allocator(sz);
    printf(PREFIX "new    %5d -> %p\n", (int) sz, p);

    ++totalAllocs;
    if (++currAllocs > maxAllocs)
        maxAllocs = currAllocs;
    totalBytes += sz;
    currBytes += sz;
    if (++currBytes > maxBytes)
        maxBytes = currBytes;

    return p;
}

void* Object::operator new (size_t sz, void* p) {
    printf(PREFIX "new #  %5d  @ %p\n", (int) sz, p);

    return p;
}

void Object::operator delete (void* p, size_t sz) {
    allocator(0, p);
    printf(PREFIX "delete %5d  : %p\n", (int) sz, p);

    --currAllocs;
    currBytes -= sz;
}

void Object::gcStats () {
#if VERBOSE_GC
#undef printf
    printf(PREFIX "total: %5d allocs %8d b\n", totalAllocs, totalBytes);
    printf(PREFIX " curr: %5d allocs %8d b\n", currAllocs, currBytes);
    printf(PREFIX "  max: %5d allocs %8d b\n", maxAllocs, maxBytes);
#endif
}
