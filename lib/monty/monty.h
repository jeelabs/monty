// Monty, a stackless VM - main header

#include <cstdint>
#include <cstdlib>
#include <cstring>

extern "C" int printf (char const*, ...);
extern "C" int puts (char const*);
extern "C" int putchar (int);

namespace monty {

// see gc.cpp - objects and vectors with garbage collection

    struct Obj {
        virtual ~Obj () {}

        static auto inPool (void const* p) -> bool;
        auto isCollectable () const -> bool { return inPool(this); }

        auto operator new (size_t bytes) -> void*;
        auto operator new (size_t bytes, uint32_t extra) -> void* {
            return operator new (bytes + extra);
        }
        void operator delete (void*);
    protected:
        constexpr Obj () {} // only derived objects can be instantiated

        virtual void marker () const {} // called to mark all ref'd objects
        friend void mark (Obj const&);
    };

    struct Vec {
        constexpr Vec () {}
        constexpr Vec (void const* ptr, uint32_t num =0)
                    : data ((uint8_t*) ptr), capa (num) {}
        ~Vec () { (void) adj(0); }

        Vec (Vec const&) = delete;
        auto operator= (Vec const&) -> Vec& = delete;

        static auto inPool (void const* p) -> bool;
        auto isResizable () const -> bool {
            return data == nullptr || inPool(data);
        }

        auto ptr () const -> uint8_t* { return data; }
        auto cap () const -> uint32_t { return capa; }
        auto adj (uint32_t bytes) -> bool;

    private:
        uint8_t* data = nullptr; // pointer to vector when capa > 0
        uint32_t capa = 0; // capacity in bytes

        auto slots () const -> uint32_t; // capacity in vecslots
        auto findSpace (uint32_t) -> void*; // hidden private type
        friend void compact ();
    };

    void setup (void* base, uint32_t size); // configure the memory pool

    auto gcAvail () -> uint32_t; // return free space between objects & vectors
    auto gcCheck () -> bool;     // true when it's time to collect the garbage
    void gcNow ();               // first call Interp::markAll, then gcNow
    void gcObjDump ();           // like sweep, but only to print all obj+free
    void gcReport ();            // print a brief gc summary with statistics

    struct GCStats {
        uint32_t
            checks, sweeps, compacts,
            toa, tob, tva, tvb, // totalObjAllocs/Bytes, totalVecAllocs/Bytes
            coa, cob, cva, cvb, // currObjAllocs/Bytes,  currVecAllocs/Bytes
            moa, mob, mva, mvb; // maxObjAllocs/Bytes,   maxVecAllocs/Bytes
    };
    extern GCStats gcStats;

    inline void mark (Obj const* p) { if (p != nullptr) mark(*p); }
    void mark (Obj const&);
    void sweep ();   // reclaim all unmarked objects
    void compact (); // reclaim and compact unused vector space

    extern void* (*panicOutOfMemory)(); // triggers an assertion by default

// see data.cpp - basic object data types

    // forward decl's
    enum UnOp : uint8_t;
    enum BinOp : uint8_t;
    struct Object;
    struct Lookup;
    struct Buffer;
    struct Type;
    struct Range;
    struct Q;

    // TODO keep in sync with exceptionMap in builtin.cpp, should be generated
    enum class E : uint8_t {
        BaseException,
        Exception,
        StopIteration,
        ArithmeticError,
        ZeroDivisionError,
        AssertionError,
        AttributeError,
        EOFError,
        ImportError,
        LookupError,
        IndexError,
        KeyError,
        MemoryError,
        NameError,
        OSError,
        RuntimeError,
        NotImplementedError,
        TypeError,
        ValueError,
        UnicodeError,
    };

    struct Value {
        enum Tag { Nil, Int, Str, Obj };

        constexpr Value () : v (0) {}
        constexpr Value (int arg) : v (arg * 2 + 1) {}
        constexpr Value (Q const& arg); //XXX : v (arg.id * 4 + 2) {}
                  Value (char const* arg);
        constexpr Value (Object const* arg) : p (arg) {} // TODO keep?
        constexpr Value (Object const& arg) : p (&arg) {}
                  Value (E, Value ={}, Value ={}); // also raises the exception

        operator int () const { return (intptr_t) v >> 1; }
        operator char const* () const;
        auto obj () const -> Object& { return *(Object*) v; }
        auto asObj () const -> Object&; // create int/str object if needed
        auto asInt () const -> int64_t;

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

        inline auto isNone  () const -> bool;
        inline auto isFalse () const -> bool;
        inline auto isTrue  () const -> bool;
               auto isBool  () const -> bool { return isFalse() || isTrue(); }

        auto truthy () const -> bool;

        auto operator== (Value) const -> bool;

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
        constexpr VecOf () {}
        constexpr VecOf (T const* ptr, uint32_t num =0)
                    : Vec (ptr, num * sizeof (T)), fill (num) {}

        auto cap () const -> uint32_t { return Vec::cap() / sizeof (T); }
        auto adj (uint32_t num) -> bool { return Vec::adj(num * sizeof (T)); }

        auto size () const -> uint32_t { return fill; }
        auto begin () const -> T* { return (T*) Vec::ptr(); }
        auto end () const -> T* { return begin() + fill; }
        auto operator[] (uint32_t idx) const -> T& { return begin()[idx]; }

        auto relPos (int i) const -> uint32_t { return i < 0 ? i + fill : i; }

        void move (uint32_t pos, uint32_t num, int off) {
            memmove((void*) (begin() + pos + off),
                        (void const*) (begin() + pos), num * sizeof (T));
        }
        void wipe (uint32_t pos, uint32_t num) {
            memset((void*) (begin() + pos), 0, num * sizeof (T));
        }

        void insert (uint32_t idx, uint32_t num =1) {
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

        void remove (uint32_t idx, uint32_t num =1) {
            if (fill > cap())
                fill = cap();
            if (idx >= fill)
                return;
            if (num > fill - idx)
                num = fill - idx;
            move(idx + num, fill - (idx + num), -num);
            fill -= num;
        }

        uint32_t fill = 0;
    };

    using Vector = VecOf<Value>;

    void markVec (Vector const&);

    struct ArgVec {
        ArgVec (Vector const& v, int n, Value const* p)
            : ArgVec (v, n, p - v.begin()) {}
        ArgVec (Vector const& v, int n, int o =0) : vec (v), num (n), off (o) {}

        //auto size () const -> uint32_t { return num; }
        auto begin () const -> Value const* { return vec.begin() + off; }
        auto end () const -> Value const* { return begin() + num; }
        auto operator[] (uint32_t idx) const -> Value& { return vec[off+idx]; }

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
        virtual auto len   () const -> uint32_t;
        virtual auto getAt (Value) const -> Value;
        virtual auto setAt (Value, Value) -> Value;
        virtual auto iter  () const -> Value;
        virtual auto next  () -> Value;
        virtual auto copy  (Range const&) const -> Value;
        virtual auto store (Range const&, Object const&) -> Value;

        auto sliceGetter (Value k) const -> Value;
        auto sliceSetter (Value k, Value v) -> Value;
    };

    void Value::marker () const { if (isObj()) mark(obj()); }

    //CG3 type <none>
    struct None : Object {
        static Type const info;
        auto type () const -> Type const& override;

        auto repr (Buffer&) const -> Value override;

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

    auto Value::isNone  () const -> bool { return &obj() == &None::nullObj; }
    auto Value::isFalse () const -> bool { return &obj() == &Bool::falseObj; }
    auto Value::isTrue  () const -> bool { return &obj() == &Bool::trueObj; }

// see type.cpp - collection types and type system

    //CG3 type <lookup>
    struct Lookup : Object {
        static Type const info;
        auto type () const -> Type const& override;

        struct Item { Value k, v; }; // TODO plain const Value list or dict ?

        constexpr Lookup (Item const* p, uint32_t sz)
                        : items (p), count (sz / sizeof (Item)) {}

        auto operator[] (char const* key) const -> Value;

        auto len () const -> uint32_t override { return count; }
        auto getAt (Value k) const -> Value override;

        void marker () const override;
    private:
        Item const* items;
        uint32_t count;

        friend struct Exception; // to get exception's string name
    };

    //CG< type tuple
    struct Tuple : Object {
        static auto create (ArgVec const&,Type const* =nullptr) -> Value;
        static Lookup const attrs;
        static Type const info;
        auto type () const -> Type const& override;
        auto repr (Buffer&) const -> Value override;
    //CG>
        auto size () const -> uint32_t { return fill; }
        auto begin () const -> Value const* { return data(); }
        auto end () const -> Value const* { return begin() + size(); }
        auto operator[] (uint32_t idx) const -> Value { return begin()[idx]; }

        auto len () const -> uint32_t override { return fill; }
        auto getAt (Value k) const -> Value override;
        auto iter () const -> Value override { return 0; }
        auto copy (Range const&) const -> Value override;

        void marker () const override;

        uint32_t const fill;

        static Tuple const emptyObj;
    protected:
        constexpr Tuple () : fill (0) {}
        Tuple (ArgVec const&);

        auto data () const -> Value const* { return (Value const*) (this + 1); }
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

        auto pop (int idx) -> Value;
        void append (Value v);
        Value clear () { remove(0, size()); return {}; }

        auto len () const -> uint32_t override { return fill; }
        auto getAt (Value k) const -> Value override;
        auto setAt (Value k, Value v) -> Value override;
        auto iter () const -> Value override { return 0; }
        auto copy (Range const&) const -> Value override;
        auto store (Range const&, Object const&) -> Value override;

        void marker () const override { markVec(*this); }
    protected:
        List (ArgVec const& args);
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

        auto find (Value v) const -> uint32_t;

        struct Proxy { Set& s; Value v;
            operator bool () const { return ((Set const&) s).has(v); }
            auto operator= (bool) -> bool;
        };

        // operator[] is problematic when the value is an int
        auto has (Value key) const -> bool { return find(key) < size(); }
        auto has (Value key) -> Proxy { return {*this, key}; }

        auto binop (BinOp, Value) const -> Value override;

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

        struct Proxy { Dict& d; Value k;
            operator Value () const { return ((Dict const&) d).at(k); }
            auto operator= (Value v) -> Value;
        };

        auto at (Value key) const -> Value;
        auto at (Value key) -> Proxy { return {*this, key}; }

        auto getAt (Value k) const -> Value override { return at(k); }
        auto setAt (Value k, Value v) -> Value override { return at(k) = v; }

        auto keys () -> Value;
        auto values () -> Value;
        auto items () -> Value;

        void marker () const override;

        Object const* chain {nullptr};
    private:
        Dict (uint32_t n) { adj(2*n); }
    };

    //CG3 type <dictview>
    struct DictView : Object {
        static Type const info;
        auto type () const -> Type const& override;

        DictView (Dict const& d, int vt) : dict (d), vtype (vt) {}

        auto len () const -> uint32_t override { return dict.fill; }
        auto getAt (Value k) const -> Value override;
        auto iter () const -> Value override { return 0; }

        void marker () const override { dict.marker(); }
    private:
        Dict const& dict;
        int vtype;
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

        constexpr Type (Value s, Factory f =noFactory, Lookup const* a =nullptr)
                        : Dict (a), name (s), factory (f) {}

        auto call (ArgVec const&) const -> Value override;
        auto attr (char const* name, Value&) const -> Value override {
            return getAt(name);
        }

        Value name;
        Factory factory;

    private:
        static auto noFactory (ArgVec const&, Type const*) -> Value;
    };

// see context.cpp - run time contexts and interpreter

    // forward decl's
    struct Module;
    struct Bytecode;

    //CG3 type <callable>
    struct Callable : Object {
        static Type const info;
        auto type () const -> Type const& override;

        Callable (Value callee, Module* mod)
                : Callable (callee, nullptr, nullptr, mod) {}
        Callable (Value callee, Value pos, Value kw)
                : Callable (callee, pos.ifType<Tuple>(), kw.ifType<Dict>()) {}
        Callable (Value, Tuple* =nullptr, Dict* =nullptr, Module* =nullptr);

        auto call (ArgVec const&) const -> Value override;

        void marker () const override;

    // TODO private:
        Module& mo;
        Bytecode const& bc;
        Tuple* pos;
        Dict* kw;
    };

    //CG3 type <context>
    struct Context : List {
        static Type const info;
        auto type () const -> Type const& override;
        auto repr (Buffer& buf) const -> Value override;

        // first entries in a context are reserved slots for specific state
        enum Slot { Caller, Event, NumSlots };
        auto caller () const -> Value& { return begin()[Caller]; }
        auto event () const -> Value& { return begin()[Event]; }

        struct Frame {
            //    <------- previous ------->  <---- actual ---->
            Value base, spOff, ipOff, callee, ep, locals, result, stack [];
            // result must be just below stack for proper module/class init
        };

        auto frame () const -> Frame& { return *(Frame*) (begin() + base); }

        Context (Context* from =nullptr) {
            insert(0, NumSlots);
            caller() = from;
        }

        void enter (Callable const&);
        auto leave (Value v ={}) -> Value;

        auto spBase () const -> Value* { return frame().stack; }
        auto ipBase () const -> uint8_t const*; //XXX { return callee->bc.start(); }

        auto fastSlot (uint32_t i) const -> Value&; /*XXX {
            return spBase()[callee->bc.fastSlotTop() + ~i];
        }*/
        auto derefSlot (uint32_t i) const -> Value&; /*XXX {
            return fastSlot(i).asType<Cell>().val;
        }*/

        static constexpr int EXC_STEP = 3; // use 3 entries per exception
        auto excBase (int incr =0) -> Value*;

        auto globals () const -> Module& { return callee->mo; }

        constexpr static auto FinallyTag = 1<<20;
        void raise (Value exc ={});
        void caught ();

        auto iter () const -> Value override { return this; }
        auto next () -> Value override;

        void marker () const override { List::marker(); mark(callee); }

        int8_t qid = 0;
        // previous values are saved in current stack frame
        uint16_t base = 0;
        uint16_t spOff = 0;
        uint16_t ipOff = 0;
        Callable const* callee {nullptr};
    };

    struct Interp {
        static auto frame () -> Context::Frame& { return context->frame(); }

        static void suspend (uint32_t id);
        static void resume (Context& ctx);

        static void exception (Value exc);  // throw exception in curr context
        static void interrupt (uint32_t n); // trigger a soft-irq (irq-safe)
        static auto nextPending () -> int;  // next pending or -1 (irq-safe)
        static auto pendingBit (uint32_t) -> bool; // test and clear bit

        static auto findTask (Context& ctx) -> int;
        static auto getQueueId () -> int;
        static void dropQueueId (int);

        static auto isAlive () -> bool { return tasks.len() > 0; }

        static void markAll (); // for gc

        static constexpr auto NUM_QUEUES = 32;

        static volatile uint32_t pending;   // for irq-safe inner loop exit
        static uint32_t queueIds;           // which queues are in use
        static Context* context;            // current context, if any
        static List tasks;                  // list of all tasks
        static Dict modules;                // loaded modules
    };
}
