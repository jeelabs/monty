// see mem.cpp - objects and vectors with garbage collection
namespace Monty {

    struct Obj {
        virtual ~Obj () {}

        auto isCollectable () const -> bool;

        auto operator new (size_t bytes) -> void*;
        auto operator new (size_t bytes, size_t extra) -> void* {
            return operator new (bytes + extra);
        }
        void operator delete (void*);
    protected:
        constexpr Obj () {} // only derived objects can be instantiated

        virtual void marker () const {} // called to mark all ref'd objects
        friend void mark (Obj const&);
    };

    struct Vec {
        constexpr Vec (void const* ptr =nullptr, size_t len =0) // TODO caps b
            : extra (0), caps (len/sizeof (void*)/2), data ((uint8_t*) ptr) {}
        ~Vec () { (void) resize(0); }

        auto isResizable () const -> bool;

        auto ptr () const -> uint8_t* { return data; }
        auto cap () const -> size_t;
        auto resize (size_t bytes) -> bool;

    protected:
        uint32_t extra :8;  // for use in derived classes TODO remove
    private:
        uint32_t caps :24;  // capacity in slots, see cap() TODO in bytes
        uint8_t* data;      // points into memory pool when cap() > 0

        auto findSpace (size_t) -> void*; // hidden private type
        friend void compact ();
    };

    void setup (uintptr_t* base, size_t bytes); // configure the memory pool
    auto avail () -> size_t; // free bytes between the object & vector areas

    inline void mark (Obj const* p) { if (p != nullptr) mark(*p); }
    void mark (Obj const&);
    void sweep ();   // reclaim all unmarked objects
    void compact (); // reclaim and compact unused vector space

    extern void (*panicOutOfMemory)(); // triggers an assertion by default

} // namespace Monty

// see chunk.cpp - typed and chunked access to vectors
namespace Monty {

    struct Array; // forward decl
    struct Value; // forward decl

    template< typename T >
    struct VecOf : Vec {
        auto ptr () const -> T* { return (T*) Vec::ptr(); }
        auto cap () const -> size_t { return Vec::cap() / sizeof (T); }

        auto operator[] (size_t idx) const -> T { return ptr()[idx]; }
        auto operator[] (size_t idx) -> T& { return ptr()[idx]; }

        void move (size_t pos, size_t num, int off) {
            memmove((void*) (ptr() + pos + off),
                        (void const*) (ptr() + pos), num * sizeof (T));
        }
        void wipe (size_t pos, size_t num) {
            memset((void*) (ptr() + pos), 0, num * sizeof (T));
        }
    };

    struct Chunk {
        Chunk (Vec& v) : vec (v) {}

        auto isValid () const -> bool { return (void*) &vec != nullptr; }

        auto asVec () const -> Vec& { return vec; }
        template< typename T >
        auto asVecOf () const -> VecOf<T>& { return (VecOf<T>&) vec; }

        Vec& vec;        // parent vector
        size_t off{0};   // starting offset
        size_t len{~0U}; // maximum length
    };

    template< typename T >
    struct ChunkOf : Chunk {
        using Chunk::Chunk; // re-use constructor

        auto length () const -> size_t {
            auto& vec = asVecOf<T>();
            auto n = vec.cap() - off;
            return n > len ? len : n >= 0 ? n : 0;
        }

        auto operator[] (size_t idx) const -> T& {
            // assert(idx < length());
            auto& vec = asVecOf<T>();
            return vec[off+idx];
        }

        void insert (size_t idx, size_t num =1) {
            // TODO deal with off != 0, and cap/resize
            if (idx > len) {
                num += idx - len;
                idx = len;
            }
            auto& vec = asVecOf<T>();
            vec.move(idx, len - idx, num);
            vec.wipe(idx, num);
            len += num;
        }

        void remove (size_t idx, size_t num =1) {
            // TODO deal with off != 0, and cap/resize
            if (idx >= len)
                return;
            if (num > len - idx)
                num = len - idx;
            auto& vec = asVecOf<T>();
            vec.move(idx + num, len - (idx + num), -num);
            len -= num;
        }
    };

    struct Segment : Chunk {
        static auto create (char type, Vec& vec) -> Segment&;

        using Chunk::Chunk; // re-use constructor
        virtual ~Segment () {}

        operator Value () const;
        Segment& operator= (Value);

        virtual auto typ () const -> char;
        virtual auto vec () const -> Vec&;

        virtual auto get (int idx) const -> Value;
        virtual void set (int idx, Value val);
        virtual void ins (size_t idx, size_t num =1);
        virtual void del (size_t idx, size_t num =1);
    };

    void mark (Segment const&);
    void mark (ChunkOf<Segment> const&);
    void mark (ChunkOf<Value> const&);

} // namespace Monty

// see type.cpp - basic object types and type system
namespace Monty {

    // forward decl's
    enum class UnOp : uint8_t;
    enum class BinOp : uint8_t;
    struct Object;
    struct Type;
    struct Lookup;

    // TODO keep these aliases until codegen.py has been updated
    using LookupObj = Lookup;
    using TypeObj = Type;

    struct Value {
        enum Tag { Nil, Int, Str, Obj };

        constexpr Value () : v (0) {}
                  Value (int arg);
                  Value (char const* arg);
        constexpr Value (Object const* arg) : p (arg) {} // TODO keep?
        constexpr Value (Object const& arg) : p (&arg) {}

        operator int () const { return (intptr_t) v >> 1; }
        operator char const* () const { return (char const*) (v >> 2); }
        auto obj () const -> Object& { return *(Object*) v; }
        // TODO inline auto objPtr () const -> ForceObj;

        template< typename T > // return null pointer if not of required type
        auto ifType () const -> T* { return check(T::info) ? (T*) &obj() : 0; }

        template< typename T > // type-asserted safe cast via Object::type()
        auto asType () const -> T& { verify(T::info); return *(T*) &obj(); }

        auto tag () const -> Tag {
            return (v&1) != 0 ? Int : // bit 0 set
                       v == 0 ? Nil : // all bits 0
                   (v&2) != 0 ? Str : // bit 1 set, ptr shifted 2 up
                                Obj;  // bits 0 and 1 clear, ptr stored as is
        }

        auto id () const -> uintptr_t { return v; }

        auto isNil () const -> bool { return v == 0; }
        auto isInt () const -> bool { return (v&1) != 0; }
        auto isStr () const -> bool { return (v&3) == 2; }
        auto isObj () const -> bool { return (v&3) == 0 && v != 0; }

        inline auto isNone  () const -> bool;
        inline auto isFalse () const -> bool;
        inline auto isTrue  () const -> bool;
               auto isBool  () const -> bool { return isFalse() || isTrue(); }

        auto truthy () const -> bool;

        auto isEq (Value) const -> bool;
        auto unOp (UnOp op) const -> Value;
        auto binOp (BinOp op, Value rhs) const -> Value;
        void dump (char const* msg =nullptr) const; // see builtin.h

        static auto asBool (bool f) -> Value { return f ? True : False; }
        auto invert () const -> Value { return asBool(!truthy()); }

        static Value const None;
        static Value const False;
        static Value const True;
    private:
        auto check (Type const& t) const -> bool;
        void verify (Type const& t) const;

        union {
            uintptr_t v;
            const void* p;
        };
    };

    // can't use "CG3 type <object>", as type() is virtual iso override
    struct Object : Obj {
        static const Type info;
        virtual auto type () const -> Type const&;

        // virtual auto repr (BufferObj&) const -> Value; // see builtin.h
        virtual auto unop  (UnOp) const -> Value;
        virtual auto binop (BinOp, Value) const -> Value;
    };

    //CG3 type <none>
    struct None : Object {
        static const TypeObj info;
        const TypeObj& type () const override;

        //auto repr (BufferObj&) const -> Value override; // see builtin.h
        auto unop (UnOp) const -> Value override;

        static None const noneObj;
    private:
        constexpr None () {} // can't construct more instances
    };

    //CG< type bool
    struct Bool : Object {
        static Value create (const TypeObj&, int argc, Value argv[]);
        static const LookupObj attrs;
        static const TypeObj info;
        const TypeObj& type () const override;
    //CG>

        //auto repr (BufferObj&) const -> Value override; // see builtin.h
        auto unop (UnOp) const -> Value override;

        static Bool const trueObj;
        static Bool const falseObj;
    private:
        constexpr Bool () {} // can't construct more instances
    };

    //CG< type int
    struct Long : Object {
        static Value create (const TypeObj&, int argc, Value argv[]);
        static const LookupObj attrs;
        static const TypeObj info;
        const TypeObj& type () const override;
    //CG>

        constexpr Long (int64_t v) : i (v) {}

        operator int64_t () const { return i; }

        //auto repr (BufferObj&) const -> Value override; // see builtin.h
        auto unop (UnOp) const -> Value override;

    private:
        int64_t i __attribute__((packed));
    }; // packing gives a better fit on 32b arch, and has no effect on 64b

    //CG< type type
    struct Type : Object {
        static Value create (const TypeObj&, int argc, Value argv[]);
        static const LookupObj attrs;
        static const TypeObj info;
        const TypeObj& type () const override;
    //CG>

        typedef auto (*Factory)(Type const&,int,Value[]) -> Value;

        char const* name;
        Factory const factory;

        constexpr Type (char const* s, Factory f =noFactory, Lookup const* =nullptr)
            : name (s), factory (f) { /* TODO chain = a; */ }

        //auto call (ChunkOf<Value>& args) const -> Value override;
        //auto attr (char const*, Value&) const -> Value override;

    private:
        static auto noFactory (Type const&,int,Value[]) -> Value;
    };

    auto Value::isNone  () const -> bool { return &obj() == &None::noneObj; }
    auto Value::isFalse () const -> bool { return &obj() == &Bool::falseObj; }
    auto Value::isTrue  () const -> bool { return &obj() == &Bool::trueObj; }

} // namespace Monty

// see array.cpp - vectors, arrays, and other derived types
namespace Monty {

#if 1 // TODO obsolete
    struct Vector : private Vec {
        Vector (size_t bits);

        auto length () const -> size_t { return fill; }
        int width () const { auto b = 1<<extra; return b < 8 ? -b : b/8; }
        size_t widthOf (int num) const { return ((num << extra) + 7) >> 3; }

        auto getPtr (int idx) const -> uint8_t*;
        auto getInt (int idx) const -> int;
        auto getIntU (int idx) const -> uint32_t;

        void set (int idx, void const* ptr);
        void set (int idx, int val);

        void ins (size_t idx, int num =1);
        void del (size_t idx, int num =1);

    protected:
        uint32_t fill{0}; // in elements
    };
#endif

    //CG< type array
    struct Array : Object {
        static Value create (const TypeObj&, int argc, Value argv[]);
        static const LookupObj attrs;
        static const TypeObj info;
        const TypeObj& type () const override;
    //CG>

        Array (char type) : segment (Segment::create(type, vec)) {}

        struct Proxy { Segment& s; size_t i;
            operator Value () const { return s.get(i); }
            Value operator= (Value v) { s.set(i, v); return v; }
        };

        auto operator[] (size_t idx) const -> Value { return segment.get(idx); }
        auto operator[] (size_t idx) -> Proxy { return Proxy{segment, idx}; }

        void marker () const override { mark(segment); }

    private:
        Vec vec;
        Segment& segment;
        friend struct Segment;
    };

    //CG< type list
    struct List : Object {
        static Value create (const TypeObj&, int argc, Value argv[]);
        static const LookupObj attrs;
        static const TypeObj info;
        const TypeObj& type () const override;
    //CG>

        List () : items (Segment::create('V', vec)) {}

        void marker () const override { mark(items); }

    private:
        VecOf<Value> vec;
        Segment& items;
    };

    //CG< type dict
    struct Dict : Object {
        static Value create (const TypeObj&, int argc, Value argv[]);
        static const LookupObj attrs;
        static const TypeObj info;
        const TypeObj& type () const override;
    //CG>

        Dict () : keys (Segment::create('V', vec)),
                  vals (Segment::create('V', vec)) {}

        void marker () const override { mark(keys); mark(vals); }

    private:
        VecOf<Value> vec;
        Segment& keys;
        Segment& vals;
    };

} // namespace Monty
