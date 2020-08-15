#include <cstdint>
#include <cstdlib>
#include <cstring>

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

    void setup (void* base, size_t size); // configure the memory pool

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
        auto asObj () const -> Object&; // create int/str object if needed

        template< typename T > // return null pointer if not of required type
        //auto ifType () const -> T* { return dynamic_cast<T*>(&obj()); }
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

        auto relPos (int i) const -> uint32_t { return i < 0 ? i + fill : i; }

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

    // can't use "CG type <object>", as type/repr are virtual iso override
    struct Object : Obj {
        static const Type info;
        virtual auto type () const -> Type const&;
        virtual auto repr  (Buffer&) const -> Value;

        virtual auto call  (Vector const&, int, int) const -> Value;
        virtual auto unop  (UnOp) const -> Value;
        virtual auto binop (BinOp, Value) const -> Value;
        virtual auto attr  (const char*, Value&) const -> Value;
        virtual auto len   () const -> size_t;
        virtual auto getAt (Value) const -> Value;
        virtual auto setAt (Value, Value) -> Value;
        virtual auto next  () -> Value;
    };

    //CG3 type <none>
    struct None : Object {
        static Type const info;
        auto type () const -> Type const& override;

        auto repr (Buffer&) const -> Value override;
        auto unop (UnOp) const -> Value override;

        static None const nullObj;
    private:
        constexpr None () {} // can't construct more instances
    };

    //CG< type bool
    struct Bool : Object {
        static auto create (Vector const&,int,int,Type const* =nullptr) -> Value;
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
        static auto create (Vector const&,int,int,Type const* =nullptr) -> Value;
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
    struct Bytes : Object, VecOf<uint8_t> {
        static auto create (Vector const&,int,int,Type const* =nullptr) -> Value;
        static Lookup const attrs;
        static Type const info;
        auto type () const -> Type const& override;
        auto repr (Buffer&) const -> Value override;
    //CG>
        Bytes (void const* =nullptr, size_t =0);

        auto hash (const uint8_t* p, size_t n) const -> uint32_t;

        auto unop (UnOp) const -> Value override;
        auto binop (BinOp, Value) const -> Value override;
        auto len () const -> size_t override { return size(); }
        auto getAt (Value k) const -> Value override;
    };

    //CG< type str
    struct Str : Bytes {
        static auto create (Vector const&,int,int,Type const* =nullptr) -> Value;
        static Lookup const attrs;
        static Type const info;
        auto type () const -> Type const& override;
        auto repr (Buffer&) const -> Value override;
    //CG>
        Str (char const* s, int n =-1);

        operator char const* () const { return (char const*) begin(); }

        auto unop (UnOp) const -> Value override;
        auto binop (BinOp, Value) const -> Value override;
        auto getAt (Value k) const -> Value override;
    };

    //CG< type range
    struct Range : Object {
        static auto create (Vector const&,int,int,Type const* =nullptr) -> Value;
        static Lookup const attrs;
        static Type const info;
        auto type () const -> Type const& override;
        auto repr (Buffer&) const -> Value override;
    //CG>
    };

    //CG< type slice
    struct Slice : Object {
        static auto create (Vector const&,int,int,Type const* =nullptr) -> Value;
        static Lookup const attrs;
        static Type const info;
        auto type () const -> Type const& override;
        auto repr (Buffer&) const -> Value override;
    //CG>
        Slice (Value a, Value b, Value c);

    private:
        int32_t off, num, step;
    };

    //CG3 type <lookup>
    struct Lookup : Object {
        static Type const info;
        auto type () const -> Type const& override;
        struct Item { char const* k; Value v; };

        constexpr Lookup (Item const* p, size_t sz)
            : items (p), count (sz / sizeof (Item)) {}

        auto operator[] (char const* key) const -> Value;

        auto len () const -> size_t override { return count; }
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

    //CG3 type <buffer>
    struct Buffer : Object {
        static Type const info;
        auto type () const -> Type const& override;
        void putc (char v) { write((uint8_t const*) &v, 1); }
        void puts (char const* s) { while (*s != 0) putc(*s++); }
        void print (const char* fmt, ...);

        auto operator<< (Value v) -> Buffer&;

        bool sep {false};
    protected:
        virtual void write (uint8_t const* ptr, size_t len) const;
    private:
        int splitInt (uint32_t val, int base, uint8_t* buf);
        void putFiller (int n, char fill);
        void putInt (int val, int base, int width, char fill);
    };

// see array.cpp - arrays, dicts, and other derived types

    //CG< type tuple
    struct Tuple : Object {
        static auto create (Vector const&,int,int,Type const* =nullptr) -> Value;
        static Lookup const attrs;
        static Type const info;
        auto type () const -> Type const& override;
        auto repr (Buffer&) const -> Value override;
    //CG>
        auto size () const -> size_t { return fill; }
        auto begin () const -> Value const* { return data(); }
        auto end () const -> Value const* { return begin() + size(); }
        auto operator[] (size_t idx) const -> Value { return begin()[idx]; }

        auto len () const -> size_t override { return size(); }
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
        static auto create (Vector const&,int,int,Type const* =nullptr) -> Value;
        static Lookup const attrs;
        static Type const info;
        auto type () const -> Type const& override;
        auto repr (Buffer&) const -> Value override;
    //CG>
        constexpr List () {}
        List (Vector const& vec, int argc, int args);

        auto len () const -> size_t override { return size(); }
        auto getAt (Value k) const -> Value override;
        auto setAt (Value k, Value v) -> Value override;

        void marker () const override { mark((Vector const&) *this); }
    };

    //CG< type set
    struct Set : List {
        static auto create (Vector const&,int,int,Type const* =nullptr) -> Value;
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
        auto has (Value key) const -> bool { return find(key) < size(); }
        auto has (Value key) -> Proxy { return {*this, key}; }

        auto getAt (Value k) const -> Value override;
        auto setAt (Value k, Value v) -> Value override;
    };

    //CG< type dict
    struct Dict : Set {
        static auto create (Vector const&,int,int,Type const* =nullptr) -> Value;
        static Lookup const attrs;
        static Type const info;
        auto type () const -> Type const& override;
        auto repr (Buffer&) const -> Value override;
    //CG>
        constexpr Dict (Object const* ch =nullptr) : chain (ch) {}
        Dict (size_t n) { adj(2*n); }

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
        static auto create (Vector const&,int,int,Type const* =nullptr) -> Value;
        static Lookup const attrs;
        static Type const info;
        auto type () const -> Type const& override;
        auto repr (Buffer&) const -> Value override;
    //CG>
        using Factory = auto (*)(Vector const&,int,int,Type const*) -> Value;

        constexpr Type (char const* s, Factory f =noFactory,
                                        Lookup const* a =nullptr)
            : Dict (a), name (s), factory (f) {}

        auto call (Vector const& vec, int argc, int args) const -> Value override;
        auto attr (char const* name, Value& self) const -> Value override {
            return getAt(name);
        }

        char const* name;
        Factory factory;
    private:
        static auto noFactory (Vector const&, int, int, Type const*) -> Value;
    };

    //CG< type class
    struct Class : Type {
        static auto create (Vector const&,int,int,Type const* =nullptr) -> Value;
        static Lookup const attrs;
        static Type const info;
        auto type () const -> Type const& override;
        auto repr (Buffer&) const -> Value override;
    //CG>
        Class (Vector const& vec, int argc, int args);
    };

    // can't use CG, because type() must not be auto-generated
    struct Inst : Dict {
        static auto create (Vector const&, int, int, Type const*) -> Value;
        static Lookup const attrs;
        static Type const info;
        auto repr (Buffer&) const -> Value override;

        auto type () const -> Type const& override { return *(Type*) chain; }

    private:
        Inst (Vector const& vec, int argc, int args, Class const& cls);
    };

    extern Lookup const builtins;

// see stack.cpp - execution state, stacks, and callables

    //CG3 type <function>
    struct Function : Object {
        static Type const info;
        auto type () const -> Type const& override;
        using Prim = auto (*)(Vector const&,int,int) -> Value;

        constexpr Function (Prim f) : func (f) {}

        auto call (Vector const& vec, int argc, int args) const -> Value override {
            return func(vec, argc, args);
        }

    protected:
        Prim func;
    };

    //CG3 type <boundmeth>
    struct BoundMeth : Object {
        static Type const info;
        auto type () const -> Type const& override;
        constexpr BoundMeth (Value f, Value o) : meth (f), self (o) {}

        void marker () const override { mark(meth); mark(self); }

        Value meth;
        Value self;
    };

// see exec.cpp - importing, loading, and bytecode execution

    //CG3 type <module>
    struct Module : Dict {
        static Type const info;
        auto type () const -> Type const& override;
        Module (Value qpool) : Dict (&builtins), qp (qpool) {}

        Value attr (const char* s, Value&) const override { return getAt(s); }

        void marker () const override { Dict::marker(); mark(qp); }

        Value qp;
    };

    //CG3 type <bytecode>
    struct Bytecode : Object {
        static Type const info;
        auto type () const -> Type const& override;
        auto constAt (size_t i) const -> Value { return constObjs[i]; }
        auto fastSlotTop () const -> size_t { return stackSz; }
        auto excLevel () const -> size_t { return excDepth; }
        auto isGenerator () const -> bool { return (flags & 1) != 0; }
        auto hasVarArgs () const -> bool { return (flags & 4) != 0; }

        auto numArgs (int t) const -> uint32_t {
            return t == 0 ? n_pos : t == 1 ? n_def_pos : n_kwonly;
        }

        auto codeStart () const -> uint8_t const* {
            return (uint8_t const*) (this + 1) + code;
        }

        void marker () const override {} // TODO
    private:
        Bytecode () {}

        Vector constObjs;
        int16_t code;
        int16_t stackSz;
        int16_t flags;
        int16_t hdrSz;
        int16_t size;
        int16_t nData;
        int16_t nCode;
        int8_t excDepth;
        int8_t n_pos;
        int8_t n_kwonly;
        int8_t n_def_pos;

        friend struct Loader;
    };

    //CG3 type <callable>
    struct Callable : Object {
        static Type const info;
        auto type () const -> Type const& override;
        Callable (Module& mod, Bytecode const& callee,
                    Tuple* t =nullptr, Dict* d =nullptr)
            : mo (mod), code (callee), pos (t), kw (d) {}

        auto qStrAt (size_t) const -> char const*;

        auto call (Vector const& vec, int argc, int args) const -> Value override;

        void marker () const override;

    // TODO private:
        Module& mo;
        Bytecode const& code;
        Tuple* pos;
        Dict* kw;
    };

    //CG3 type <context>
    struct Context : List {
        static Type const info;
        auto type () const -> Type const& override;

        //using Object::repr; TODO can't be used to bypass List::repr ?
        auto repr (Buffer& buf) const -> Value override {
            return Object::repr(buf); // don't print as a list
        }

        Context (Context* from =nullptr) : caller (from) {}

        void enter (Callable const&);
        Value leave (Value v);

        auto getQstr (size_t i) const -> char const* {
            return callee->qStrAt(i);
        }
        auto getConst (size_t i) const -> Value {
            return callee->code.constAt(i);
        }
        auto fastSlot (size_t i) const -> Value& {
            return spBase()[callee->code.fastSlotTop() + ~i];
        }
        auto spBase () const -> Value* {
            return frame().stack;
        }
        auto ipBase () const -> uint8_t const* {
            return callee->code.codeStart();
        }

        static constexpr int EXC_STEP = 3; // use 3 slots per exception
        auto excBase (int incr =0) -> Value*;

        auto globals () const -> Module& { return callee->mo; }

        void raise (Value exc ={});
        void caught ();

        auto next () -> Value override;

        void marker () const override;

        // previous values are saved in current stack frame
        size_t spIdx {0};
        size_t ipIdx {0};
        size_t epIdx {0};
        Callable const* callee {nullptr};
        Dict* locals {nullptr};
        Value result;

        size_t limit {0};
        Value event;
        Context* caller;

    private:
        struct Frame {
            Value link, sp, ip, ep, callee, locals, result, stack [];
        };

        auto frame () const -> Frame& { return *(Frame*) end(); }
    };

    struct Interp {
        static void resume (Context& ctx);

        static void exception (Value exc);  // throw exception in curr context
        static void interrupt (uint32_t n); // trigger a soft-irq (irq-safe)
        static auto nextPending () -> int;  // next pending or -1 (irq-safe)
        static auto wasPending (uint32_t) -> bool; // test and clear bit

        static volatile uint32_t pending;   // for irq-safe inner loop exit
        static Context* context;             // current context, if any
    };

    auto loadModule (uint8_t const* addr) -> Module*;

} // namespace Monty
