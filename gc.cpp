// Implementation of the main datatypes, i.e. Value, TypeObj, FrameObj, Context.

#define VERBOSE_GC      2 // show detailed memory allocation info & stats

#include "monty.h"

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

uint32_t totalAllocs,
         totalBytes,
         currAllocs,
         currBytes,
         maxAllocs,
         maxBytes;

#if NATIVE
constexpr int MEM_BYTES = 24 * 1024;    // 24 Kb total memory
constexpr int MEM_ALIGN = 16;           // 16-byte boundaries
#else
constexpr int MEM_BYTES = 12 * 1024;    // 12 Kb total memory
constexpr int MEM_ALIGN = 8;            // 8-byte boundaries
#endif

static uint32_t pool [MEM_BYTES / sizeof (uint32_t)]; // actual memory pool
static uint32_t used [MEM_BYTES / MEM_ALIGN / 32];    // 1b per align boundary

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
