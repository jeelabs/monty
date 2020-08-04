// This is the public header for Monty, everything needed by add-ons is in here.

#include <stdint.h>
#include <stdlib.h>

struct Vector {
#if 0
    constexpr Vector (size_t bits) : logBits (0), capacity (0) {
        while (bits > (1U << logBits))
            ++logBits;
    }
#else
    constexpr static uint8_t log2 (size_t b) {
        return b <= 1 ? 0 : b <= 2 ? 1 : b <= 4 ? 2 : b <= 8 ? 3 : b <= 16 ? 4 :
                b <= 32 ? 5 : b <= 64 ? 6 : b <= 128 ? 7 : b <= 256 ? 8 :
                 b <= 512 ? 9 : b <= 1024 ? 10 : b <= 2048 ? 11 : 12;
    }
    constexpr Vector (size_t bits) : logBits (log2(bits)), capacity (0) {}
#endif

    ~Vector () { alloc(0); }

    size_t length () const { return fill; }
    int width () const { auto b = 1<<logBits; return b < 8 ? -b : b/8; }
    int widthOf (int num) const { return (num << logBits) >> 3; }

    int getInt (int idx) const;
    uint32_t getIntU (int idx) const;
    void* getPtr (int idx) const;

    void set (int idx, int val);
    void set (int idx, const void* ptr);

    void ins (int idx, int num =1);
    void del (int idx, int num =1);

    static void gcCompact ();   // see gc.c
protected:
    struct Data {
        Vector* v;
        union { uint32_t n; uint8_t d [1]; };
        Data* next () const;
    };

    Data* data = 0;
    uint32_t logBits :8;
    uint32_t capacity :24;      // in bytes
    uint32_t fill = 0;          // in elements
private:
    void alloc (size_t sz);     // see gc.c
};

template< typename T >
struct VecOf : Vector {
    constexpr VecOf () : Vector (8 * sizeof (T)) {}

    T get (int idx) const { return *(T*) getPtr(idx); }
    void set (int idx, T val) { Vector::set(idx, &val); }
};

// defined in defs.h
enum class Op : uint8_t;
enum class UnOp : uint8_t;
enum class BinOp : uint8_t;
typedef const Op* OpPtrRO;

struct Object;    // forward decl
struct TypeObj;   // forward decl
struct ModuleObj; // forward decl
struct LookupObj; // forward decl
struct SeqObj;    // forward decl
struct ForceObj;  // forward decl
struct Context;   // forward decl
struct BufferObj; // forward decl

struct Value {
    enum Tag { Nil, Int, Str, Obj };

    constexpr Value ()                  : v (0) {}
    constexpr Value (int arg)           : v ((arg << 1) | 1) {}
              Value (const char* arg)   : v (((uintptr_t) arg << 2) | 2) {}
    constexpr Value (const Object* arg) : p (arg) {}
    constexpr Value (const Object& arg) : p (&arg) {}

    operator int () const { return (intptr_t) v >> 1; }
    operator const char* () const { return (const char*) (v >> 2); }
    Object& obj () const { return *(Object*) p; }
    inline ForceObj objPtr () const;

    template< typename T > // return null pointer if not of required type
    T* ifType () const { return check(T::info) ? (T*) &obj() : 0; }

    template< typename T > // type-asserted safe cast via Object::type()
    T& asType () const { verify(T::info); return *(T*) &obj(); }

    enum Tag tag () const {
        return (v & 1) ? Int : // bit 0 set
                v == 0 ? Nil : // all bits 0
               (v & 2) ? Str : // bit 1 set, ptr shifted 2 up
                         Obj;  // bits 0 and 1 clear, ptr stored as is
    }

    uintptr_t id () const { return v; }

    bool isNil () const { return v == 0; }
    bool isInt () const { return v & 1; }
    bool isStr () const { return (v & 3) == 2; }
    bool isObj () const { return (v & 3) == 0 && v != 0; }

    inline bool isNone  () const;
    inline bool isFalse () const;
    inline bool isTrue  () const;
           bool isBool  () const { return isFalse() || isTrue(); }

    bool truthy () const;

    bool isEq (Value) const;
    Value unOp (UnOp op) const;
    Value binOp (BinOp op, Value rhs) const;
    void dump (const char* msg =0) const; // see builtin.h

    static Value asBool (int f) { return f ? True : False; }
    Value invert () const { return asBool(!truthy()); }

    static const Value None;
    static const Value False;
    static const Value True;

    static Value invalid; // special value, see DictObj::atKey
private:
    bool check (const TypeObj& t) const;
    void verify (const TypeObj& t) const;

    union {
        uintptr_t v;
        const void* p;
    };

    friend Context; // TODO yuck, just so Context::handlers [] can be inited
};

struct VecOfValue : VecOf<Value> {
    void markVec (void (*gc)(const Object&)) const;
};

struct Object {
    virtual ~Object () {}

    static const TypeObj info;
    virtual const TypeObj& type () const;

    virtual void mark (void (*gc)(const Object&)) const {}

    virtual Value repr  (BufferObj&) const; // see builtin.h
    virtual Value call  (int, Value[]) const;
    virtual Value unop  (UnOp) const;
    virtual Value binop (BinOp, Value) const;
    virtual Value attr  (const char*, Value&) const;
    virtual Value at    (Value) const;
    virtual Value iter  () const;
    virtual Value next  ();

    virtual const SeqObj& asSeq () const;

    void* operator new (size_t);
    void operator delete (void*);

    static void gcStats ();
protected:
    void* operator new (size_t, size_t);
};

//CG3 type <none>
struct NoneObj : Object {
    static const TypeObj info;
    const TypeObj& type () const override;

    Value repr  (BufferObj&) const override; // see builtin.h

    static const NoneObj noneObj;
private:
    constexpr NoneObj () {} // can't construct more instances
};

bool Value::isNone  () const { return &obj() == &NoneObj::noneObj; }

//CG< type bool
struct BoolObj : Object {
    static Value create (const TypeObj&, int argc, Value argv[]);
    static const LookupObj attrs;
    static const TypeObj info;
    const TypeObj& type () const override;
//CG>

    Value repr (BufferObj&) const override; // see builtin.h
    Value unop (UnOp) const override;

    static const BoolObj trueObj;
    static const BoolObj falseObj;
private:
    constexpr BoolObj () {} // can't construct more instances
};

bool Value::isFalse () const { return &obj() == &BoolObj::falseObj; }
bool Value::isTrue  () const { return &obj() == &BoolObj::trueObj; }

//CG< type int
struct IntObj : Object {
    static Value create (const TypeObj&, int argc, Value argv[]);
    static const LookupObj attrs;
    static const TypeObj info;
    const TypeObj& type () const override;
//CG>

    IntObj (int v) : i (v) {}
    IntObj (unsigned v) : i (v) {}

private:
    int64_t i;
};

//CG< type slice
struct SliceObj : Object {
    static Value create (const TypeObj&, int argc, Value argv[]);
    static const LookupObj attrs;
    static const TypeObj info;
    const TypeObj& type () const override;
//CG>

    SliceObj (Value a, Value b, Value c);

private:
    int32_t off, num, cap;
};

//CG3 type <iterator>
struct IterObj : Object {
    static const TypeObj info;
    const TypeObj& type () const override;

    IterObj (Value arg) : seq (arg) {}

    Value next () override;

private:
    Value seq;
    int pos = 0;
};

//CG3 type <sequence>
struct SeqObj : Object {
    static const TypeObj info;
    const TypeObj& type () const override;

    const SeqObj& asSeq () const override { return *this; }

    virtual Value  isIn  (Value) const;
    virtual Value  plus  (Value) const;
    virtual Value  times (Value) const;
    virtual size_t len   ()      const;
    virtual Value  min   ()      const;
    virtual Value  max   ()      const;
    virtual Value  index (Value) const;
    virtual Value  count (Value) const;

    static const SeqObj dummy;
protected:
    constexpr SeqObj () {} // cannot be instantiated directly
};

//CG< type bytes
struct BytesObj : SeqObj, protected Vector {
    static Value create (const TypeObj&, int argc, Value argv[]);
    static const LookupObj attrs;
    static const TypeObj info;
    const TypeObj& type () const override;
//CG>

    BytesObj (const void* p, size_t n);
    ~BytesObj () override;
    operator const uint8_t* () const;

    Value repr (BufferObj&) const override; // see builtin.h
    Value unop (UnOp) const override;
    Value binop (BinOp, Value) const override;
    Value at (Value) const override;
    size_t len () const override { return hasVec() ? length() : noVec().size; }

    Value decode () const;

    void* operator new (size_t, size_t);

    static uint32_t hash (const uint8_t* p, size_t n);
protected:
    static constexpr int MAX_NOVEC = 16;
    struct NoVec { uint8_t flag, size; uint8_t bytes [MAX_NOVEC]; };
    static_assert (sizeof (NoVec) >= sizeof (Vector), "MAX_NOVEC is too small");

    bool hasVec () const { return ((uintptr_t) data & 1) == 0; }
    NoVec& noVec () const { return *(NoVec*) (const Vector*) this; }
};

//CG< type str
struct StrObj : SeqObj {
    static Value create (const TypeObj&, int argc, Value argv[]);
    static const LookupObj attrs;
    static const TypeObj info;
    const TypeObj& type () const override;
//CG>

    StrObj (const char* v) : s (v) {}
    operator const char* () const { return s; }

    Value repr (BufferObj&) const override; // see builtin.h
    Value at (Value) const override;
    size_t len () const override;
    Value count (Value) const override { return 9; } // TODO

    Value encode () const;
    Value format (int argc, Value argv[]) { return 4; } // TODO

private:
    const char* s;
};

struct ForceObj : Value {
    ForceObj (const Value* p) : Value (*p), i (*p), s (*p) {}
    Object& operator* () const;
    Object* operator-> () const { return & operator*(); }

private:
    IntObj i;
    StrObj s;
};

ForceObj Value::objPtr () const { return this; }

//CG< type tuple
struct TupleObj : SeqObj {
    static Value create (const TypeObj&, int argc, Value argv[]);
    static const LookupObj attrs;
    static const TypeObj info;
    const TypeObj& type () const override;
//CG>
    void mark (void (*gc)(const Object&)) const override;

    Value repr (BufferObj&) const override; // see builtin.h
    size_t len () const override { return length; }
    Value at (Value) const override;

private:
    TupleObj (int argc, Value argv[]);

    uint16_t length;
    Value vec [];
};

//CG3 type <lookup>
struct LookupObj : SeqObj {
    static const TypeObj info;
    const TypeObj& type () const override;

    struct Item { const char* k; const Object* v; };

    constexpr LookupObj (const Item* p, size_t n) : vec (p), len (n) {}

    void mark (void (*gc)(const Object&)) const override;
    Value at (Value) const override;

private:
    const Item* vec;
    size_t len;
};

//CG3 type <mut-seq>
struct MutSeqObj : SeqObj, protected VecOfValue {
    static const TypeObj info;
    const TypeObj& type () const override;

    void mark (void (*gc)(const Object&)) const override;

    size_t len () const override { return length(); }

    virtual void insert (int, Value);
    virtual Value pop (int =-1);
    virtual void remove (Value);
    virtual void reverse ();

    void append (Value v) { insert(length(), v); }

    static MutSeqObj dummy;
protected:
    constexpr MutSeqObj () {} // cannot be instantiated directly
};

//CG< type array
struct ArrayObj : MutSeqObj {
    static Value create (const TypeObj&, int argc, Value argv[]);
    static const LookupObj attrs;
    static const TypeObj info;
    const TypeObj& type () const override;
//CG>

    void mark (void (*gc)(const Object&)) const override;

    ArrayObj (char t, size_t sz =0);

    Value repr (BufferObj&) const override; // see builtin.h
    size_t len () const override { return length(); }
    Value at (Value i) const override { return get(i); }

    bool isBuffer () const { return logBits == 3; }
    size_t write (const void* p, size_t n);

    Value get (int idx) const;
    void set (int idx, Value val);

    char atype;
    static int typeBits (char typecode);
};

//CG3 type <buffer>
struct BufferObj : Object, private VecOf<uint8_t> {
    static const TypeObj info;
    const TypeObj& type () const override;

    BufferObj (Value wfun);

    void mark (void (*gc)(const Object&)) const override;

    bool need (size_t bytes);
    void putc (uint8_t v) ;// TODO { base[fill++] = v; }
    //void put (const void* p, size_t n);

    void printf(const char* fmt, ...);

private:
    int splitInt (uint32_t val, int base, uint8_t* buf);
    void putFiller (int n, char fill);
    void putInt (int val, int base, int width, char fill);

    size_t room () const { return capacity - fill; }
    bool flush ();

    Value writer;
    uint8_t* base = 0;
};

//CG< type list
struct ListObj : MutSeqObj {
    static Value create (const TypeObj&, int argc, Value argv[]);
    static const LookupObj attrs;
    static const TypeObj info;
    const TypeObj& type () const override;
//CG>

    ListObj (int argc, Value argv[]);

    Value repr (BufferObj&) const override; // see builtin.h
    Value at (Value) const override;
};

//CG< type set
struct SetObj : ListObj {
    static Value create (const TypeObj&, int argc, Value argv[]);
    static const LookupObj attrs;
    static const TypeObj info;
    const TypeObj& type () const override;
//CG>

    SetObj (int argc, Value argv[]) : ListObj (argc, argv) {}

    Value repr (BufferObj&) const override; // see builtin.h
};

//CG< type dict
struct DictObj : MutSeqObj {
    static Value create (const TypeObj&, int argc, Value argv[]);
    static const LookupObj attrs;
    static const TypeObj info;
    const TypeObj& type () const override;
//CG>
    enum Mode { Get, Set, Del };
    const Object* chain = 0; // TODO hide

    constexpr DictObj (const Object* ch =nullptr) : chain (ch) {}
    DictObj (int size);

    void mark (void (*gc)(const Object&)) const override;

    Value repr (BufferObj&) const override; // see builtin.h
    size_t len () const override { return length() / 2; }
    Value at (Value key) const override;
    Value& atKey (Value key, Mode =Get);

    void addPair (Value k, Value v);
};

//CG3 type <type>
struct TypeObj : DictObj {
    static const TypeObj info;
    const TypeObj& type () const override;

    typedef Value (*Factory)(const TypeObj&,int,Value[]);

    const char* name;
    const Factory factory;

    constexpr TypeObj (const char* s, Factory f =noFactory, const LookupObj* a =0)
        : DictObj (a), name (s), factory (f) {}

    Value call (int argc, Value argv[]) const override;
    Value attr (const char*, Value&) const override;

private:
    static Value noFactory (const TypeObj&, int, Value[]);
};

//CG< type class
struct ClassObj : TypeObj {
    static Value create (const TypeObj&, int argc, Value argv[]);
    static const LookupObj attrs;
    static const TypeObj info;
    const TypeObj& type () const override;
//CG>

    ClassObj (int argc, Value argv[]);

    Value repr (BufferObj&) const override; // see builtin.h
};

// can't be generated, too different
struct InstanceObj : DictObj {
    static Value create (const TypeObj&, int argc, Value argv[]);

    TypeObj& type () const override { return *(TypeObj*) chain; }
    Value attr (const char*, Value&) const override;

private:
    InstanceObj (const ClassObj& parent, int argc, Value argv[]);

    Value repr (BufferObj&) const override; // see builtin.h
};

//CG3 type <function>
struct FunObj : Object {
    static const TypeObj info;
    const TypeObj& type () const override;

    typedef Value (*Func)(int,Value[]);

    constexpr FunObj (Func f) : func (f) {}

    Value call (int argc, Value argv[]) const override {
        return func(argc, argv);
    }

private:
    const Func func;
};

struct MethodBase {
    virtual Value call (Value self, int argc, Value argv[]) const =0;

    template< typename T >
    static Value argConv (Value (T::*meth)() const,
                            Value self, int argc, Value argv[]) {
        return (((T&) self.obj()).*meth)();
    }

    template< typename T >
    static Value argConv (Value (T::*meth)(),
                            Value self, int argc, Value argv[]) {
        return (((T&) self.obj()).*meth)();
    }

    template< typename T >
    static Value argConv (Value (T::*meth)(Value) const,
                            Value self, int argc, Value argv[]) {
        return (((T&) self.obj()).*meth)(argv[1]);
    }

    template< typename T >
    static Value argConv (Value (T::*meth)(Value),
                            Value self, int argc, Value argv[]) {
        return (((T&) self.obj()).*meth)(argv[1]);
    }

    template< typename T >
    static Value argConv (Value (T::*meth)(int),
                            Value self, int argc, Value argv[]) {
        return (((T&) self.obj()).*meth)(argv[1]);
    }

    template< typename T >
    static Value argConv (Value (T::*meth)(const char *),
                            Value self, int argc, Value argv[]) {
        return (((T&) self.obj()).*meth)(argv[1]);
    }

    template< typename T >
    static Value argConv (Value (T::*meth)(int, Value[]),
                            Value self, int argc, Value argv[]) {
        return (((T&) self.obj()).*meth)(argc, argv);
    }

    template< typename T >
    static Value argConv (void (T::*meth)(Value),
                            Value self, int argc, Value argv[]) {
        (((T&) self.obj()).*meth)(argv[1]);
        return Value ();
    }
};

template< typename M >
struct Method : MethodBase {
    constexpr Method (M memberPtr) : methPtr (memberPtr) {}

    Value call (Value self, int argc, Value argv[]) const override {
        return MethodBase::argConv(methPtr, *self.objPtr(), argc, argv);
    }

private:
    const M methPtr;
};

//CG3 type <method>
struct MethObj : Object {
    static const TypeObj info;
    const TypeObj& type () const override;

    constexpr MethObj (const MethodBase& m) : meth (m) {}

    Value call (int argc, Value argv[]) const override {
        return meth.call(argv[0], argc, argv);
    }

    template< typename M >
    constexpr static Method<M> wrap (M memberPtr) {
        return memberPtr;
    }

private:
    const MethodBase& meth;
};

//CG3 type <bound-meth>
struct BoundMethObj : Object {
    static const TypeObj info;
    const TypeObj& type () const override;

    constexpr BoundMethObj (Value f, Value o) : meth (f), self (o) {}

    Value call (int argc, Value argv[]) const override;

private:
    Value meth;
    Value self;
};

//CG3 type <bytecode>
struct BytecodeObj : Object {
    static const TypeObj info;
    const TypeObj& type () const override;

    ModuleObj& owner;

    OpPtrRO code;
    VecOfValue constObjs;
    int16_t stackSz;
    int16_t flags;
    int8_t excDepth;
    int8_t n_pos;
    int8_t n_kwonly;
    int8_t n_def_pos;
    int16_t hdrSz;
    int16_t size;
    int16_t nData;
    int16_t nCode;

    static BytecodeObj& create (ModuleObj& mo, int bytes);
    BytecodeObj (ModuleObj& mo) : owner (mo) {}

    bool isCoro () const { return (flags & 1) != 0; } // TODO see CallArgsObj
    int frameSize () const { return stackSz + 3 * excDepth; } // TODO three?

    void mark (void (*gc)(const Object&)) const override;
};

//CG3 type <callargs>
struct CallArgsObj : Object {
    static const TypeObj info;
    const TypeObj& type () const override;

    CallArgsObj (Value bc, TupleObj* pos =0, DictObj* kw =0)
        : bytecode (bc.asType<BytecodeObj>()), posArgs (pos), kwArgs (kw) {}

    bool isCoro () const { return (bytecode.flags & 1) != 0; }
    bool hasVarArgs () const { return (bytecode.flags & 4) != 0; }

    void mark (void (*gc)(const Object&)) const override;

    Value call (int argc, Value argv[]) const override;
    Value call (int argc, Value argv[], DictObj* dp, const Object* retVal) const;

private:
    const BytecodeObj& bytecode;
    const TupleObj* posArgs; // TODO: better: empty tuple
    const DictObj* kwArgs;   // TODO: better: empty dict
};

//CG3 type <module>
struct ModuleObj : DictObj {
    static const TypeObj info;
    const TypeObj& type () const override;

    const BytecodeObj* init = 0;

    constexpr ModuleObj (const LookupObj* lu =0) : DictObj (lu) {}

    void mark (void (*gc)(const Object&)) const override;

    Value attr (const char*, Value&) const override;
};

//CG3 type <resumable>
struct ResumableObj : Object {
    static const TypeObj info;
    const TypeObj& type () const override;

    void mark (void (*gc)(const Object&)) const override;

    virtual bool step (Value v) =0;

    Value retVal;
    ResumableObj* chain = 0;
protected:
    ResumableObj (int argc, Value argv[]) : nargs (argc), args (argv) {}

    int nargs;
    Value* args; // TODO how about vector compaction? always len 1 ? use tuple?
};

//CG3 type <frame>
struct FrameObj : DictObj {
    static const TypeObj info;
    const TypeObj& type () const override;

    const BytecodeObj& bcObj;
    FrameObj* caller = 0;
    Context* ctx = 0;
    DictObj* locals;
    const Object* result; // module/class init & resumable, TODO Value?
    uint8_t excTop = 0;

    FrameObj (const BytecodeObj& bc, DictObj* dp, const Object* retVal);

    void mark (void (*gc)(const Object&)) const override;

    Value next () override;

    // TODO careful: bottom is only correct when this frame is the top one
    Value* bottom () const;
    void leave ();

    bool isCoro () const { return bcObj.isCoro(); }
    Value& fastSlot (int n) const { return bottom()[bcObj.stackSz + ~n]; }

    Value* exceptionPushPop (int inc) {
        excTop += inc; // TODO 3 = # of stack entries per exception
        return bottom() + bcObj.stackSz + 3 * (inc < 0 ? excTop : excTop - 1);
    }

private:
    int16_t spOffset = -1;
    OpPtrRO savedIp; // could be an offset

    friend Context; // Context::flip() can access savedIp & spOffset
};

//CG3 type <context>
struct Context : Object, private VecOfValue {
    static const TypeObj info;
    const TypeObj& type () const override;

    Value* base () const { return (Value*) getPtr(0); }
    Value* limit () const { return base() + length(); }

    int extend (int num);
    void shrink (int num);

    void start (ModuleObj& mod, const LookupObj& builtins);

    static Value print (BufferObj&, Value);

    Value nextPending ();
    static void raise (Value =Value ());
    static int setHandler (Value);

    FrameObj* flip (FrameObj*);

    static void suspend (ListObj& queue);
    static void wakeUp (Value task, Value retVal =Value ());
    void resume (Value);

    void doCall (Value func, int argc, Value argv []);
    static Context* prepare (bool coro);

    static void gcCheck (bool =false); // see gc.c
    static void gcTrigger (); // may not be called from inner vm loop

    static ListObj tasks;
    bool isAlive () const;

    static DictObj modules; // see builtin.h
protected:
    Context () {}

    OpPtrRO ip = 0;
    Value* sp = 0;
    FrameObj* fp = 0;

    void popState ();
    void saveState ();
    void restoreState ();

    void mark (void (*gc)(const Object&)) const override;

    static Context* vm;
    static volatile uint32_t pending;
private:
    static constexpr auto MAX_HANDLERS = 8 * sizeof pending;

    static Value handlers [];
};
