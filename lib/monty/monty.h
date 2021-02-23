// Monty, a stackless VM - main header

#pragma once
#include <cstdint>
#include <cstdlib>
#include <cstring>

extern "C" int printf (char const*, ...);

namespace monty {

// see gc.cpp - objects and vectors with garbage collection

    template< typename T >
    struct Obj {
        static auto isInPool (void const* p) -> bool;
        auto isCollectable () const -> bool { return isInPool(this); }

        auto operator new (size_t bytes) -> void*;
        auto operator new (size_t bytes, uint32_t extra) -> void* {
            return operator new (bytes + extra);
        }
        void operator delete (void*);

        static void sweep ();   // reclaim all unmarked objects
        static void dumpAll (); // like sweep, but only to print all obj+free

        // JT's "Rule of 5"
        Obj () =default;
        Obj (Obj&&) =delete;
        Obj (Obj const&) =delete;
        auto operator= (Obj&&) -> Obj& =delete;
        auto operator= (Obj const&) -> Obj& =delete;
    };

    struct Vec {
        constexpr Vec () =default;
        constexpr Vec (void const* ptr, uint32_t num =0)
                    : _data ((uint8_t*) ptr), _capa (num) {}
        ~Vec () { (void) adj(0); }

        static auto isInPool (void const* p) -> bool;
        auto isResizable () const -> bool {
            return _data == nullptr || isInPool(_data);
        }

        static void compact (); // reclaim and compact unused vector space
        static void dumpAll (); // like compact, but only to print all vec+free

    // protected: TODO test/*/main.cpp need access
        auto ptr () const -> uint8_t* { return _data; }
        auto cap () const -> uint32_t { return _capa; }
        auto adj (uint32_t bytes) -> bool;
    private:
        uint8_t* _data = nullptr; // pointer to vector when capa > 0
        uint32_t _capa = 0; // capacity in bytes

        auto slots () const -> uint32_t; // capacity in vecslots
        auto findSpace (uint32_t) -> void*; // hidden private type

        // JT's "Rule of 5"
        Vec (Vec&&) =delete;
        Vec (Vec const&) =delete;
        auto operator= (Vec&&) -> Vec& =delete;
        auto operator= (Vec const&) -> Vec& =delete;
    };

    void gcSetup (void* base, uint32_t size); // configure the memory pool
    auto gcMax () -> uint32_t; // return free space between objects & vectors
    auto gcCheck () -> bool;   // true when it's time to collect the garbage
    void gcReport ();          // print a brief gc summary with statistics

    union GCStats {
        struct {
            uint32_t
                checks, sweeps, compacts,
                toa, tob, tva, tvb, // totalObjAllocs/Bytes,totalVecAllocs/Bytes
                coa, cob, cva, cvb, // currObjAllocs/Bytes,currVecAllocs/Bytes
                moa, mob, mva, mvb; // maxObjAllocs/Bytes,maxVecAllocs/Bytes
        };
        uint32_t v [15];
    };
    extern GCStats gcStats;

    extern void* (*panicOutOfMemory)(); // triggers an assertion by default

// see data.cpp - basic object data types

    // forward decl's
    struct Object;
    struct Lookup;
    struct Buffer;
    struct Type;
    struct Range;

    extern char const qstrBase [];
    extern int const qstrBaseLen;

    struct Q {
        constexpr Q (uint16_t i, char const* =nullptr) : _id (i) {}

        operator char const* () const { return str(_id); }

        static auto hash (void const*, uint32_t) -> uint32_t;
        static auto str (uint16_t) -> char const*;
        static auto find (char const*) -> uint16_t;
        static auto make (char const*) -> uint16_t;
        static auto last () -> uint16_t;

        uint16_t _id;
    };

    enum class E : uint8_t { // parsed by codegen.py, see builtin.cpp
        //CG< exceptions
        BaseException,       // -
        Exception,           // BaseException
        StopIteration,       // Exception
        AssertionError,      // Exception
        AttributeError,      // Exception
        EOFError,            // Exception
        ImportError,         // Exception
        MemoryError,         // Exception
        NameError,           // Exception
        OSError,             // Exception
        TypeError,           // Exception
        ArithmeticError,     // Exception
        ZeroDivisionError,   // ArithmeticError
        LookupError,         // Exception
        IndexError,          // LookupError
        KeyError,            // LookupError
        RuntimeError,        // Exception
        NotImplementedError, // RuntimeError
        ValueError,          // Exception
        UnicodeError,        // ValueError
        //CG>
    };

    enum UnOp : uint8_t {
        Pos, Neg, Inv, Not,
        Boln, Hash, Abs, Intg,
    };

    enum BinOp : uint8_t {
        //CG< binops ../../git/micropython/py/runtime0.h 35
        Less,
        More,
        Equal,
        LessEqual,
        MoreEqual,
        NotEqual,
        In,
        Is,
        ExceptionMatch,
        InplaceOr,
        InplaceXor,
        InplaceAnd,
        InplaceLshift,
        InplaceRshift,
        InplaceAdd,
        InplaceSubtract,
        InplaceMultiply,
        InplaceMatMultiply,
        InplaceFloorDivide,
        InplaceTrueDivide,
        InplaceModulo,
        InplacePower,
        Or,
        Xor,
        And,
        Lshift,
        Rshift,
        Add,
        Subtract,
        Multiply,
        MatMultiply,
        FloorDivide,
        TrueDivide,
        Modulo,
        Power,
        //CG>
        Contains,
    };

    struct Value {
        enum Tag { Nil, Int, Str, Obj };

        constexpr Value () : _v (0) {}
        constexpr Value (int arg) : _v (arg * 2 + 1) {}
        constexpr Value (Q const& arg) : _v (arg._id * 4 + 2) {}
                  Value (char const* arg);
        constexpr Value (Object const* arg) : _p (arg) {} // TODO keep?
        constexpr Value (Object const& arg) : _p (&arg) {}
                  Value (E, Value ={}, Value ={}); // also raises the exception

        operator int () const { return (intptr_t) _v >> 1; }
        operator char const* () const;
        auto operator ->() const -> Object*;
        auto obj () const -> Object& { return *_o; }
        auto asObj () const -> Object&; // create int/str object if needed
        auto asInt () const -> int64_t;

        template< typename T > // return null pointer if not of required type
        auto ifType () const -> T* { return check(T::info) ? (T*) &obj() : 0; }

        template< typename T > // type-asserted safe cast via Object::type()
        auto asType () const -> T& { verify(T::info); return *(T*) &obj(); }

        auto tag () const -> Tag {
            return (_v&1) != 0 ? Int : // bit 0 set
                       _v == 0 ? Nil : // all bits 0
                   (_v&2) != 0 ? Str : // bit 1 set, ptr shifted 2 up
                                 Obj;  // bits 0 and 1 clear, ptr stored as is
        }

        auto id () const -> uintptr_t { return _v; }

        auto isNil () const -> bool { return _v == 0; }
        auto isInt () const -> bool { return (_v&1) == Int; }
        auto isStr () const -> bool { return (_v&3) == Str; }
        auto isObj () const -> bool { return (_v&3) == 0 && _v != 0; }

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
            uintptr_t _v;
            void const* _p;
            Object* _o;
        };
    };

    extern Value const Null;
    extern Value const False;
    extern Value const True;
    extern Value const Empty; // Tuple

    auto Value::asBool (bool f) -> Value { return f ? True : False; }

    template< typename T >
    struct VecOf : private Vec {
        uint32_t _fill = 0;

        constexpr VecOf () =default;
        template< size_t N >
        constexpr VecOf (T const (&ary)[N])
                    : Vec (&ary, N * sizeof (T)), _fill (N) {}
        constexpr VecOf (T const* ptr, uint32_t num)
                    : Vec (ptr, num * sizeof (T)), _fill (num) {}

        auto cap () const -> uint32_t { return Vec::cap() / sizeof (T); }
        auto adj (uint32_t num) -> bool { return Vec::adj(num * sizeof (T)); }

        constexpr auto size () const -> uint32_t { return _fill; }
        constexpr auto begin () const -> T* { return (T*) Vec::ptr(); }
        constexpr auto end () const -> T* { return begin() + _fill; }
        auto operator[] (uint32_t idx) const -> T& { return begin()[idx]; }

        auto relPos (int i) const -> uint32_t { return i < 0 ? i + _fill : i; }

        void move (uint32_t pos, uint32_t num, int off) {
            memmove((void*) (begin() + pos + off),
                        (void const*) (begin() + pos), num * sizeof (T));
        }
        void wipe (uint32_t pos, uint32_t num) {
            memset((void*) (begin() + pos), 0, num * sizeof (T));
        }

        void insert (uint32_t idx, uint32_t num =1) {
            if (_fill > cap())
                _fill = cap();
            if (idx > _fill) {
                num += idx - _fill;
                idx = _fill;
            }
            auto need = _fill + num;
            if (need > cap())
                adj(need);
            move(idx, _fill - idx, num);
            wipe(idx, num);
            _fill += num;
        }

        void remove (uint32_t idx, uint32_t num =1) {
            if (_fill > cap())
                _fill = cap();
            if (idx >= _fill)
                return;
            if (num > _fill - idx)
                num = _fill - idx;
            move(idx + num, _fill - (idx + num), -num);
            _fill -= num;
        }

        auto find (T v) const -> uint32_t {
            for (auto& e : *this)
                if (e == v)
                    return &e - begin();
            return _fill;
        }

        void push (T v, uint32_t idx =0) {
            insert(idx);
            begin()[idx] = v;
        }

        void append (T v) { push(v, _fill); } // push to end

        auto pull (uint32_t idx =0) -> T {
            if (idx >= _fill)
                return {};
            T v = begin()[idx];
            remove(idx);
            return v;
        }

        auto pop () -> T { return pull(_fill-1); } // pull from end

        using Vec::compact; // allow public access
    };

    using Vector = VecOf<Value>;

    void markVec (Vector const&);

    struct ArgVec {
        ArgVec (Vector const& v)
            : ArgVec (v, v.size()) {}
        ArgVec (Vector const& v, int n, Value const* p)
            : ArgVec (v, n, p - v.begin()) {}
        constexpr ArgVec (Vector const& v, int n, int o =0)
            : _vec (v), _num (n), _off (o) {}

        auto size () const -> uint32_t { return _num; }
        auto begin () const -> Value const* { return _vec.begin() + _off; }
        auto end () const -> Value const* { return begin() + _num; }
        auto operator[] (uint32_t idx) const -> Value& { return _vec[_off+idx]; }

        Vector const& _vec;
        int _num;
        int _off;
    };

    // see https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern
    // see JT's 10 minute video: https://www.youtube.com/watch?v=ZQ-8laAr9Dg
    // it doesn't do much here, just avoids an extra layer of class derivation

    // can't use "CG type <object>", this is the start of the type hierarchy
    struct Object : Obj<Object> {
        virtual ~Object () =default;

        static const Type info;
        virtual auto type () const -> Type const& { return info; }
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

        virtual void marker () const {} // called to mark all ref'd objects

        auto sliceGetter (Value k) const -> Value;
        auto sliceSetter (Value k, Value v) -> Value;
    };

    void mark (Object const&);
    inline void mark (Object const* p) { if (p != nullptr) mark(*p); }

    void Value::marker () const { if (isObj()) mark(obj()); }

    //CG3 type <none>
    struct None : Object {
        static Type info;
        auto type () const -> Type const& override { return info; }

        auto repr (Buffer&) const -> Value override;

        static None const nullObj;
    private:
        constexpr None () =default; // can't construct more instances
    };

    //CG< type bool
    struct Bool : Object {
        static auto create (ArgVec const&,Type const* =nullptr) -> Value;
        static Lookup const attrs;
        static Type info;
        auto type () const -> Type const& override { return info; }
        auto repr (Buffer&) const -> Value override;
    //CG>
        auto unop (UnOp) const -> Value override;

        static Bool const trueObj;
        static Bool const falseObj;
    private:
        constexpr Bool () =default; // can't construct more instances
    };

    auto Value::isNone  () const -> bool { return &obj() == &None::nullObj; }
    auto Value::isFalse () const -> bool { return &obj() == &Bool::falseObj; }
    auto Value::isTrue  () const -> bool { return &obj() == &Bool::trueObj; }

    //CG< type int
    struct Int : Object {
        static auto create (ArgVec const&,Type const* =nullptr) -> Value;
        static Lookup const attrs;
        static Type info;
        auto type () const -> Type const& override { return info; }
        auto repr (Buffer&) const -> Value override;
    //CG>
        static auto make (int64_t i) -> Value;
        static auto conv (char const* s) -> Value;

        constexpr Int (int64_t v) : _i64 (v) {}

        operator int64_t () const { return _i64; }

        auto unop (UnOp) const -> Value override;
        auto binop (BinOp, Value) const -> Value override;

    private:
        int64_t _i64 __attribute__((packed));
    }; // packing gives a better fit on 32b arch, and has no effect on 64b

// see type.cpp - collection types and type system

    using ByteVec = VecOf<uint8_t>;

    //CG3 type <iterator>
    struct Iterator : Object {
        static Type info;
        auto type () const -> Type const& override { return info; }

        Iterator (Object const& obj, int pos =0) : _ipos (pos), iobj (obj) {}

        auto next() -> Value override;

        void marker () const override { mark(iobj); }
    private:
        int _ipos;
        Object const& iobj;
    };

    //CG< type range
    struct Range : Object {
        static auto create (ArgVec const&,Type const* =nullptr) -> Value;
        static Lookup const attrs;
        static Type info;
        auto type () const -> Type const& override { return info; }
        auto repr (Buffer&) const -> Value override;
    //CG>
        auto len () const -> uint32_t override;
        auto getAt (Value k) const -> Value override;
        auto iter () const -> Value override { return 0; }

        Range (int a, int b, int c) : _from (a), _to (b), _by (c) {}

        int32_t _from, _to, _by;
    };

    //CG< type slice
    struct Slice : Object {
        static auto create (ArgVec const&,Type const* =nullptr) -> Value;
        static Lookup const attrs;
        static Type info;
        auto type () const -> Type const& override { return info; }
        auto repr (Buffer&) const -> Value override;
    //CG>
        auto asRange (int sz) const -> Range;

    private:
        Slice (Value a, Value b, Value c) : _off (a), _num (b), _step (c) {}

        Value _off, _num, _step;
    };

    //CG< type bytes
    struct Bytes : Object, ByteVec {
        static auto create (ArgVec const&,Type const* =nullptr) -> Value;
        static Lookup const attrs;
        static Type info;
        auto type () const -> Type const& override { return info; }
        auto repr (Buffer&) const -> Value override;
    //CG>
        constexpr Bytes () =default;
        Bytes (void const*, uint32_t =0);

        auto unop (UnOp) const -> Value override;
        auto binop (BinOp, Value) const -> Value override;
        auto len () const -> uint32_t override { return _fill; }
        auto getAt (Value k) const -> Value override;
        auto iter () const -> Value override { return 0; }
        auto copy (Range const&) const -> Value override;
    };

    //CG< type str
    struct Str : Bytes {
        static auto create (ArgVec const&,Type const* =nullptr) -> Value;
        static Lookup const attrs;
        static Type info;
        auto type () const -> Type const& override { return info; }
        auto repr (Buffer&) const -> Value override;
    //CG>
        Str (char const* s, int n =-1);

        operator char const* () const { return (char const*) begin(); }

        auto unop (UnOp) const -> Value override;
        auto binop (BinOp, Value) const -> Value override;
        auto getAt (Value k) const -> Value override;
    };

    //CG3 type <buffer>
    struct Buffer : Bytes {
        static Type info;
        auto type () const -> Type const& override { return info; }
        auto repr (Buffer& buf) const -> Value override;

        ~Buffer () override;

        void write (uint8_t const* ptr, uint32_t num);
        void putc (char v) { write((uint8_t const*) &v, 1); }
        void puts (char const* s) { while (*s != 0) putc(*s++); }
        void print (char const* fmt, ...);

        auto operator<< (Value v) -> Buffer&;
        auto operator<< (char c) -> Buffer& { putc(c); return *this; }
        auto operator<< (int i) -> Buffer& { return *this << (Value) i; }
        auto operator<< (char const* s) -> Buffer& { puts(s); return *this; }

    private:
        int splitInt (uint32_t val, int base, uint8_t* buf);
        void putFiller (int n, char fill);
        void putInt (int val, int base, int width, char fill);
    };

    struct VaryVec : private ByteVec {
        constexpr VaryVec (void const* ptr =nullptr, uint32_t num =0)
                    : ByteVec ((uint8_t const*) ptr, num) {}

        using ByteVec::size;

        auto first () const -> uint8_t const* { return begin(); }
        auto limit () const -> uint8_t const* { return begin() + pos(_fill); }

        auto atGet (uint32_t i) const -> uint8_t* {
            return begin() + pos(i);
        }
        auto atLen (uint32_t i) const -> uint32_t {
            return pos(i+1) - pos(i);
        }
        void atAdj (uint32_t idx, uint32_t num);
        void atSet (uint32_t i, void const* ptr, uint32_t num);

        void insert (uint32_t idx, uint32_t num =1);
        void remove (uint32_t idx, uint32_t num =1);
    private:
        auto pos (uint32_t i) const -> uint16_t& {
            return ((uint16_t*) begin())[i];
        }
    };

    //CG3 type <lookup>
    struct Lookup : Object {
        static Type info;
        auto type () const -> Type const& override { return info; }

        struct Item { Value k, v; }; // TODO plain const Value list or dict ?

        constexpr Lookup (Item const* p =nullptr, uint32_t sz =0)
                        : _items (p), _count (sz / sizeof (Item)) {}

        auto operator[] (char const* key) const -> Value;

        auto len () const -> uint32_t override { return _count; }
        auto getAt (Value k) const -> Value override;

        void marker () const override;
    private:
        Item const* _items;
        uint32_t _count;

        friend struct Exception; // to get exception's string name
    };

    //CG< type tuple
    struct Tuple : Object {
        static auto create (ArgVec const&,Type const* =nullptr) -> Value;
        static Lookup const attrs;
        static Type info;
        auto type () const -> Type const& override { return info; }
        auto repr (Buffer&) const -> Value override;
    //CG>
        auto size () const -> uint32_t { return _fill; }
        auto begin () const -> Value const* { return data(); }
        auto end () const -> Value const* { return begin() + size(); }
        auto operator[] (uint32_t idx) const -> Value { return begin()[idx]; }

        auto len () const -> uint32_t override { return _fill; }
        auto getAt (Value k) const -> Value override;
        auto iter () const -> Value override { return 0; }
        auto copy (Range const&) const -> Value override;

        void marker () const override;

        uint32_t const _fill;

        static Tuple const emptyObj;
    protected:
        constexpr Tuple () : _fill (0) {}
        Tuple (ArgVec const&);

        auto data () const -> Value const* { return (Value const*) (this + 1); }
    };

    //CG< type list
    struct List : Object, Vector {
        static auto create (ArgVec const&,Type const* =nullptr) -> Value;
        static Lookup const attrs;
        static Type info;
        auto type () const -> Type const& override { return info; }
        auto repr (Buffer&) const -> Value override;
    //CG>
        constexpr List () =default;

        //CG: wrap List pop append clear
        auto pop (int idx) -> Value;
        void append (Value v);
        Value clear () { remove(0, size()); return {}; }

        auto len () const -> uint32_t override { return _fill; }
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
        static Type info;
        auto type () const -> Type const& override { return info; }
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
        static Type info;
        auto type () const -> Type const& override { return info; }
        auto repr (Buffer&) const -> Value override;
    //CG>
        constexpr Dict (Object const* ch =nullptr) : _chain (ch) {}

        struct Proxy { Dict& d; Value k;
            operator Value () const { return ((Dict const&) d).at(k); }
            auto operator= (Value v) -> Value;
        };

        auto at (Value key) const -> Value;
        auto at (Value key) -> Proxy { return {*this, key}; }

        auto getAt (Value k) const -> Value override { return at(k); }
        auto setAt (Value k, Value v) -> Value override { return at(k) = v; }

        //CG: wrap Dict keys values items
        auto keys () -> Value;
        auto values () -> Value;
        auto items () -> Value;

        void marker () const override;

        Object const* _chain {nullptr};
    private:
        Dict (uint32_t n) { adj(2*n); }
    };

    //CG3 type <dictview>
    struct DictView : Object {
        static Type info;
        auto type () const -> Type const& override { return info; }

        DictView (Dict const& d, int vt) : _dict (d), _vtype (vt) {}

        auto len () const -> uint32_t override { return _dict._fill; }
        auto getAt (Value k) const -> Value override;
        auto iter () const -> Value override { return 0; }

        void marker () const override { _dict.marker(); }
    private:
        Dict const& _dict;
        int _vtype;
    };

    //CG< type type
    struct Type : Dict {
        static auto create (ArgVec const&,Type const* =nullptr) -> Value;
        static Lookup const attrs;
        static Type info;
        auto type () const -> Type const& override { return info; }
        auto repr (Buffer&) const -> Value override;
    //CG>
        using Factory = auto (*)(ArgVec const&,Type const*) -> Value;

        constexpr Type (Value s, Lookup const* a =nullptr, Factory f =noFactory)
                        : Dict (a), _name (s), _factory (f) {}

        auto call (ArgVec const&) const -> Value override;
        auto attr (char const* name, Value&) const -> Value override {
            return getAt(name);
        }

        Value _name;
        Factory _factory;

    private:
        static auto noFactory (ArgVec const&, Type const*) -> Value;
    };

    //CG< type class
    struct Class : Type {
        static auto create (ArgVec const&,Type const* =nullptr) -> Value;
        static Lookup const attrs;
        static Type info;
        auto type () const -> Type const& override { return info; }
        auto repr (Buffer&) const -> Value override;
    //CG>
    private:
        Class (ArgVec const& args);
    };

    //CG< type super
    struct Super : Object {
        static auto create (ArgVec const&,Type const* =nullptr) -> Value;
        static Lookup const attrs;
        static Type info;
        auto type () const -> Type const& override { return info; }
        auto repr (Buffer&) const -> Value override;
    //CG>

        void marker () const override { _sclass.marker(); _sinst.marker(); }
    private:
        Super (ArgVec const& args);

        Value _sclass;
        Value _sinst;
    };

    // can't use CG, because type() can't be auto-generated
    struct Inst : Dict {
        static auto create (ArgVec const&, Type const*) -> Value;
        static Lookup const attrs;
        static Type const info;
        auto repr (Buffer&) const -> Value override;

        auto type () const -> Type const& override { return *(Type*) _chain; }
        auto attr (char const* name, Value& self) const -> Value override {
            self = this;
            return getAt(name);
        }

    private:
        Inst (ArgVec const& args, Class const& cls);
    };

// see array.cpp - arrays, dicts, and other derived types

    //CG< type array
    struct Array : Bytes {
        static auto create (ArgVec const&,Type const* =nullptr) -> Value;
        static Lookup const attrs;
        static Type info;
        auto type () const -> Type const& override { return info; }
        auto repr (Buffer&) const -> Value override;
    //CG>
        constexpr static auto LEN_BITS = 27;

        //constexpr Array () =default; // default is array of Value items
        Array (char type, uint32_t num =0);

        auto mode () const -> char;

        void insert (uint32_t idx, uint32_t num =1);
        void remove (uint32_t idx, uint32_t num =1);

        auto len () const -> uint32_t override;
        auto getAt (Value k) const -> Value override;
        auto setAt (Value k, Value v) -> Value override;
        auto copy (Range const&) const -> Value override;
        auto store (Range const&, Object const&) -> Value override;

    private:
        auto sel () const -> uint8_t { return _fill >> LEN_BITS; }
    };

// see stack.cpp - events, stacklets, and various call mechanisms

    //CG< type event
    struct Event : Object {
        static auto create (ArgVec const&,Type const* =nullptr) -> Value;
        static Lookup const attrs;
        static Type info;
        auto type () const -> Type const& override { return info; }
        auto repr (Buffer&) const -> Value override;
    //CG>

        ~Event () override { deregHandler(); set(); }

        auto unop (UnOp) const -> Value override;
        auto binop (BinOp, Value) const -> Value override;

        void marker () const override { markVec(_queue); }

        auto regHandler () -> uint32_t;
        void deregHandler ();

        operator bool () const { return _value; }

        //CG: wrap Event wait set clear
        void set ();
        void clear () { _value = false; }
        void wait ();

        static int queued;
        static Vector triggers;
    private:
        Vector _queue;
        bool _value = false;
        int8_t _id = -1;
    };

    //CG3 type <stacklet>
    struct Stacklet : Event, Vector {
        static Type info;
        auto type () const -> Type const& override { return info; }
        auto repr (Buffer&) const -> Value override;

        auto binop (BinOp, Value) const -> Value override;

        static void yield (bool =false);
        static void suspend (Vector& =Event::triggers);
        static auto runLoop () -> bool;

        virtual auto run () -> bool =0;
        virtual void raise (Value);

        static void exception (Value); // a safe way to current->raise()
        static void gcAll ();

        void marker () const override { Event::marker(); markVec(*this); }

        // see https://en.cppreference.com/w/c/atomic and
        // https://gcc.gnu.org/onlinedocs/gcc/_005f_005fatomic-Builtins.html
        static auto setPending (uint32_t n) -> bool {
            auto prev = __atomic_fetch_or(&pending, 1<<n, __ATOMIC_RELAXED);
            return (prev >> n) & 1;
        }
        static auto clearPending (uint32_t n) -> bool {
            auto prev = __atomic_fetch_and(&pending, ~(1<<n), __ATOMIC_RELAXED);
            return (prev >> n) & 1;
        }
        static auto allPending () -> uint32_t {
            return __atomic_fetch_and(&pending, 0, __ATOMIC_RELAXED); // clears
        }

        static List tasks;
        static volatile uint32_t pending;
        static Stacklet* current;
    };

    //CG3 type <module>
    struct Module : Dict {
        static Type info;
        auto type () const -> Type const& override { return info; }
        auto repr (Buffer&) const -> Value override;

        constexpr Module (Value nm, Object const& lu =builtins)
            : Dict (&lu), _name (nm) {}

        auto attr (char const* s, Value&) const -> Value override {
            Value v = getAt(s);
            return v.isNil() && strcmp(s, "__name__") == 0 ? _name : v;
        }

        static Dict builtins;
        static Dict loaded;

        Value _name;
    };

    //CG3 type <function>
    struct Function : Object {
        static Type info;
        auto type () const -> Type const& override { return info; }
        using Prim = auto (*)(ArgVec const&) -> Value;

        constexpr Function (Prim f) : _func (f) {}

        auto call (ArgVec const& args) const -> Value override {
            return _func(args);
        }

    private:
        Prim _func;
    };

    // Horrendous C++11 ... this wraps several different argument calls into a
    // virtual MethodBase object, which Method can then call in a generic way.
    // It's probably just a neophyte's version of STL's <functional> types ...
    // TODO maybe an "argument pack" or "forwarding" can simplify this stuff?

    // obj.meth() const -> void
    template< typename T >
    auto argConv (void (T::*m)() const, Object& o, ArgVec const&) -> Value {
        (((T&) o).*m)();
        return {};
    }
    // obj.meth() -> void
    template< typename T >
    auto argConv (void (T::*m)(), Object& o, ArgVec const&) -> Value {
        (((T&) o).*m)();
        return {};
    }
    // obj.meth() -> Value
    template< typename T, typename V >
    auto argConv (V (T::*m)(), Object& o, ArgVec const&) -> V {
        return (((T&) o).*m)();
    }
    // obj.meth(arg) -> void
    template< typename T, typename U >
    auto argConv (void (T::*m)(U), Object& o, ArgVec const& a) -> Value {
        (((T&) o).*m)(a[1]);
        return {};
    }
    // obj.meth(arg) -> Value
    template< typename T, typename U, typename V >
    auto argConv (V (T::*m)(U), Object& o, ArgVec const& a) -> V {
        return (((T&) o).*m)(a[1]);
    }
    // obj.meth(argvec) -> Value
    template< typename T, typename V >
    auto argConv (V (T::*m)(ArgVec const&), Object& o, ArgVec const& a) -> V {
        return (((T&) o).*m)(a);
    }

    // Method objects point to objects of this base class to make virtual calls
    struct MethodBase {
        virtual auto call (Object&, ArgVec const&) const -> Value = 0;
    };

    template< typename M >
    struct MethodDef : MethodBase {
        constexpr MethodDef (M memberPtr) : _methPtr (memberPtr) {}

        auto call (Object& self, ArgVec const& args) const -> Value override {
            return argConv(_methPtr, self, args);
        }

    private:
        const M _methPtr;
    };

    //CG3 type <method>
    struct Method : Object {
        static Type info;
        auto type () const -> Type const& override { return info; }

        constexpr Method (MethodBase const& m) : _meth (m) {}

        auto call (ArgVec const& args) const -> Value override {
            return _meth.call(args[0].obj(), args);
        }

        template< typename M >
        constexpr static auto wrap (M memberPtr) -> MethodDef<M> {
            return memberPtr;
        }

    private:
        const MethodBase& _meth;
    };

    //CG3 type <boundmeth>
    struct BoundMeth : Object {
        static Type info;
        auto type () const -> Type const& override { return info; }

        BoundMeth (Object const& f, Value o) : _meth (f), _self (o) {}

        auto call (ArgVec const&) const -> Value override;

        void marker () const override { mark(_meth); _self.marker(); }
    private:
        Object const& _meth;
        Value _self;
    };

    //CG3 type <cell>
    struct Cell : Object {
        static Type info;
        auto type () const -> Type const& override { return info; }

        Cell (Value v) : _val (v) {}

        void marker () const override { _val.marker(); }

        Value _val;
    };

    //CG3 type <closure>
    struct Closure : List {
        static Type info;
        auto type () const -> Type const& override { return info; }
        auto repr (Buffer& buf) const -> Value override;

        Closure (Object const&, ArgVec const&);

        auto call (ArgVec const&) const -> Value override;

        void marker () const override { List::marker(); mark(_func); }
    private:
        Object const& _func;
    };

// see builtin.cpp - exceptions and auto-generated built-in tables

    //CG3 type <exception>
    struct Exception : Tuple {
        static Type info;
        auto type () const -> Type const& override { return info; }
        auto repr (Buffer&) const -> Value override;

        struct Extra { E code; uint16_t ipOff; Object const* callee; };
        auto extra () const -> Extra& { return *(Extra*) end(); }

        static auto create (E, ArgVec const&) -> Value; // diff API
        static Lookup const bases; // this maps the derivation hierarchy
        static auto findId (Function const&) -> int; // find in builtinsMap

        void marker () const override;
    private:
        Exception (E exc, ArgVec const& args);

        auto binop (BinOp, Value) const -> Value override;
    };

// see library.cpp - runtime library functions for several datatypes
    void libInstall ();

// defined outside of the Monty core itself, e.g. in main.cpp cq pyvm.cpp
    auto vmImport (char const* name) -> uint8_t const*;
    auto vmLaunch (void const* data) -> Stacklet*;
}
