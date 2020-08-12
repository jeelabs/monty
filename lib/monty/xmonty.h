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
        constexpr Vec () : caps (0), data (nullptr) {}
        constexpr Vec (void const* ptr, size_t len =0) // TODO caps b
            : caps (len/sizeof (void*)/2), data ((uint8_t*) ptr) {}
        ~Vec () { (void) adj(0); }

        Vec (Vec const&) = delete;
        auto operator= (Vec const&) -> Vec& = delete;
        // TODO Vec (Vec&& v); auto operator= (Vec&& v) -> Vec&;

        auto isResizable () const -> bool;

        auto ptr () const -> uint8_t* { return data; }
        auto cap () const -> size_t;
        auto adj (size_t bytes) -> bool;

    private:
        uint32_t caps; // capacity in slots, see cap() TODO in bytes
        uint8_t* data; // points into memory pool when cap() > 0

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

// see type.cpp - basic object types and type system

    // forward decl's
    enum UnOp : uint8_t;
    enum BinOp : uint8_t;
    struct Callable;
    struct Object;
    struct Lookup;
    struct Buffer;
    struct Type;
    struct Context;

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

    template< typename T >
    struct VecOf : private Vec {
        using Vec::Vec;

        auto cap () const -> size_t { return Vec::cap() / sizeof (T); }
        auto adj (size_t num) -> bool { return Vec::adj(num * sizeof (T)); }

        auto size () const -> size_t { return fill; }
        auto begin () const -> T* { return (T*) Vec::ptr(); }
        auto end () const -> T* { return begin() + fill; }
        auto operator[] (size_t idx) const -> T& { return begin()[idx]; }

        void move (size_t pos, size_t num, int off) {
            memmove((void*) (begin() + pos + off),
                        (void const*) (begin() + pos), num * sizeof (T));
        }
        void wipe (size_t pos, size_t num) {
            memset((void*) (begin() + pos), 0, num * sizeof (T));
        }

        void insert (size_t idx, size_t num =1) {
            if (fill > cap())
                fill = cap();
            if (idx > fill) {
                num += idx - fill;
                idx = fill;
            }
            auto need = fill + num;
            if (need > cap())
                adj(need);
            move(idx, fill - idx, num);
            wipe(idx, num);
            fill += num;
        }

        void remove (size_t idx, size_t num =1) {
            if (fill > cap())
                fill = cap();
            if (idx >= fill)
                return;
            if (num > fill - idx)
                num = fill - idx;
            move(idx + num, fill - (idx + num), -num);
            fill -= num;
        }

        uint32_t fill {0};
    };

    using Vector = VecOf<Value>;

    void mark (Vector const&);

    struct Chunk {
        constexpr Chunk (Vector& v) : vec (v) {}

        auto size () const -> size_t {
            auto n = vec.cap() - off;
            return (int) n < 0 ? 0 : n < len ? n : len;
        }
        auto begin () const -> Value* { return vec.begin() + off; }
        auto end () const -> Value* { return begin() + size(); }
        auto operator[] (size_t idx) const -> Value& { return begin()[idx]; }

        size_t len {~0U}; // maximum length, in typed units
        size_t off {0};   // starting offset, in typed units
        Vector& vec;      // parent vector
    };

    // can't use "CG type <object>", as type/repr are virtual iso override
    struct Object : Obj {
        static const Type info;
        virtual auto type () const -> Type const&;
        virtual auto repr  (Buffer&) const -> Value;

        virtual auto call  (Context&, int, int) const -> Value;
        virtual auto unop  (UnOp) const -> Value;
        virtual auto binop (BinOp, Value) const -> Value;
        virtual auto attr  (const char*, Value&) const -> Value;
        virtual auto getAt (Value) const -> Value;
        virtual auto setAt (Value, Value) -> Value;
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
        static auto create (Chunk const&,Type const* =nullptr) -> Value;
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
        static auto create (Chunk const&,Type const* =nullptr) -> Value;
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
        static auto create (Chunk const&,Type const* =nullptr) -> Value;
        static Lookup const attrs;
        static Type const info;
        auto type () const -> Type const& override;
        auto repr (Buffer&) const -> Value override;
    //CG>
        Bytes (uint8_t const* =nullptr, size_t =0) {} // TODO
    };

    //CG< type str
    struct Str : Bytes {
        static auto create (Chunk const&,Type const* =nullptr) -> Value;
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

        void marker () const override { mark(seq); }
    // TODO private:
        Value seq;
        size_t pos {0};
    };

    //CG< type range
    struct Range : Object {
        static auto create (Chunk const&,Type const* =nullptr) -> Value;
        static Lookup const attrs;
        static Type const info;
        auto type () const -> Type const& override;
        auto repr (Buffer&) const -> Value override;
    //CG>
    };

    //CG< type slice
    struct Slice : Object {
        static auto create (Chunk const&,Type const* =nullptr) -> Value;
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

        constexpr Lookup (Item const* p, size_t sz)
            : items (p), count (sz / sizeof (Item)) {}

        auto operator[] (char const* key) const -> Value;

        auto getAt (Value k) const -> Value override;

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

    //CG< type tuple
    struct Tuple : Object {
        static auto create (Chunk const&,Type const* =nullptr) -> Value;
        static Lookup const attrs;
        static Type const info;
        auto type () const -> Type const& override;
        auto repr (Buffer&) const -> Value override;
    //CG>
        auto size () const -> size_t { return fill; }
        auto begin () const -> Value const* { return data(); }
        auto end () const -> Value const* { return begin() + size(); }
        auto operator[] (size_t idx) const -> Value { return begin()[idx]; }

        auto getAt (Value k) const -> Value override;

        size_t const fill;

        static Tuple const emptyObj;
    private:
        constexpr Tuple () : fill (0) {}
        Tuple (size_t n, Value const* vals =nullptr);

        auto data () const -> Value const* { return (Value const*) (this + 1); }
    };

    //CG< type list
    struct List : Object, Vector {
        static auto create (Chunk const&,Type const* =nullptr) -> Value;
        static Lookup const attrs;
        static Type const info;
        auto type () const -> Type const& override;
        auto repr (Buffer&) const -> Value override;
    //CG>
        constexpr List () {}
        List (size_t n, Value const* vals);

        auto getAt (Value k) const -> Value override;
        auto setAt (Value k, Value v) -> Value override;

        void marker () const override { mark((Vector const&) *this); }
    };

    //CG< type set
    struct Set : List {
        static auto create (Chunk const&,Type const* =nullptr) -> Value;
        static Lookup const attrs;
        static Type const info;
        auto type () const -> Type const& override;
        auto repr (Buffer&) const -> Value override;
    //CG>
        using List::List;

        auto find (Value v) const -> size_t;

        struct Proxy { Set& s; Value v;
            operator bool () const { return ((Set const&) s).has(v); }
            auto operator= (bool) -> bool;
        };

        // operator[] is problematic when the value is an int
        auto has (Value key) const -> bool;
        auto has (Value key) -> Proxy { return {*this, key}; }

        auto getAt (Value k) const -> Value override;
        auto setAt (Value k, Value v) -> Value override;
    };

    //CG< type dict
    struct Dict : Set {
        static auto create (Chunk const&,Type const* =nullptr) -> Value;
        static Lookup const attrs;
        static Type const info;
        auto type () const -> Type const& override;
        auto repr (Buffer&) const -> Value override;
    //CG>
        constexpr Dict (Object const* ch =nullptr) : chain (ch) {}
        Dict (size_t n);

        struct Proxy { Dict& d; Value k;
            operator Value () const { return ((Dict const&) d).at(k); }
            auto operator= (Value v) -> Value;
        };

        auto at (Value key) const -> Value;
        auto at (Value key) -> Proxy { return {*this, key}; }

        auto getAt (Value k) const -> Value override { return at(k); }
        auto setAt (Value k, Value v) -> Value override { return at(k) = v; }

        void marker () const override { Set::marker(); mark(chain); }
    // TODO protected:
        Object const* chain {nullptr};
    };

    //CG< type type
    struct Type : Dict {
        static auto create (Chunk const&,Type const* =nullptr) -> Value;
        static Lookup const attrs;
        static Type const info;
        auto type () const -> Type const& override;
        auto repr (Buffer&) const -> Value override;
    //CG>
        using Factory = auto (*)(Chunk const&, Type const*) -> Value;

        constexpr Type (char const* s, Factory f =noFactory,
                                        Lookup const* a =nullptr)
            : Dict (a), name (s), factory (f) {}

        auto call (Context& ctx, int argc, int args) const -> Value override;
        auto attr (char const* name, Value& self) const -> Value override {
            return getAt(name);
        }

        char const* name;
        Factory factory;
    private:
        static auto noFactory (Chunk const&,Type const*) -> Value;
    };

    //CG< type class
    struct Class : Type {
        static auto create (Chunk const&,Type const* =nullptr) -> Value;
        static Lookup const attrs;
        static Type const info;
        auto type () const -> Type const& override;
        auto repr (Buffer&) const -> Value override;
    //CG>
        Class () : Type (nullptr) {} // TODO
    };

    // can't use CG, because type() must not be auto-generated
    struct Inst : Dict {
        static auto create (Chunk const&,Type const*) -> Value;
        static Lookup const attrs;
        static Type const info;
        auto type () const -> Type const& override;
        auto repr (Buffer&) const -> Value override;

        Inst () {}
    };

    extern Lookup const builtins;

// see stack.cpp - execution state, stacks, and callables

    //CG< type <function>
    struct Function : Object {
        static Type const info;
        auto type () const -> Type const& override;
        auto repr (Buffer&) const -> Value override;
    //CG>
        using Prim = auto (*)(Context&,int,int) -> Value;

        constexpr Function (Prim f) : func (f) {}

        auto call (Context& ctx, int argc, int args) const -> Value override {
            return func(ctx, argc, args);
        }

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

        //auto call (int ac, Chunk const& av) const -> Value override;

        void marker () const override { mark(meth); mark(self); }
    protected:
        Value meth;
        Value self;
    };

// see exec.cpp - importing, loading, and bytecode execution

    //CG< type <module>
    struct Module : Dict {
        static Type const info;
        auto type () const -> Type const& override;
        auto repr (Buffer&) const -> Value override;
    //CG>
        Module (Value qpool) : Dict (&builtins), qp (qpool) {}

        Value attr (const char* s, Value&) const override { return getAt(s); }

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

        auto qStrAt (size_t) const -> char const*;
        auto constAt (size_t i) const -> Value;

        auto fastSlotTop () const -> size_t;
        auto excDepth () const -> size_t;
        auto isGenerator () const -> bool;
        auto hasVarArgs () const -> bool;
        auto codeStart () const -> uint8_t const*;

        //auto call (int ac, Chunk const& av) const -> Value override;

        void marker () const override {
            mo.marker(); mark(bc); mark(pos); mark(kw);
        }

    // TODO private:
        Module& mo;
        Value bc; // don't expose actual type
        Tuple* pos {nullptr};
        Dict* kw {nullptr};
    };

    //CG< type <context>
    struct Context : List {
        static Type const info;
        auto type () const -> Type const& override;
        auto repr (Buffer&) const -> Value override;
    //CG>
        struct Frame {
            Value link, sp, ip, ep, code, locals, result, stack [];
        };

        Context () { insert(0, NumSlots); slot(Limit) = NumSlots; }

        enum Slot { Limit, Event, Caller, NumSlots };
        auto slot (Slot s) const -> Value& { return begin()[s]; }

        auto frame () const -> Frame& { return *(Frame*) end(); }

        void enter (Callable const&, Chunk const&, Dict const* =nullptr);
        Value leave (Value v);

        auto getQstr (size_t i) const -> char const* {
            return callee().qStrAt(i);
        }
        auto getConst (size_t i) const -> Value {
            return callee().constAt(i);
        }
        auto fastSlot (size_t i) const -> Value& {
            return spBase()[callee().fastSlotTop() + ~i];
        }
        auto spBase () const -> Value* {
            return frame().stack;
        }
        auto ipBase () const -> uint8_t const* {
            return callee().codeStart();
        }

        static constexpr int EXC_STEP = 3; // use 3 slots per exception
        auto excBase (int incr =0) const -> Value*;

        auto locals () const -> Object& { return frame().locals.obj(); }
        auto globals () const -> Object& { return callee().mo; }
        auto asArgs (size_t len, Value const* ptr =nullptr) -> Chunk;

        void raise (Value exc ={}) const;
        void raise (uint8_t op, uint16_t arg) const {
            raise((~0U << 24) | (op << 16) | arg);
        }

    private:
        auto callee () const -> Callable& {
            //return (Callable&) frame().code.obj();
            return frame().code.asType<Callable>();
        }
    };

    extern volatile uint32_t pending; // used for irq-safe inner loop exit bits

    auto loadModule (uint8_t const* addr) -> Module*;

} // namespace Monty
