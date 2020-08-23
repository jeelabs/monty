#include <cstdint>
#include <cstdlib>
#include <cstring>

namespace Monty {
    extern "C" int printf (char const*, ...);

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
    auto gcCheck () -> bool; // true when it's time to collect the garbage

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

    struct Q {
        constexpr Q (uint16_t i, char const* p) : id (i), s (p) {}

        constexpr operator char const* () const { return s; }

        static auto str (uint16_t) -> char const*;
        static auto find (char const*) -> uint16_t;
        static auto make (char const*) -> uint16_t;

        uint16_t id;
        char const* s;
    };

    struct Value {
        enum Tag { Nil, Int, Str, Obj };

        constexpr Value () : v (0) {}
        constexpr Value (int arg) : v (arg * 2 + 1) {}
        constexpr Value (Q const& arg) : v (arg.id * 4 + 2) {}
                  Value (char const* arg);
        constexpr Value (Object const* arg) : p (arg) {} // TODO keep?
        constexpr Value (Object const& arg) : p (&arg) {}

        operator int () const { return (intptr_t) v >> 1; }
        operator char const* () const;
        auto obj () const -> Object& { return *(Object*) v; }
        auto asObj () const -> Object&; // create int/str object if needed

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
        auto isObj () const -> bool { return (v&3) == 0 && v != 0; }

        inline auto isNull  () const -> bool;
        inline auto isFalse () const -> bool;
        inline auto isTrue  () const -> bool;
               auto isBool  () const -> bool { return isFalse() || isTrue(); }

        auto truthy () const -> bool;

        auto operator== (Value) const -> bool;
        auto operator< (Value) const -> bool;

        auto unOp (UnOp op) const -> Value;
        auto binOp (BinOp op, Value rhs) const -> Value;

        inline void marker () const;
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

    using ByteVec = VecOf<uint8_t>;
    using Vector = VecOf<Value>;

    void mark (Vector const&);

    struct ArgVec {
        ArgVec (Vector const& v, int n, int o) : vec (v), num (n), off (o) {}

        //auto size () const -> size_t { return num; }
        auto begin () const -> Value const* { return vec.begin() + off; }
        auto end () const -> Value const* { return begin() + num; }
        auto operator[] (size_t idx) const -> Value& { return vec[off+idx]; }

        Vector const& vec;
        int num;
        int off;
    };

    // can't use "CG type <object>", as type/repr are virtual iso override
    struct Object : Obj {
        static const Type info;
        virtual auto type () const -> Type const&;
        virtual auto repr  (Buffer&) const -> Value;

        virtual auto call  (ArgVec const&) const -> Value;
        virtual auto unop  (UnOp) const -> Value;
        virtual auto binop (BinOp, Value) const -> Value;
        virtual auto attr  (char const*, Value&) const -> Value;
        virtual auto len   () const -> size_t;
        virtual auto getAt (Value) const -> Value;
        virtual auto setAt (Value, Value) -> Value;
        virtual auto next  () -> Value;
    };

    void Value::marker () const { if (isObj()) mark(obj()); }

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
        static auto create (ArgVec const&,Type const* =nullptr) -> Value;
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
    struct Int : Object {
        static auto create (ArgVec const&,Type const* =nullptr) -> Value;
        static Lookup const attrs;
        static Type const info;
        auto type () const -> Type const& override;
        auto repr (Buffer&) const -> Value override;
    //CG>
        constexpr Int (int64_t v) : i (v) {}

        operator int64_t () const { return i; }

        auto unop (UnOp) const -> Value override;
        auto binop (BinOp, Value) const -> Value override;

    private:
        int64_t i __attribute__((packed));
    }; // packing gives a better fit on 32b arch, and has no effect on 64b

    //CG< type bytes
    struct Bytes : Object, ByteVec {
        static auto create (ArgVec const&,Type const* =nullptr) -> Value;
        static Lookup const attrs;
        static Type const info;
        auto type () const -> Type const& override;
        auto repr (Buffer&) const -> Value override;
    //CG>
        constexpr Bytes () {}
        Bytes (void const*, size_t =0);

        auto hash (const uint8_t* p, size_t n) const -> uint32_t;

        auto unop (UnOp) const -> Value override;
        auto binop (BinOp, Value) const -> Value override;
        auto len () const -> size_t override { return size(); }
        auto getAt (Value k) const -> Value override;
    };

    //CG< type str
    struct Str : Bytes {
        static auto create (ArgVec const&,Type const* =nullptr) -> Value;
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
        static auto create (ArgVec const&,Type const* =nullptr) -> Value;
        static Lookup const attrs;
        static Type const info;
        auto type () const -> Type const& override;
        auto repr (Buffer&) const -> Value override;
    //CG>
    };

    //CG< type slice
    struct Slice : Object {
        static auto create (ArgVec const&,Type const* =nullptr) -> Value;
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

    //CG< type tuple
    struct Tuple : Object {
        static auto create (ArgVec const&,Type const* =nullptr) -> Value;
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

        void marker () const override;

        size_t const fill;

        static Tuple const emptyObj;
    protected:
        constexpr Tuple () : fill (0) {}
        Tuple (ArgVec const&);

        auto data () const -> Value const* { return (Value const*) (this + 1); }
    };

    //CG3 type <exception>
    struct Exception : Tuple {
        static Type const info;
        auto type () const -> Type const& override;

        struct Extra { int code; };

        static auto create (int, ArgVec const&) -> Value; // diff API
        static Lookup const bases; // this maps the derivation hierarchy

        void marker () const override;
    private:
        Exception (int exc, ArgVec const& args);

        auto extra () -> Extra& { return *(Extra*) end(); }
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
        void print (char const* fmt, ...);

        auto operator<< (Value v) -> Buffer&; // TODO get rid of this C++'ism

        bool sep {false};
    protected:
        virtual void write (uint8_t const* ptr, size_t len) const;
    private:
        int splitInt (uint32_t val, int base, uint8_t* buf);
        void putFiller (int n, char fill);
        void putInt (int val, int base, int width, char fill);
    };

// see array.cpp - arrays, dicts, and other derived types

    //CG< type array
    struct Array : Object, ByteVec {
        static auto create (ArgVec const&,Type const* =nullptr) -> Value;
        static Lookup const attrs;
        static Type const info;
        auto type () const -> Type const& override;
        auto repr (Buffer&) const -> Value override;
    //CG>
        constexpr Array () {} // default is array of Value items
        Array (char type, size_t len =0);

        auto mode () const -> char;

        auto len () const -> size_t override;
        auto getAt (Value k) const -> Value override;
        auto setAt (Value k, Value v) -> Value override;

    private:
        auto sel () const -> uint8_t { return fill >> 28; }
    };

    //CG< type list
    struct List : Object, Vector {
        static auto create (ArgVec const&,Type const* =nullptr) -> Value;
        static Lookup const attrs;
        static Type const info;
        auto type () const -> Type const& override;
        auto repr (Buffer&) const -> Value override;
    //CG>
        constexpr List () {}
        List (ArgVec const& args);

        auto pop (int idx) -> Value;
        void append (Value v);

        auto len () const -> size_t override { return size(); }
        auto getAt (Value k) const -> Value override;
        auto setAt (Value k, Value v) -> Value override;

        void marker () const override { mark((Vector const&) *this); }
    };

    //CG< type set
    struct Set : List {
        static auto create (ArgVec const&,Type const* =nullptr) -> Value;
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
        static auto create (ArgVec const&,Type const* =nullptr) -> Value;
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

        void marker () const override;
    // TODO protected:
        Object const* chain {nullptr};
    };

    //CG< type type
    struct Type : Dict {
        static auto create (ArgVec const&,Type const* =nullptr) -> Value;
        static Lookup const attrs;
        static Type const info;
        auto type () const -> Type const& override;
        auto repr (Buffer&) const -> Value override;
    //CG>
        using Factory = auto (*)(ArgVec const&,Type const*) -> Value;

        constexpr Type (char const* s, Factory f =noFactory,
                                        Lookup const* a =nullptr)
                    : Dict (a), name (s), factory (f) {}

        auto call (ArgVec const&) const -> Value override;
        auto attr (char const* name, Value&) const -> Value override {
            return getAt(name);
        }

        char const* name;
        Factory factory;
    private:
        static auto noFactory (ArgVec const&, Type const*) -> Value;
    };

    //CG< type class
    struct Class : Type {
        static auto create (ArgVec const&,Type const* =nullptr) -> Value;
        static Lookup const attrs;
        static Type const info;
        auto type () const -> Type const& override;
        auto repr (Buffer&) const -> Value override;
    //CG>
        Class (ArgVec const& args);
    };

    // can't use CG, because type() must not be auto-generated
    struct Inst : Dict {
        static auto create (ArgVec const&, Type const*) -> Value;
        static Lookup const attrs;
        static Type const info;
        auto repr (Buffer&) const -> Value override;

        auto type () const -> Type const& override { return *(Type*) chain; }
        auto attr (char const* name, Value& self) const -> Value override {
            self = this;
            return Dict::attr(name, self);
        }

    private:
        Inst (ArgVec const& args, Class const& cls);
    };

    extern Lookup const builtins;

// see call.cpp - execution state, stacks, and callables

    //CG3 type <function>
    struct Function : Object {
        static Type const info;
        auto type () const -> Type const& override;
        using Prim = auto (*)(ArgVec const&) -> Value;

        constexpr Function (Prim f) : func (f) {}

        auto call (ArgVec const& args) const -> Value override {
            return func(args);
        }

    protected:
        Prim func;
    };

    // Horrendous C++ ... this wraps several different argument calls into a
    // virtual MethodBase object, which Method can then call in a generic way.
    // It's probably just a neophyte's version of STL's <functional> types ...
    // TODO maybe an "argument pack" or "forwarding" can simplify this stuff?

    template< typename T >
    auto argConv (auto (T::*meth)() -> Value,
                                Object& self, ArgVec const&) -> Value {
        return (((T&) self).*meth)();
    }
    template< typename T >
    auto argConv (auto (T::*meth)(Value) -> Value,
                                Object& self, ArgVec const& args) -> Value {
        return (((T&) self).*meth)(args[1]);
    }
    template< typename T >
    auto argConv (void (T::*meth)(Value),
                                Object& self, ArgVec const& args) -> Value {
        (((T&) self).*meth)(args[1]);
        return {};
    }
    template< typename T >
    auto argConv (auto (T::*meth)(int) -> Value,
                                Object& self, ArgVec const& args) -> Value {
        return (((T&) self).*meth)(args[1]);
    }
    template< typename T >
    auto argConv (auto (T::*meth)(const char *) -> Value,
                                Object& self, ArgVec const& args) -> Value {
        return (((T&) self).*meth)(args[1]);
    }
    template< typename T >
    auto argConv (auto (T::*meth)(ArgVec const&) -> Value,
                                Object& self, ArgVec const& args) -> Value {
        return (((T&) self).*meth)(args);
    }

    // Method objects point to objects of this base class to make virtual calls
    struct MethodBase {
        virtual auto call (Object&, ArgVec const&) const -> Value = 0;
    };

    template< typename M >
    struct MethodDef : MethodBase {
        constexpr MethodDef (M memberPtr) : methPtr (memberPtr) {}

        auto call (Object& self, ArgVec const& args) const -> Value override {
            return argConv(methPtr, self, args);
        }

    private:
        const M methPtr;
    };

    //CG3 type <method>
    struct Method : Object {
        static Type const info;
        auto type () const -> Type const& override;

        constexpr Method (MethodBase const& m) : meth (m) {}

        auto call (ArgVec const& args) const -> Value override {
            return meth.call(args[0].obj(), args);
        }

        template< typename M >
        constexpr static auto wrap (M memberPtr) -> MethodDef<M> {
            return memberPtr;
        }

    private:
        const MethodBase& meth;
    };

    //CG3 type <module>
    struct Module : Dict {
        static Type const info;
        auto type () const -> Type const& override;

        Module (Lookup const* lu, Value pool ={}) : Dict (lu), qp (pool) {}

        Value attr (char const* s, Value&) const override { return getAt(s); }

        void marker () const override { Dict::marker(); qp.marker(); }

        Value qp;
    };

    //CG3 type <bytecode>
    struct Bytecode : List {
        static Type const info;
        auto type () const -> Type const& override;

        auto fastSlotTop () const -> size_t { return stackSz; }
        auto excLevel () const -> size_t { return excDepth; }
        auto isGenerator () const -> bool { return (flags & 1) != 0; }
        auto hasVarArgs () const -> bool { return (flags & 4) != 0; }
        auto numCells () const -> size_t { return n_cell; }

        auto numArgs (int t) const -> uint32_t {
            return t == 0 ? n_pos : t == 1 ? n_def_pos : n_kwonly;
        }

        auto start () const -> uint8_t const* {
            return (uint8_t const*) (this + 1) + code;
        }

    private:
        Bytecode () {}

        int16_t code;
        int16_t stackSz;
        int16_t hdrSz;
        int16_t size;

        int8_t flags;
        int8_t nData;
        int8_t nCode;
        int8_t excDepth;
        int8_t n_pos;
        int8_t n_kwonly;
        int8_t n_def_pos;
        int8_t n_cell;

        friend struct Loader;
    };

    //CG3 type <callable>
    struct Callable : Object {
        static Type const info;
        auto type () const -> Type const& override;

        Callable (Value callee, Module* mod)
                : Callable (callee, nullptr, nullptr, mod) {}
        Callable (Value callee, Value pos, Value kw)
                : Callable (callee, pos.ifType<Tuple>(), kw.ifType<Dict>()) {}
        Callable (Value, Tuple* =nullptr, Dict* =nullptr, Module* =nullptr);

        auto qStrAt (size_t) const -> char const*;

        auto call (ArgVec const&) const -> Value override;

        void marker () const override;

    // TODO private:
        Module& mo;
        Bytecode const& code;
        Tuple* pos;
        Dict* kw;
    };

    //CG3 type <boundmeth>
    struct BoundMeth : Object {
        static Type const info;
        auto type () const -> Type const& override;

        BoundMeth (Callable const& f, Value o) : meth (f), self (o) {}

        auto call (ArgVec const&) const -> Value override;

        void marker () const override { meth.marker(); self.marker(); }
    private:
        Callable const& meth;
        Value self;
    };

    //CG3 type <cell>
    struct Cell : Object {
        static Type const info;
        auto type () const -> Type const& override;

        Cell (Value v) : val (v) {}

        void marker () const override { val.marker(); }

        Value val;
    };

    //CG3 type <closure>
    struct Closure : List {
        static Type const info;
        auto type () const -> Type const& override;

        //using Object::repr; TODO can't be used to bypass List::repr ?
        auto repr (Buffer& buf) const -> Value override {
            return Object::repr(buf); // don't print as a list
        }

        Closure (Callable const&, ArgVec const&);

        auto call (ArgVec const&) const -> Value override;

        void marker () const override { List::marker(); func.marker(); }
    private:
        Callable const& func;
    };

    //CG3 type <context>
    struct Context : List {
        static Type const info;
        auto type () const -> Type const& override;

        //using Object::repr; TODO can't be used to bypass List::repr ?
        auto repr (Buffer& buf) const -> Value override {
            return Object::repr(buf); // don't print as a list
        }

        struct Frame {
            //    <------- previous ------->  <---- actual ---->
            Value base, spOff, ipOff, callee, ep, locals, result, stack [];
        };

        auto frame () const -> Frame& { return *(Frame*) (begin() + base); }

        Context (Context* from =nullptr) : caller (from) {}

        void enter (Callable const&);
        Value leave (Value v);

        auto spBase () const -> Value* { return frame().stack; }
        auto ipBase () const -> uint8_t const* { return callee->code.start(); }

        auto fastSlot (size_t i) const -> Value& {
            return spBase()[callee->code.fastSlotTop() + ~i];
        }
        auto derefSlot (size_t i) const -> Value& {
            return fastSlot(i).asType<Cell>().val;
        }

        static constexpr int EXC_STEP = 3; // use 3 slots per exception
        auto excBase (int incr =0) -> Value*;

        auto globals () const -> Module& { return callee->mo; }

        void raise (Value exc ={});
        void caught ();

        auto call (ArgVec const&) const -> Value override;
        auto next () -> Value override;

        void marker () const override;

        // previous values are saved in current stack frame
        size_t base {0};
        size_t spOff {0};
        size_t ipOff {0};
        Callable const* callee {nullptr};

        Value event;
        Context* caller;
    };

    struct Interp {
        static auto frame () -> Context::Frame& { return context->frame(); }

        static void suspend (List& queue);
        static void resume (Context& ctx);

        static void exception (Value exc);  // throw exception in curr context
        static void interrupt (uint32_t n); // trigger a soft-irq (irq-safe)
        static auto nextPending () -> int;  // next pending or -1 (irq-safe)
        static auto pendingBit (uint32_t) -> bool; // test and clear bit

        static int setHandler (Value);
        bool isAlive () const;

        void markAll (); // for gc

        static volatile uint32_t pending;   // for irq-safe inner loop exit
        static Context* context;            // current context, if any
        static List tasks;                  // runnable task queue
    protected:
        static constexpr auto MAX_HANDLERS = 8 * sizeof pending;
        static Value handlers [];
    };

// see import.cpp - importing and loading bytecodes

    auto importer (char const* name, uint8_t const* addr) -> Callable*;

} // namespace Monty
