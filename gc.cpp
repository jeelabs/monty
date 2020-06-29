// Memory allocation and garbage collection for objects and vectors.

#define VERBOSE_GC      2 // show detailed memory allocation info & stats

#include "monty.h"

#include <assert.h>

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
constexpr int MEM_ALIGN = 16;           // 16-byte boundaries
#else
constexpr int MEM_BYTES = 12 * 1024;    // 12 Kb total memory
constexpr int MEM_ALIGN = 8;            // 8-byte boundaries
#endif

// 15-bit size header with 8-byte granularity can handle 256 KB memory
struct Header {
    uint16_t free :1;
    uint16_t size :15; // in MEM_ALIGN units

    void coalesce ();

    size_t bytes () const { return size * MEM_ALIGN - sizeof (Header); }
    Header& next () { return this[bytes() / sizeof (Header) + 1]; }
};

static Header mem [MEM_BYTES / sizeof (Header)];

static size_t b2slots (size_t b) {
    return (b + MEM_ALIGN);
}

static Header& header (void *p) {
    assert(((uintptr_t) p) % MEM_ALIGN == 0);
    assert(mem < p && p < mem + sizeof mem);
    return ((Header*) p)[-1];
}

static void initMem () {
    auto n = MEM_ALIGN / sizeof (Header);
    mem[0].size = n;
    mem[0].next().free = 1;
    mem[0].next().size = MEM_BYTES / MEM_ALIGN - 2;
    mem[0].next().next().size = 0;
}

void Header::coalesce () {
    if (free == 1)
        while (next().free)
            size += next().size;
}

static void release (void* p) {
    if (p != 0) {
        auto& h = header(p);
        assert(h.free == 0);
        h.free = 1;
    }
}

static void* allocate (size_t sz) {
    if (mem[0].size == 0)
        initMem();

    for (auto h = &mem->next(); h < mem + sizeof mem; h = &h->next())
        if (h->free) {
            h->coalesce();
            if (sz <= h->bytes()) {
                // ...
                h->free = 0;
                return h + 1;
            }
        }

    assert(false);
    return 0; // ouch, ran out of memory
}

static void resize (void* p, size_t sz) {
    // if sz == 0, use release instead
    // if p == 0, use allocate() instead
    auto& h = header(p);
    // round sz
    // if ==, done
    // if <, release past end
    // if > :
    //   coalesce past end
    //   if it now fits, done
    //   if <, alloc new & move
    //   if >, release past end
}

uint32_t totalAllocs,
         totalBytes,
         currAllocs,
         currBytes,
         maxAllocs,
         maxBytes;

void* Object::allocator (size_t sz, void* p) {
    if (sz != 0)
        return realloc(p, sz);
    if (p != 0)
        free(p);
    return 0;
}

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
