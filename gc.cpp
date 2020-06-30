// Memory allocation and garbage collection for objects and vectors.

#define VERBOSE_GC      0 // gc info & stats: 0 = off, 1 = stats, 2 = detailed
#define USE_MALLOC      0 // use standard allocator, no garbage collection
#define GC_REPORTS   1000 // print a gc stats report every 1000 allocs

#include "monty.h"

#include <assert.h>
#include <string.h>

#if VERBOSE_GC
#if NATIVE
#include <stdio.h>
#else
#include <jee.h>
#endif
#endif

#if VERBOSE_GC < 2
#define printf(...)
#endif

#define PREFIX "\t\tgc "

#if NATIVE
constexpr int MEM_BYTES = 20 * 1024;    // 20 Kb total memory
constexpr int MEM_ALIGN = 16;           // 16-byte slot boundaries
#else
constexpr int MEM_BYTES = 10 * 1024;    // 10 Kb total memory
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

#if 0
static void dump () {
    for (auto h = &mem[HPS-1]; h2s(*h) > 0; h = &next(h)) {
        const char* s = "*FREE*";
        if (inUse(*h)) {
            auto& obj = *(const Object*) h2p(h);
            s = obj.type().name;
        }
        printf("\t\t\t\tdump: %p %4x>%4x %6db %s\n",
                h, (uint16_t) *h, (uint16_t) next(h), h2b(*h), s);
    }
}
#endif

static uint32_t totalObjAllocs,
                totalObjBytes,
                currObjAllocs,
                currObjBytes,
                maxObjAllocs,
                maxObjBytes,
                totalVecAllocs,
                totalVecBytes,
                currVecAllocs,
                currVecBytes,
                maxVecAllocs,
                maxVecBytes;

static void release (void* p) {
    if (p != 0) {
        auto& h = p2h(p);
        assert(inUse(h));
        h &= ~USED;
        --currObjAllocs;
        currObjBytes -= h2s(h) * MEM_ALIGN;
    }
}

static void* allocate (size_t sz) {
    if (mem[HPS-1] == 0)
        initMem();

    auto ns = b2s(sz);
    printf("  alloc %d slots %d\n", (int) sz, ns);
    for (auto h = &mem[HPS-1]; h2s(*h) > 0; h = &next(h))
        if (!inUse(*h)) {
            //printf("    b %d ns %d h %p *h %04x\n", (int) sz, ns, h, *h);
            coalesce(h);
            if (*h >= ns) {
                auto os = *h;
                *h = ns;
                //printf("    h %p next h %p end %p\n", h, &next(h), mem + MAX);
                if (os > ns)
                    next(h) = os - ns;
                *h |= USED;
                auto p = h2p(h);
                printf("    -> %p *h %04x next %04x\n",
                        p, (uint16_t) *h, (uint16_t) next(h));

                if (++totalObjAllocs % GC_REPORTS == 0)
                    Object::gcStats();
                if (++currObjAllocs > maxObjAllocs)
                    maxObjAllocs = currObjAllocs;

                totalObjBytes += ns * MEM_ALIGN;
                currObjBytes += ns * MEM_ALIGN;
                if (currObjBytes > maxObjBytes)
                    maxObjBytes = currObjBytes;

                if (Context::gcCheck())
                    Context::raise(Value::nil); // exit inner loop
                return p;
            }
        }

    assert(false);
    return 0; // ouch, ran out of memory
}

static void* resize (void* p, size_t osz, size_t nsz) {
    if (nsz == 0) {
        assert(p != 0);
        --currVecAllocs;
        currVecBytes -= osz;
        free(p);
        return 0;
    }

    if (p == 0) {
        ++totalVecAllocs;
        if (++currVecAllocs > maxVecAllocs)
            maxVecAllocs = currVecAllocs;
    }

    currVecBytes += nsz - osz;
    if (currVecBytes > maxVecBytes)
        maxVecBytes = currVecBytes;

#if 1 // USE_MALLOC
    return realloc(p, nsz);
#else
    void* q = 0;
    if (sz > 0) {
        // determine old and new slot sizes
        auto os = p != 0 ? h2s(p2h(p)) : 0;
        size_t ns = b2s(sz);
        if (ns == os) // no change
            return p;
        if (ns < os) { // truncate
            auto& h = p2h(p);
            h = USED | ns;
            next(&h) = os - ns;
            return p;
        }
        // won't fit, need to copy to a new block
        q = allocate(sz);
        if (p != 0) {
            printf("resize #%d: %p #%d -> %p #%d (%db)\n",
                    (int) sz, p, (int) os, q, (int) ns,
                    (int) (os * MEM_ALIGN - HBYT));
            memcpy(q, p, os * MEM_ALIGN - HBYT);
        }
    }
    release(p);
    return q;
#endif
}

#if USE_MALLOC
#define allocate malloc
#define release free
#endif

// used only to alloc/resize/free variable data vectors
void Vector::alloc (size_t sz) {
    printf(PREFIX "resize %5d -> %-5d @ %p (used %d)\n",
            capacity, (int) sz, this, currObjBytes);
#if 0
    data = (uint8_t*) resize(data, capacity, sz);
#else
    currVecBytes += sz - capacity;
    if (currVecBytes > maxVecBytes)
        maxVecBytes = currVecBytes;

    constexpr auto PSZ = sizeof (void*);
    Data* p = 0;
    if (data != 0) {
        if (sz > 0)
            p = (Data*) realloc(data - PSZ, sz + PSZ);
        else {
            --currVecAllocs;

            free(data - PSZ);
        }
    } else {
        if (sz > 0) {
            ++totalVecAllocs;
            if (++currVecAllocs > maxVecAllocs)
                maxVecAllocs = currVecAllocs;

            p = (Data*) malloc(sz + PSZ);
        }
    }
    if (p != 0) {
        p->v = this;
        data = p->d;
    } else
        data = 0;
#endif
}

void* Object::operator new (size_t sz) {
    auto p = allocate(sz);
    printf(PREFIX "new    %5d -> %p (used %d)\n", (int) sz, p, currObjBytes);
    return p;
}

void* Object::operator new (size_t sz, void* p) {
    printf(PREFIX "new #  %5d  @ %p (used %d)\n", (int) sz, p, currObjBytes);
    return p;
}

void Object::operator delete (void* p) {
    printf(PREFIX "delete        : %p\n", p);
    auto& obj = *(const Object*) p;
    (void) obj;
    printf("\t\t\tdelete %p %s\n", &obj, obj.type().name);
    release(p);
}

#undef printf

void Object::gcStats () {
#if VERBOSE_GC
    printf("gc: total %6d objs %8d b, %6d vecs %8d b\n",
            totalObjAllocs, totalObjBytes, totalVecAllocs, totalVecBytes);
    printf("gc:  curr %6d objs %8d b, %6d vecs %8d b\n",
            currObjAllocs, currObjBytes, currVecAllocs, currVecBytes);
    printf("gc:   max %6d objs %8d b, %6d vecs %8d b\n",
            maxObjAllocs, maxObjBytes, maxVecAllocs, maxVecBytes);
#endif
}

bool Context::gcCheck () {
    return vm != 0 && (MEM_BYTES - currObjBytes) < (MEM_BYTES / 10);
}

static uint32_t tagBits [MAX/HPS/32];

static bool tagged (size_t n) {
    return (tagBits[n/32] & (1 << (n % 32))) != 0;
}

static void setTag (size_t n) {
    tagBits[n/32] |= 1 << (n % 32);
}

static void gcMarker (const Object& obj) {
    auto& vt = *(const void**) &obj;
    auto off = ((const hdr_t*) &obj - mem) / HPS;
    if (0 <= off && off < MAX/HPS) {
        if (tagged(off))
            return;
        //printf("\t\t\t\tmark %p ...%p %s\n", &obj, vt, obj.type().name);
        setTag(off);
    }
    obj.mark(gcMarker);
}

static void gcSweeper () {
    for (auto h = &mem[HPS-1]; h2s(*h) > 0; h = &next(h)) {
        //const char* s = "*FREE*";
        if (inUse(*h)) {
            auto obj = (Object*) h2p(h);
            auto off = ((const hdr_t*) obj - mem) / HPS;
            assert(0 <= off && off < MAX/HPS);
            if (!tagged(off)) {
                //printf("deleting %p %s\n", obj, obj->type().name);
                delete obj;
            }
        }
    }
}

void Context::gcTrigger () {
    //printf("gc triggered, %d b free\n", (int) (MEM_BYTES - currObjBytes));
    memset(tagBits, 0, sizeof tagBits);
    assert(vm != 0);
    gcMarker(*vm);
    gcSweeper();
}
