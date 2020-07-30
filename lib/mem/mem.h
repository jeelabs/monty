// objects and vectors with garbage collection

namespace Mem {

void init (uintptr_t* base, size_t size);
size_t avail ();
void sweep();

struct Obj {
    virtual ~Obj () {}

    bool isAllocated () const;

    void* operator new (size_t sz);
    void* operator new (size_t sz, size_t extra) {
        return operator new (sz + extra);
    }
    void operator delete (void* p);

    static void marker (const Obj* p) { if (p != 0) marker(*p); }
    static void marker (const Obj& obj);
protected:
    virtual void mark () const {}
};

} // namespace Mem
