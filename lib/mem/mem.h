// objects and vectors with garbage collection

namespace Mem {

struct Obj {
    virtual ~Obj () {}

    bool isAllocated () const;

    void* operator new (size_t sz);
    void* operator new (size_t sz, size_t extra) {
        return operator new (sz + extra);
    }
    void operator delete (void* p);
protected:
    virtual void mark () const {}
    friend void mark (const Obj&); // i.e. Mem::mark
};

void init (uintptr_t* base, size_t size);
size_t avail ();

inline void mark (const Obj* p) { if (p != 0) mark(*p); }
void mark (const Obj& obj);
void sweep();

} // namespace Mem
