// Objects and vectors with garbage collection, public header.
namespace Monty {

    struct Obj {
        virtual ~Obj () {}

        bool inObjPool () const;

        void* operator new (size_t sz);
        void* operator new (size_t sz, size_t n) { return operator new (sz+n); }
        void operator delete (void* p);
        protected:
        virtual void mark () const {}
        friend void mark (const Obj&); // i.e. Monty::mark
    };

    struct Vec {
        Vec (int i =0) : data (0), info (i), capa (0) {}
        ~Vec () { resize(0); }

        uint8_t* ptr () const { return data; }
        size_t cap () const {
            return capa > 0 ? (2 * capa - 1) * sizeof (void*) : 0;
        }

        void resize (size_t sz);

        protected:
        uint8_t* data;
        uint32_t info :8;
        uint32_t capa :24;
    };

    void init (uintptr_t* base, size_t size);
    size_t avail ();

    inline void mark (const Obj* p) { if (p != 0) mark(*p); }
    void mark (const Obj& obj);
    void sweep();
    void compact();

} // namespace Monty
