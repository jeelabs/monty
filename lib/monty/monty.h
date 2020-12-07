// Monty, a stackless VM - main header

#include <cstdint>
#include <cstdlib>
#include <cstring>

extern "C" int printf (char const*, ...);
extern "C" int puts (char const*);
extern "C" int putchar (int);

// see gc.cpp - objects and vectors with garbage collection
namespace monty {
    struct Obj {
        virtual ~Obj () {}

        static auto inPool (void const* p) -> bool;
        auto isCollectable () const -> bool { return inPool(this); }

        auto operator new (size_t bytes) -> void*;
        auto operator new (size_t bytes, uint32_t extra) -> void* {
            return operator new (bytes + extra);
        }
        void operator delete (void*);
    protected:
        constexpr Obj () {} // only derived objects can be instantiated

        virtual void marker () const {} // called to mark all ref'd objects
        friend void mark (Obj const&);
    };

    struct Vec {
        constexpr Vec () {}
        constexpr Vec (void const* ptr, uint32_t num =0)
                    : data ((uint8_t*) ptr), capa (num) {}
        ~Vec () { (void) adj(0); }

        Vec (Vec const&) = delete;
        auto operator= (Vec const&) -> Vec& = delete;

        static auto inPool (void const* p) -> bool;
        auto isResizable () const -> bool {
            return data == nullptr || inPool(data);
        }

        auto ptr () const -> uint8_t* { return data; }
        auto cap () const -> uint32_t { return capa; }
        auto adj (uint32_t bytes) -> bool;

    private:
        uint8_t* data = nullptr; // pointer to vector when capa > 0
        uint32_t capa = 0; // capacity in bytes

        auto slots () const -> uint32_t; // capacity in vecslots
        auto findSpace (uint32_t) -> void*; // hidden private type
        friend void compact ();
    };

    void setup (void* base, uint32_t size); // configure the memory pool

    auto gcAvail () -> uint32_t; // return free space between objects & vectors
    auto gcCheck () -> bool;     // true when it's time to collect the garbage
    void gcNow ();               // first call Interp::markAll, then gcNow
    void gcObjDump ();           // like sweep, but only to print all obj+free
    void gcReport ();            // print a brief gc summary with statistics

    struct GCStats {
        uint32_t
            checks, sweeps, compacts,
            toa, tob, tva, tvb, // totalObjAllocs/Bytes, totalVecAllocs/Bytes
            coa, cob, cva, cvb, // currObjAllocs/Bytes,  currVecAllocs/Bytes
            moa, mob, mva, mvb; // maxObjAllocs/Bytes,   maxVecAllocs/Bytes
    };
    extern GCStats gcStats;

    inline void mark (Obj const* p) { if (p != nullptr) mark(*p); }
    void mark (Obj const&);
    void sweep ();   // reclaim all unmarked objects
    void compact (); // reclaim and compact unused vector space

    extern void* (*panicOutOfMemory)(); // triggers an assertion by default
}
