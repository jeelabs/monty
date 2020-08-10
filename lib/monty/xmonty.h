namespace Monty {

    extern "C" int printf (const char*, ...);

// see gc.cpp - objects and vectors with garbage collection

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
            : size (len/sizeof (void*)/2), data ((uint8_t*) ptr) {}
        ~Vec () { (void) adj(0); }

        Vec (Vec const&) = delete;
        Vec& operator= (Vec const&) = delete;
        // TODO Vec (Vec&& v); Vec& operator= (Vec&& v);

        auto isResizable () const -> bool;

        auto ptr () const -> uint8_t* { return data; }
        auto cap () const -> size_t;
        auto adj (size_t bytes) -> bool;

    private:
        uint32_t size;  // capacity in slots, see cap() TODO in bytes
        uint8_t* data;  // points into memory pool when cap() > 0

        auto findSpace (size_t) -> void*; // hidden private type
        friend void compact ();
    };

    void setup (uintptr_t* base, size_t bytes); // configure the memory pool

    template< size_t N > // convenience wrapper
    void setup (uintptr_t (&array)[N]) { setup(array, sizeof array); }

    auto avail () -> size_t; // free bytes between the object & vector areas

    inline void mark (Obj const* p) { if (p != nullptr) mark(*p); }
    void mark (Obj const&);
    void sweep ();   // reclaim all unmarked objects
    void compact (); // reclaim and compact unused vector space

    extern void (*panicOutOfMemory)(); // triggers an assertion by default

// see chunk.cpp - typed and chunked access to vectors

    struct Value; // forward decl

    template< typename T >
    struct VecOf : private Vec {
        auto ptr () const -> T* { return (T*) Vec::ptr(); }
        auto cap () const -> size_t { return Vec::cap() / sizeof (T); }
        auto adj (size_t num) -> bool { return Vec::adj(num * sizeof (T)); }

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

        template< typename T >
        auto asVecOf () const -> VecOf<T>& { return (VecOf<T>&) vec; }

    protected:
        size_t off {0};   // starting offset, in typed units
        size_t len {~0U}; // maximum length, in typed units
    private:
        Vec& vec;         // parent vector
    };

    template< typename T >
    struct ChunkOf : Chunk {
        using Chunk::off; // make public
        using Chunk::len; // make public

        constexpr ChunkOf (Vec& v) : Chunk (v) {}
        constexpr ChunkOf (VecOf<T>& v) : Chunk ((Vec&) v) {}

        auto asVec () const -> VecOf<T>& { return asVecOf<T>(); }

        auto length () const -> size_t {
            auto n = asVec().cap() - off;
            return (int) n < 0 ? 0 : n < len ? n : len;
        }

        auto operator[] (size_t idx) const -> T& {
            // assert(idx < length());
            return asVec()[off+idx];
        }

        auto begin () const -> T const* { return &asVec()[0]; }
        auto end () const -> T const* { return begin() + length(); }

        auto begin () -> T* { return &asVec()[0]; }
        auto end () -> T* { return begin() + length(); }

        void insert (size_t idx, size_t num =1) {
            auto& vot = asVec();
            if (len > vot.cap())
                len = vot.cap();
            if (idx > len) {
                num += idx - len;
                idx = len;
            }
            auto need = off + len + num;
            if (need > vot.cap())
                vot.adj(need);
            vot.move(off + idx, len - idx, num);
            vot.wipe(off + idx, num);
            len += num;
        }

        void remove (size_t idx, size_t num =1) {
            auto& vot = asVec();
            if (len > vot.cap())
                len = vot.cap();
            if (idx >= len)
                return;
            if (num > len - idx)
                num = len - idx;
            vot.move(off + idx + num, len - (idx + num), -num);
            len -= num;
        }
    };

    struct Segment : Chunk {
        static auto make (char type, Vec& vec) -> Segment;

        using Chunk::Chunk;
        virtual ~Segment () {}

        operator Value () const;
        Segment& operator= (Value);

        virtual auto typ () const -> char;
        virtual auto len () const -> size_t;
        virtual auto get (int idx) const -> Value;
        virtual void set (int idx, Value val);
        virtual void ins (size_t idx, size_t num =1);
        virtual void del (size_t idx, size_t num =1);
    };

    template< char C, typename T >
    struct SegmentOf : Segment {
        using Segment::Segment;

        auto typ () const -> char override { return C; }
        auto len () const -> size_t override { return ccot().length(); }
        auto get (int i) const -> Value override;
        void set (int i, Value v) override;
        void ins (size_t i, size_t n =1) override { cot().insert(i, n); }
        void del (size_t i, size_t n =1) override { cot().remove(i, n); }

    private:
        auto cot () -> ChunkOf<T>& { return *(ChunkOf<T>*) this; }
        auto ccot () const -> ChunkOf<T> const& {
            return *(ChunkOf<T> const*) this;
        }
    };

    using CofV = ChunkOf<Value>;

    void mark (Segment const&);
    void mark (ChunkOf<Segment> const&);
    void mark (CofV const&);

// see type.cpp - basic object types and type system

    // forward decl's
    enum UnOp : uint8_t;
    enum BinOp : uint8_t;
    struct Callable;
    struct Object;
    struct Lookup;
    struct Buffer;
    struct Type;

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
        auto isInt () const -> bool { return (v&1) == Int; }
        auto isStr () const -> bool { return (v&3) == Str; }
        auto isObj () const -> bool { return (v&3) == Nil && v != 0; }

        inline auto isNull  () const -> bool;
        inline auto isFalse () const -> bool;
        inline auto isTrue  () const -> bool;
               auto isBool  () const -> bool { return isFalse() || isTrue(); }

        auto truthy () const -> bool;

        auto operator== (Value) const -> bool;
        auto operator< (Value) const -> bool;

        auto unOp (UnOp op) const -> Value;
        auto binOp (BinOp op, Value rhs) const -> Value;
        void dump (char const* msg =nullptr) const;

        static inline auto asBool (bool f) -> Value;
        auto invert () const -> Value { return asBool(!truthy()); }

    private:
        auto check (Type const& t) const -> bool;
        void verify (Type const& t) const;

        union {
            uintptr_t v;
            const void* p;
        };
    };

    inline void mark (Value v) { if (v.isObj()) mark(v.obj()); }

    extern Value const Null;
    extern Value const False;
    extern Value const True;
    extern Value const Empty; // Tuple

    auto Value::asBool (bool f) -> Value { return f ? True : False; }

    // define SegmentOf<C,T>'s get & set, now that Value type is complete
    template< char C, typename T >
    auto SegmentOf<C,T>::get (int i) const -> Value {
        return ccot()[i];
    }
    template< char C, typename T >
    void SegmentOf<C,T>::set (int i, Value v) {
        cot()[i] = v;
    }

    // can't use "CG3 type <object>", as type() is virtual iso override
    struct Object : Obj {
        static const Type info;
        virtual auto type () const -> Type const&;

        virtual auto repr  (Buffer&) const -> Value;
        virtual auto unop  (UnOp) const -> Value;
        virtual auto binop (BinOp, Value) const -> Value;
        virtual auto atget (Value) -> Value;
        virtual auto atset (Value, Value) -> Value;
    };

    //CG< type <none>
    struct None : Object {
        static Type const info;
        auto type () const -> Type const& override;
        auto repr (Buffer&) const -> Value override;
    //CG>

        auto unop (UnOp) const -> Value override;

        static None const nullObj;
    private:
        constexpr None () {} // can't construct more instances
    };

    //CG< type bool
    struct Bool : Object {
        static auto create (ChunkOf<Value> const&,Type const* =nullptr) -> Value;
        static Lookup const attrs;
        static Type const info;
        auto type () const -> Type const& override;
        auto repr (Buffer&) const -> Value override;
    //CG>

        auto unop (UnOp) const -> Value override;

        static Bool const trueObj;
        static Bool const falseObj;
    private:
        constexpr Bool () {} // can't construct more instances
    };

    //CG< type int
    struct Fixed : Object {
        static auto create (ChunkOf<Value> const&,Type const* =nullptr) -> Value;
        static Lookup const attrs;
        static Type const info;
        auto type () const -> Type const& override;
        auto repr (Buffer&) const -> Value override;
    //CG>

        constexpr Fixed (int64_t v) : i (v) {}

        operator int64_t () const { return i; }

        auto unop (UnOp) const -> Value override;
        auto binop (BinOp, Value) const -> Value override;

    private:
        int64_t i __attribute__((packed));
    }; // packing gives a better fit on 32b arch, and has no effect on 64b

    //CG< type bytes
    struct Bytes : Object {
        static auto create (ChunkOf<Value> const&,Type const* =nullptr) -> Value;
        static Lookup const attrs;
        static Type const info;
        auto type () const -> Type const& override;
        auto repr (Buffer&) const -> Value override;
    //CG>
        Bytes (uint8_t const* =nullptr, size_t =0) {} // TODO
    };

    //CG< type str
    struct Str : Bytes {
        static auto create (ChunkOf<Value> const&,Type const* =nullptr) -> Value;
        static Lookup const attrs;
        static Type const info;
        auto type () const -> Type const& override;
        auto repr (Buffer&) const -> Value override;
    //CG>
        Str (char const*) {} // TODO
    };

    //CG< type <iterator>
    struct Iter : Object {
        static Type const info;
        auto type () const -> Type const& override;
        auto repr (Buffer&) const -> Value override;
    //CG>

        Iter (Value arg) : seq (arg) {}

        //Value next () override;

    // TODO private:
        Value seq;
        size_t pos {0};
    };

    //CG< type range
    struct Range : Object {
        static auto create (ChunkOf<Value> const&,Type const* =nullptr) -> Value;
        static Lookup const attrs;
        static Type const info;
        auto type () const -> Type const& override;
        auto repr (Buffer&) const -> Value override;
    //CG>
    };

    //CG< type slice
    struct Slice : Object {
        static auto create (ChunkOf<Value> const&,Type const* =nullptr) -> Value;
        static Lookup const attrs;
        static Type const info;
        auto type () const -> Type const& override;
        auto repr (Buffer&) const -> Value override;
    //CG>
        Slice (Value a, Value b, Value c);

    private:
        int32_t off, num, step;
    };

    //CG< type <lookup>
    struct Lookup : Object {
        static Type const info;
        auto type () const -> Type const& override;
        auto repr (Buffer&) const -> Value override;
    //CG>

        struct Item { char const* k; Value v; };

        constexpr Lookup (Item const* p, size_t n) : items (p), count (n) {}

        auto operator[] (char const* key) -> Value;

        void marker () const override;
    protected:
        Item const* items;
        size_t count;
    };

    auto Value::isNull  () const -> bool { return &obj() == &None::nullObj; }
    auto Value::isFalse () const -> bool { return &obj() == &Bool::falseObj; }
    auto Value::isTrue  () const -> bool { return &obj() == &Bool::trueObj; }

// see repr.cpp - repr, printing, and buffering

    //CG< type <buffer>
    struct Buffer : Object {
        static Type const info;
        auto type () const -> Type const& override;
        auto repr (Buffer&) const -> Value override;
    //CG>

        void putc (char v) { write((uint8_t const*) &v, 1); }
        void puts (char const* s) { while (*s != 0) putc(*s++); }
        void print (const char* fmt, ...);

        auto operator<< (Value v) -> Buffer&;

    protected:
        virtual void write (uint8_t const* ptr, size_t len) const;
    private:
        int splitInt (uint32_t val, int base, uint8_t* buf);
        void putFiller (int n, char fill);
        void putInt (int val, int base, int width, char fill);

        bool sep {false};
    };

// see array.cpp - arrays, dicts, and other derived types

    //CG< type array
    struct Array : Object {
        static auto create (ChunkOf<Value> const&,Type const* =nullptr) -> Value;
        static Lookup const attrs;
        static Type const info;
        auto type () const -> Type const& override;
        auto repr (Buffer&) const -> Value override;
    //CG>

        Array (char type) : items (Segment::make(type, vec)) {}

        struct Proxy { Segment& seg; size_t idx;
            operator Value () const { return seg.get(idx); }
            Value operator= (Value v) { seg.set(idx, v); return v; }
        };

        auto operator[] (size_t idx) -> Proxy { return {items, idx}; }
        auto len () const -> size_t { return items.len(); }

        auto atget (Value k) -> Value override;
        auto atset (Value k, Value v) -> Value override;

        void marker () const override { mark(items); }
    protected:
        Vec vec;
        Segment items;
    };

    //CG< type tuple
    struct Tuple : Object {
        static auto create (ChunkOf<Value> const&,Type const* =nullptr) -> Value;
        static Lookup const attrs;
        static Type const info;
        auto type () const -> Type const& override;
        auto repr (Buffer&) const -> Value override;
    //CG>

        auto operator[] (size_t idx) const -> Value { return data()[idx]; }
        auto len () const -> size_t { return num; }

        auto begin () const -> Value const* { return data(); }
        auto end () const -> Value const* { return data() + num; }

        auto atget (Value k) -> Value override;

        static Tuple const emptyObj;
    private:
        Tuple (size_t n =0, Value const* vals =nullptr);

        auto data () const -> Value const* { return (Value const*) (this + 1); }

        size_t num;
    };

    //CG< type list
    struct List : Object {
        static auto create (ChunkOf<Value> const&,Type const* =nullptr) -> Value;
        static Lookup const attrs;
        static Type const info;
        auto type () const -> Type const& override;
        auto repr (Buffer&) const -> Value override;
    //CG>
        List () : List (0, nullptr) {}
        List (size_t n, Value const* vals);

        auto operator[] (size_t idx) -> Value& { return items[idx]; }
        auto len () const -> size_t { return items.length(); }

        void ins (size_t i, size_t n =1) { items.insert(i, n); }
        void del (size_t i, size_t n =1) { items.remove(i, n); }

        auto begin () -> Value* { return items.begin(); }
        auto end () -> Value* { return items.end(); }

        auto atget (Value k) -> Value override;
        auto atset (Value k, Value v) -> Value override;

        void marker () const override { mark(items); }

    protected:
        Vec vec;
        CofV items {vec};
    };

    //CG< type set
    struct Set : List {
        static auto create (ChunkOf<Value> const&,Type const* =nullptr) -> Value;
        static Lookup const attrs;
        static Type const info;
        auto type () const -> Type const& override;
        auto repr (Buffer&) const -> Value override;
    //CG>
        Set () : Set (0, nullptr) {}
        Set (size_t n, Value const* vals);

        auto find (Value v) const -> size_t;

        struct Proxy { Set& s; Value const v;
            operator bool () const;
            auto operator= (bool) -> bool;
        };

        // operator[] is problematic when the value is an int
        auto has (Value key) -> Proxy { return {*this, key}; }

        void ins (size_t i, size_t n =1) = delete;
        void del (size_t i, size_t n =1) = delete;

        auto atget (Value k) -> Value override;
        auto atset (Value k, Value v) -> Value override;
    };

    //CG< type dict
    struct Dict : Set {
        static auto create (ChunkOf<Value> const&,Type const* =nullptr) -> Value;
        static Lookup const attrs;
        static Type const info;
        auto type () const -> Type const& override;
        auto repr (Buffer&) const -> Value override;
    //CG>
        Dict (size_t n =0);

        struct Proxy { Dict& d; Value const k;
            operator Value () const;
            auto operator= (Value v) -> Value;
        };

        auto at (Value key) -> Proxy { return {*this, key}; }

        auto atget (Value k) -> Value override { return (*this)[k]; }
        auto atset (Value k, Value v) -> Value override { return (*this)[k] = v; }

        void marker () const override { Set::marker(); mark(chain); }
    protected:
        Object* chain {nullptr};
    };

    //CG< type type
    struct Type : Dict {
        static auto create (ChunkOf<Value> const&,Type const* =nullptr) -> Value;
        static Lookup const attrs;
        static Type const info;
        auto type () const -> Type const& override;
        auto repr (Buffer&) const -> Value override;
    //CG>

        using Factory = auto (*)(CofV const&, Type const*) -> Value;

        Type (char const* s, Factory f =noFactory, Lookup const* =nullptr)
            : name (s), factory (f) { /* TODO chain = a; */ }

        //auto call (int ac, CofV const& av) const -> Value override;
        //auto attr (char const*, Value&) const -> Value override;

        char const* const name;
        Factory const factory;

    private:
        static auto noFactory (CofV const&,Type const*) -> Value;
    };

    //CG< type class
    struct Class : Type {
        static auto create (ChunkOf<Value> const&,Type const* =nullptr) -> Value;
        static Lookup const attrs;
        static Type const info;
        auto type () const -> Type const& override;
        auto repr (Buffer&) const -> Value override;
    //CG>

        Class () : Type (nullptr) {} // TODO
    };

    // can't use CG, because type() must not be auto-generated
    struct Inst : Dict {
        static auto create (CofV const&,Type const*) -> Value;
        static Lookup const attrs;
        static Type const info;
        auto type () const -> Type const& override;
        auto repr (Buffer&) const -> Value override;

        Inst () {}
    };

// see stack.cpp - execution state, stacks, and callables

    //CG< type <function>
    struct Function : Object {
        static Type const info;
        auto type () const -> Type const& override;
        auto repr (Buffer&) const -> Value override;
    //CG>
        using Prim = auto (*)(int,CofV const&) -> Value;

        constexpr Function (Prim f) : func (f) {}

        //auto call (int ac, CofV const& av) const -> Value override {
        //    return func(ac, av);
        //}

    protected:
        Prim func;
    };

    //CG< type <boundmeth>
    struct BoundMeth : Object {
        static Type const info;
        auto type () const -> Type const& override;
        auto repr (Buffer&) const -> Value override;
    //CG>

        constexpr BoundMeth (Value f, Value o) : meth (f), self (o) {}

        //auto call (int ac, CofV const& av) const -> Value override;

        void marker () const override { mark(meth); mark(self); }
    protected:
        Value meth;
        Value self;
    };

    //CG< type <context>
    struct Context : Object {
        static Type const info;
        auto type () const -> Type const& override;
        auto repr (Buffer&) const -> Value override;
    //CG>

        enum Reg { Link, Sp, Ip, Ep, Code, Locals, Globals, Result, Extra };

        Context () {}

        void push (Callable const&);
        void pop ();

        auto ipBase () const -> uint8_t const*;
        auto fastSlot (size_t) -> Value&;

        auto asDict (Reg) -> Dict&;
        auto locals () -> Dict& { return asDict(Locals); }
        auto globals () -> Dict& { return asDict(Globals); }
        auto asArgs (size_t len, Value const* ptr =nullptr) const -> CofV;

        void marker () const override { mark(stack); } // TODO mark more items!

        CofV stack {vec};
    private:
        Vec vec;
    };

// see exec.cpp - importing, loading, and bytecode execution

    extern volatile uint32_t pending; // used for irq-safe inner loop exit bits

    //CG< type <module>
    struct Module : Dict {
        static Type const info;
        auto type () const -> Type const& override;
        auto repr (Buffer&) const -> Value override;
    //CG>

        Module (Value qpool) : qp (qpool) {}

        void marker () const override { Dict::marker(); mark(qp); }

        Value qp;
    };

    //CG< type <callable>
    struct Callable : Object {
        static Type const info;
        auto type () const -> Type const& override;
        auto repr (Buffer&) const -> Value override;
    //CG>

        Callable (Module& mod, Value callee) : mo (mod), bc (callee) {}

        auto frameSize () const -> size_t;
        auto isGenerator () const -> bool;
        auto hasVarArgs () const -> bool;
        auto codeStart () const -> uint8_t const*;

        //auto call (int ac, CofV const& av) const -> Value override;

        void marker () const override {
            mark(mo); mark(bc); mark(pos); mark(kw);
        }

    // TODO private:
        Module& mo;
        Value bc; // don't expose actual type
        Tuple* pos {nullptr};
        Dict* kw {nullptr};
    };

    auto loadModule (uint8_t const* addr) -> Module*;

} // namespace Monty
