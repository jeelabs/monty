// Objects and vectors with garbage collection.
namespace Monty {

    struct Obj {
        virtual ~Obj () {}

        auto inObjPool () const -> bool;

        auto operator new (size_t sz) -> void*;
        auto operator new (size_t sz, size_t extra) -> void* {
            return operator new (sz + extra);
        }
        void operator delete (void* p);
    protected:
        virtual void mark () const {}
        friend void mark (Obj const&); // i.e. Monty::mark
    };

    struct Vec {
        Vec () : info (0), caps (0), data (0) {}
        ~Vec () { resize(0); }

        auto ptr () const -> uint8_t* { return data; }
        auto cap () const -> size_t;
        auto resize (size_t sz) -> bool;

    protected:
        uint32_t info :8;   // for use in derived classes
    private:
        uint32_t caps :24;  // capacity in slots, see cap()
        uint8_t* data;

        auto findSpace (size_t needs) -> void*; // hide private type
    };

    void init (uintptr_t* base, size_t size);
    auto avail () -> size_t;

    inline void mark (Obj const* p) { if (p != 0) mark(*p); }
    void mark (Obj const& obj);
    void sweep();
    void compact();

    extern void (*panicOutOfMemory)();

} // namespace Monty
