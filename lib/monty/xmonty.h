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
        Obj () {} // only derived objects can be instantiated

        virtual void marker () const {} // called to mark all ref'd objects
        friend void mark (Obj const&);
    };

    struct Vec {
        Vec () : info (0), caps (0), data (0) {}
        ~Vec () { resize(0); }

        auto ptr () const -> uint8_t* { return data; }
        auto cap () const -> size_t;
        auto resize (size_t bytes) -> bool;

    protected:
        uint32_t info :8;   // for use in derived classes
    private:
        uint32_t caps :24;  // capacity in slots, see cap()
        uint8_t* data;      // points into memory pool when cap() > 0

        auto findSpace (size_t) -> void*; // hidden private type
        friend void compact ();
    };

    void setup (uintptr_t* base, size_t bytes); // configure the memory pool
    auto avail () -> size_t; // free bytes between the object & vector areas

    inline void mark (Obj const* p) { if (p != 0) mark(*p); }
    void mark (Obj const&);
    void sweep ();   // reclaim all unmarked objects
    void compact (); // reclaim and compact unused vector space

    extern void (*panicOutOfMemory)(); // triggers an assertion by default

} // namespace Monty

// see type.cpp - basic object types and type system
namespace Monty {

    struct Object; // forward decl
    struct TypeObj; // forward decl
    struct LookupObj; // forward decl
    enum class UnOp : uint8_t;
    enum class BinOp : uint8_t;

    struct Value {
        enum Tag { Nil, Int, Str, Obj };

        Value ()                  : v (0) {}
        Value (int arg)           : v ((arg << 1) | 1) {}
        Value (char const* arg)   : v (((uintptr_t) arg << 2) | 2) {}
        Value (Object const* arg) : v ((uintptr_t) arg) {}
        Value (Object const& arg) : v ((uintptr_t) &arg) {}

        operator int () const { return (intptr_t) v >> 1; }
        operator char const* () const { return (char const*) (v >> 2); }
        auto obj () const -> Object& { return *(Object*) v; }
        //inline auto objPtr () const -> ForceObj;

        template< typename T > // return null pointer if not of required type
        auto ifType () const -> T* { return check(T::info) ? (T*) &obj() : 0; }

        template< typename T > // type-asserted safe cast via Object::type()
        auto asType () const -> T& { verify(T::info); return *(T*) &obj(); }

        auto tag () const -> enum Tag {
            return (v & 1) ? Int : // bit 0 set
                    v == 0 ? Nil : // all bits 0
                (v & 2) ? Str : // bit 1 set, ptr shifted 2 up
                            Obj;  // bits 0 and 1 clear, ptr stored as is
        }

        auto id () const -> uintptr_t { return v; }

        auto isNil () const -> bool { return v == 0; }
        auto isInt () const -> bool { return v & 1; }
        auto isStr () const -> bool { return (v & 3) == 2; }
        auto isObj () const -> bool { return (v & 3) == 0 && v != 0; }

        inline auto isNone  () const -> bool;
        inline auto isFalse () const -> bool;
        inline auto isTrue  () const -> bool;
        auto isBool  () const -> bool { return isFalse() || isTrue(); }

        auto truthy () const -> bool;

        auto isEq (Value) const -> bool;
        auto unOp (UnOp op) const -> Value;
        auto binOp (BinOp op, Value rhs) const -> Value;
        void dump (char const* msg =0) const; // see builtin.h

        static auto asBool (int f) -> Value { return f ? True : False; }
        auto invert () const -> Value { return asBool(!truthy()); }

        static const Value None;
        static const Value False;
        static const Value True;
    private:
        auto check (TypeObj const& t) const -> bool;
        void verify (TypeObj const& t) const;

        uintptr_t v;
    };

    struct Object : Obj {
        static const TypeObj info;
        virtual auto type () const -> TypeObj const& =0;

        virtual auto unop  (UnOp) const -> Value =0;

        //void marker () const override {}
    };

    //CG3 type <none>
    struct NoneObj : Object {
        static const TypeObj info;
        const TypeObj& type () const override;

        //auto repr (BufferObj&) const -> Value override; // see builtin.h
        auto unop (UnOp) const -> Value override;

        static const NoneObj noneObj;
    private:
        NoneObj () {} // can't construct more instances
    };

    //CG< type bool
    struct BoolObj : Object {
        static Value create (const TypeObj&, int argc, Value argv[]);
        static const LookupObj attrs;
        static const TypeObj info;
        const TypeObj& type () const override;
    //CG>

        //auto repr (BufferObj&) const -> Value override; // see builtin.h
        auto unop (UnOp) const -> Value override;

        static const BoolObj trueObj;
        static const BoolObj falseObj;
    private:
        BoolObj () {} // can't construct more instances
    };

    auto Value::isNone  () const -> bool { return &obj() == &NoneObj::noneObj; }
    auto Value::isFalse () const -> bool { return &obj() == &BoolObj::falseObj; }
    auto Value::isTrue  () const -> bool { return &obj() == &BoolObj::trueObj; }

} // namespace Monty
