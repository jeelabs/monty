// Memory allocation, garbage collection of objects, and compaction of vectors.

#include "monty.h"
#include "config.h"

#include <assert.h>
#include <string.h>

#ifndef GC_VERBOSE
#define GC_VERBOSE      0 // gc info & stats: 0 = off, 1 = stats, 2 = detailed
#endif
#ifndef GC_MALLOCS
#define GC_MALLOCS      0 // use standard allocator, no garbage collection
#endif
#ifndef GC_REPORTS
#define GC_REPORTS   1000 // print a gc stats report every 1000 allocs
#endif

#ifndef GC_MEM_BYTES
#define GC_MEM_BYTES (20*1024)  // 20 Kb total memory
#endif
#ifndef GC_VEC_BYTES
#define GC_VEC_BYTES (3*1024)   // enough for basic testing on 32-/64-bit arch
#endif
#ifndef GC_MEM_ALIGN
#define GC_MEM_ALIGN 16         // 16-byte slot boundaries
#endif

#if GC_VERBOSE
extern "C" int debugf (const char*, ...);
#endif

#if GC_VERBOSE < 2
#define debugf(...)
#endif

#define PREFIX "\t\tgc "

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

constexpr int HPS = GC_MEM_ALIGN / HBYT;   // headers per alignment slot
constexpr int MAX = GC_MEM_BYTES / HBYT;   // memory size as header count

// use a fixed memory pool for now, indexed as headers and aligned to slots
static hdr_t mem [MAX] __attribute__ ((aligned (GC_MEM_ALIGN)));

// convert an alloc pointer to the header reference which precedes it
static hdr_t& p2h (void *p) {
    assert(((uintptr_t) p) % GC_MEM_ALIGN == 0);
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
static size_t h2b (hdr_t h) { return h2s(h) * GC_MEM_ALIGN - HBYT; }

// round bytes upwards to number of slots, leave room for next header
static int b2s (size_t bytes) {
    // with 8-byte alignment, up to 6 bytes will fit in the first slot
    return (bytes + HBYT + GC_MEM_ALIGN - 1) / GC_MEM_ALIGN;
}

// return a reference to the next block
static hdr_t& nextObj (hdr_t* h) {
    return *(h + h2b(*h) / HBYT + 1);
}

// init pool to 1 partial slot, max-1 free slots, 1 allocated header
static void initMem () {
    constexpr auto slots = MAX / HPS; // aka GC_MEM_BYTES / GC_MEM_ALIGN
    debugf("ma %d hps %d max %d slots %d mem %p\n",
            GC_MEM_ALIGN, HPS, MAX, slots, mem);
    mem[HPS-1] = slots - 1;
    mem[MAX-1] = USED; // special end marker: used, but size 0
}

// merge any following free blocks into this one
static void coalesce (hdr_t* h) {
    if (!inUse(*h))
        while (!inUse(nextObj(h)))
            *h += nextObj(h); // free headers are positive and can be added
}

static uint32_t totalObjAllocs, totalObjBytes, totalVecAllocs, totalVecBytes,
                currObjAllocs, currObjBytes, currVecAllocs, currVecBytes,
                maxObjAllocs, maxObjBytes, maxVecAllocs, maxVecBytes;

static void* allocate (size_t sz) {
#if GC_MALLOCS
    return malloc(sz);
#else
    if (mem[MAX-1] == 0)
        initMem();

    auto ns = b2s(sz);
    debugf("  alloc %d slots %d\n", (int) sz, ns);
    for (auto h = &mem[HPS-1]; h2s(*h) > 0; h = &nextObj(h))
        if (!inUse(*h)) {
            //debugf("    b %d ns %d h %p *h %04x\n", (int) sz, ns, h, *h);
            coalesce(h);
            if (*h >= ns) {
                auto os = *h;
                *h = ns;
                //debugf("    h %p next h %p end %p\n", h, &nextObj(h), mem + MAX);
                if (os > ns)
                    nextObj(h) = os - ns;
                *h |= USED;
                auto p = h2p(h);
                debugf("    -> %p *h %04x next %04x\n",
                        p, (uint16_t) *h, (uint16_t) nextObj(h));

                if (++totalObjAllocs % GC_REPORTS == 0)
                    Object::gcStats();
                if (++currObjAllocs > maxObjAllocs)
                    maxObjAllocs = currObjAllocs;

                totalObjBytes += ns * GC_MEM_ALIGN;
                currObjBytes += ns * GC_MEM_ALIGN;
                if (currObjBytes > maxObjBytes)
                    maxObjBytes = currObjBytes;

                if (Context::gcCheck())
                    Context::raise(Value::nil); // exit inner loop
                return p;
            }
        }

    assert(false);
    return 0; // ouch, ran out of memory
#endif
}

static void release (void* p) {
#if GC_MALLOCS
    free(p);
#else
    if (p != 0) {
        auto& h = p2h(p);
        assert(inUse(h));
        h &= ~USED;
        --currObjAllocs;
        currObjBytes -= h2s(h) * GC_MEM_ALIGN;
    }
#endif
}

static uint8_t vecs [GC_VEC_BYTES] __attribute__ ((aligned (GC_MEM_ALIGN)));
static uint8_t* vecTop = vecs;

static size_t roundUp (size_t n, size_t unit) {
    auto mask = unit - 1;
    assert((unit & mask) == 0); // must be power of 2
    return (n + mask) & ~ mask;
}

// used only to alloc/resize/free variable data vectors
void Vector::alloc (size_t sz) {
    debugf(PREFIX "resize %5d -> %d   @ %p (u %d) d %p\n",
            (int) capacity, (int) sz, this, (int) (vecTop - vecs), data);
#if GC_MALLOCS
    data = (Data*) realloc(data, sizeof (void*) + sz);
    data->v = this;
#else
    assert(vecs <= vecTop && vecTop < vecs + sizeof vecs);

    constexpr auto PSZ = sizeof (void*);
    constexpr auto DSZ = sizeof (Data);
    static_assert (2 * PSZ == DSZ, "unexpected Vector::Data size");

    auto osz = roundUp(PSZ + capacity, DSZ);
    auto nsz = roundUp(PSZ + sz, DSZ);

    if (nsz > osz)
        totalVecBytes += nsz - osz;
    currVecBytes += nsz - osz;
    if (currVecBytes > maxVecBytes)
        maxVecBytes = currVecBytes;

    if (sz == 0) {
        if (data != 0) {
            --currVecAllocs;
            //debugf("resize gap %p #%d\n", data, (int) capacity);
            assert(data->v == this);
            data->v = 0;
            data->n = capacity;
            data = 0;
        }
        return;
    }

    assert(nsz >= osz); // no need to support shrinking, gcCompact will do that

    if (data == 0) {
        ++totalVecAllocs;
        if (++currVecAllocs > maxVecAllocs)
            maxVecAllocs = currVecAllocs;
        assert(capacity == 0);
        data = (Data*) vecTop;
        vecTop += DSZ;
        data->v = this;
        //debugf("new data %p vecTop %p\n", data, vecTop);
    }

    if (nsz == osz)
        return; // it already fits

    //debugf("  incr sz %d to %d top %p\n", (int) osz, (int) nsz, vecTop);
    assert(data->v == this);
    assert((uint8_t*) data->next() <= vecTop);

    if ((uint8_t*) data + osz == vecTop) {
        //debugf("    last vector, expanding in-place\n");
        vecTop = (uint8_t*) data + nsz;
        return;
    }

    auto p = (Data*) vecTop;
    vecTop += nsz;
    p->v = this;
    memcpy(p->d, data->d, fill * width());

    data->v = 0;
    data->n = capacity;

    data = p;
#endif
}

void* Object::operator new (size_t sz) {
    auto p = allocate(sz);
    debugf(PREFIX "new    %5d -> %p (used %d)\n", (int) sz, p, currObjBytes);
    return p;
}

void* Object::operator new (size_t sz, void* p) {
    debugf(PREFIX "new #  %5d  @ %p (used %d)\n", (int) sz, p, currObjBytes);
    return p;
}

void Object::operator delete (void* p) {
    auto& obj = *(const Object*) p;
    (void) obj;
    debugf(PREFIX "delete        : %p %s\n", p, obj.type().name);
    release(p);
}

#undef debugf

void Object::gcStats () {
#if GC_VERBOSE
    debugf("gc: total %6d objs %8d b, %6d vecs %8d b\n",
            totalObjAllocs, totalObjBytes, totalVecAllocs, totalVecBytes);
    debugf("gc:  curr %6d objs %8d b, %6d vecs %8d b\n",
            currObjAllocs, currObjBytes, currVecAllocs, currVecBytes);
    debugf("gc:   max %6d objs %8d b, %6d vecs %8d b\n",
            maxObjAllocs, maxObjBytes, maxVecAllocs, maxVecBytes);
#endif
}

bool Context::gcCheck () {
    return vm != 0 && ((GC_MEM_BYTES - currObjBytes) < (GC_MEM_BYTES / 10) ||
            (vecTop - vecs > (int) sizeof vecs - 1000)); // TODO 10? 1000?
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
    (void) vt;
    auto off = ((const hdr_t*) &obj - mem) / HPS;
    if (0 <= off && off < MAX/HPS) {
        if (tagged(off))
            return;
#if GC_VERBOSE > 1
        debugf("\t\t\t\tmark %p ...%p %s\n", &obj, vt, obj.type().name);
#endif
        setTag(off);
    }
    obj.mark(gcMarker);
}

static void gcSweeper () {
    for (auto h = &mem[HPS-1]; h2s(*h) > 0; h = &nextObj(h)) {
        //const char* s = "*FREE*";
        if (inUse(*h)) {
            auto obj = (Object*) h2p(h);
            auto off = ((const hdr_t*) obj - mem) / HPS;
            assert(0 <= off && off < MAX/HPS);
            if (!tagged(off)) {
                //debugf("deleting %p %s\n", obj, obj->type().name);
                delete obj;
                *(void**) obj = 0; // clear vtable to make Object* unusable
            }
        }
    }
}

Vector::Data* Vector::Data::next () const {
    auto off = sizeof (void*) + (v != 0 ? v->capacity : n);
    auto p = (Data*) ((uint8_t*) this + roundUp(off, sizeof (Data)));
    assert(vecs <= (uint8_t*) p && (uint8_t*) p <= vecs + sizeof vecs);
    return p;
}

void Vector::gcCompact () {
    auto newTop = (Data*) vecs;
    for (auto p = newTop; (uint8_t*) p < vecTop; p = p->next()) {
        int n = p->next() - p;
#if GC_VERBOSE > 1
        debugf("\t\t\t\tvec %p v %p size %d (%d b)\n", p, p->v,
                (int) (p->v != 0 ? p->v->capacity : -1),
                (int) (n * sizeof (Data)));
#endif
        if (p->v != 0) {
#if GC_VERBOSE > 1
            debugf("\tcompact %p -> %p #%d\n",
                    p, newTop, (int) (n * sizeof (Data)));
#endif
            if (newTop < p->v->data) {
                p->v->data = newTop; // adjust now, p->v may get clobbered
                memmove(newTop, p, n * sizeof (Data));
            }
            newTop += n;
        }
    }
    vecTop = (uint8_t*) newTop;
}

void Context::gcTrigger () {
#if GC_VERBOSE
    debugf("gc start, used b: %7d obj + %7d vec (gap %d)\n",
            (int) currObjBytes, (int) currVecBytes,
            (int) (vecs + sizeof vecs - vecTop));
#endif
    memset(tagBits, 0, sizeof tagBits);
    assert(vm != 0);
    gcMarker(*vm);
    gcSweeper();
    Vector::gcCompact();
#if GC_VERBOSE
    debugf("gc done,  free b: %7d obj + %7d vec (max %d+%d)\n",
            (int) (GC_MEM_BYTES - currObjBytes),
            (int) (sizeof vecs - currVecBytes),
            MEM_BYTES, (int) sizeof vecs);
#endif
}
