// This is the public header for Monty, everything needed by add-ons is in here.

#include <stdint.h>
#include <stdlib.h>

struct Vector {
    uint8_t extra = 0; // available for any use
protected:
    struct Data {
        Vector* v;
        union { uint32_t n; uint8_t d [1]; };
        Data* next () const;
    };

    uint8_t logBits = 0;
    uint16_t fill = 0; // in elements
    size_t capacity = 0; // in bytes
    Data* data = 0;
public:

    Vector (size_t bits);
    ~Vector ();

    size_t length () const { return fill; }
    int width () const { auto b = 1<<logBits; return b < 8 ? -b : b/8; }

    int getInt (int idx) const;
    uint32_t getIntU (int idx) const;
    void* getPtr (int idx) const;

    void set (int idx, int val);
    void set (int idx, const void* ptr);

    void ins (int idx, int num =1);
    void del (int idx, int num =1);

    static void gcCompact ();   // see gc.c
private:
    void alloc (size_t sz);     // see gc.c
};

template< typename T >
struct VecOf : Vector {
    VecOf () : Vector (8 * sizeof (T)) {}

    T& get (int idx) const { return *(T*) getPtr(idx); }
    void set (int idx, const T& val) { Vector::set(idx, &val); }
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

struct Value {
    enum Tag { Nil, Int, Str, Obj };

    Value (int arg)           : v ((arg << 1) | 1) {}
    Value (const char* arg)   : v (((uintptr_t) arg << 2) | 2) {}
    Value (const Object* arg) : v ((uintptr_t) arg) {}
    Value (const Object& arg) : v ((uintptr_t) &arg) {}

    operator int () const { return (intptr_t) v >> 1; }
    operator const char* () const { return (const char*) (v >> 2); }
    Object& obj () const { return *(Object*) v; }
    inline ForceObj objPtr () const;

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

    bool isEq (Value) const;
    Value unOp (UnOp op) const;
    Value binOp (BinOp op, Value rhs) const;

    static const Value nil;
    static Value invalid; // special value, see DictObj::atKey
private:
    Value () : v (0) {}

    uintptr_t v;

    friend Context; // TODO yuck, just so Context::handlers [] can be inited
};

struct Object {
    virtual ~Object () {}

    virtual const TypeObj& type () const =0;
    virtual void mark (void (*gc)(const Object&)) const {};

    virtual Value repr   () const;
    virtual Value call   (int, Value[]) const;
    virtual Value unop   (UnOp) const;
    virtual Value binop  (BinOp, Value) const;
    virtual Value attr   (const char*, Value&) const;
    virtual Value at     (Value) const;
    virtual Value iter   () const;
    virtual Value next   ();

    virtual const SeqObj& asSeq () const;

    void* operator new (size_t);
    void* operator new (size_t, void*);
    void operator delete (void*);

    static void gcStats ();
};

//CG< type int
struct IntObj : Object {
    static Value create (const TypeObj&, int argc, Value argv[]);
    static const LookupObj names;
    static TypeObj info;
    TypeObj& type () const override;
//CG>

    IntObj (int v) : i (v) {}
    IntObj (unsigned v) : i (v) {}

private:
    int64_t i;
};

//CG3 type <iterator>
struct IterObj : Object {
    static TypeObj info;
    TypeObj& type () const override;

    IterObj (Value arg) : seq (arg) {}

    Value next () override;

private:
    Value seq;
    int pos = 0;
};

//CG3 type <sequence>
struct SeqObj : Object {
    static TypeObj info;
    TypeObj& type () const override;

    const SeqObj& asSeq () const override { return *this; }

    virtual Value isIn  (Value) const;
    virtual Value plus  (Value) const;
    virtual Value times (Value) const;
    virtual Value len   ()      const;
    virtual Value min   ()      const;
    virtual Value max   ()      const;
    virtual Value index (Value) const;
    virtual Value count (Value) const;

    static const SeqObj dummy;
protected:
    SeqObj () {} // cannot be instantiated directly
};

//CG< type str
struct StrObj : SeqObj {
    static Value create (const TypeObj&, int argc, Value argv[]);
    static const LookupObj names;
    static TypeObj info;
    TypeObj& type () const override;
//CG>

    StrObj (const char* v) : s (v) {}
    operator const char* () const { return s; }

    Value at (Value) const override;
    Value attr (const char*, Value&) const override;
    Value count (Value) const override { return 9; } // TODO

    static Value format (int argc, Value argv[]) { return 4; } // TODO

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
    static const LookupObj names;
    static TypeObj info;
    TypeObj& type () const override;
//CG>
    void mark (void (*gc)(const Object&)) const override;

    Value len () const override { return length; }
    Value at (Value) const override;

private:
    TupleObj (int argc, Value argv[]);

    uint16_t length;
    Value vec [];
};

//CG3 type <lookup>
struct LookupObj : SeqObj {
    static TypeObj info;
    TypeObj& type () const override;

    struct Item { const char* k; const Object* v; };

    LookupObj (const Item* p, size_t n) : vec (p), len (n) {}

    Value at (Value) const override;

private:
    const Item* vec;
    size_t len;
};

//CG3 type <mut-seq>
struct MutSeqObj : SeqObj, protected VecOf<Value> {
    static TypeObj info;
    TypeObj& type () const override;

    void mark (void (*gc)(const Object&)) const override;

    Value len () const override { return length(); }

    virtual void  insert (int, Value);
    virtual Value pop (int =-1);
    virtual void  remove (Value);
    virtual void  reverse ();

    void append (Value v) { insert(length(), v); }

    static MutSeqObj dummy;
protected:
    MutSeqObj () {} // cannot be instantiated directly
};

//CG< type list
struct ListObj : MutSeqObj {
    static Value create (const TypeObj&, int argc, Value argv[]);
    static const LookupObj names;
    static TypeObj info;
    TypeObj& type () const override;
//CG>

    ListObj (int argc, Value argv[]);

    Value at (Value) const override;
    Value attr (const char*, Value&) const override;
};

//CG< type dict
struct DictObj : MutSeqObj {
    static Value create (const TypeObj&, int argc, Value argv[]);
    static const LookupObj names;
    static TypeObj info;
    TypeObj& type () const override;
//CG>
    enum Mode { Get, Set, Del };
    const Object* chain = 0; // TODO hide

    DictObj (int size =0) {} // TODO

    Value len () const override { return length() / 2; }
    Value at (Value key) const override;
    Value& atKey (Value key, Mode =Get);
};

//CG3 type <type>
struct TypeObj : DictObj {
    static TypeObj info;
    TypeObj& type () const override;

    typedef Value (*Factory)(const TypeObj&,int,Value[]);

    const char* name;
    Factory factory;

    TypeObj (const char* s, Factory f =noFactory, const LookupObj* a =0)
        : name (s), factory (f) { chain = a; }

    Value call (int argc, Value argv[]) const override;
    Value attr (const char*, Value&) const override;

private:
    static Value noFactory (const TypeObj&, int, Value[]);
};

//CG< type class
struct ClassObj : TypeObj {
    static Value create (const TypeObj&, int argc, Value argv[]);
    static const LookupObj names;
    static TypeObj info;
    TypeObj& type () const override;
//CG>

    ClassObj (int argc, Value argv[]);
};

// can't be generated, too different
struct InstanceObj : DictObj {
    static Value create (const TypeObj&, int argc, Value argv[]);

    TypeObj& type () const override { return *(TypeObj*) chain; }
    Value attr (const char*, Value&) const override;

private:
    InstanceObj (const ClassObj& parent, int argc, Value argv[]);
};

//CG3 type <function>
struct FunObj : Object {
    static TypeObj info;
    TypeObj& type () const override;

    typedef Value (*Func)(int,Value[]);

    FunObj (Func f) : func (f) {}

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
    static Value argConv (Value (T::*meth)(Value) const,
                            Value self, int argc, Value argv[]) {
        return (((T&) self.obj()).*meth)(argv[1]);
    }

    template< typename T >
    static Value argConv (Value (T::*meth)(const char *) const,
                            Value self, int argc, Value argv[]) {
        return (((T&) self.obj()).*meth)((const char*) argv[1]);
    }

    template< typename T >
    static Value argConv (Value (T::*meth)(int, Value[]) const,
                            Value self, int argc, Value argv[]) {
        return (((T&) self.obj()).*meth)(argc, argv);
    }

    template< typename T >
    static Value argConv (void (T::*meth)(Value),
                            Value self, int argc, Value argv[]) {
        (((T&) self.obj()).*meth)(argv[1]);
        return Value::nil;
    }
};

template< typename M >
struct Method : MethodBase {
    Method (M memberPtr) : methPtr (memberPtr) {}

    Value call (Value self, int argc, Value argv[]) const override {
        return MethodBase::argConv(methPtr, *self.objPtr(), argc, argv);
    }

private:
    M methPtr;
};

//CG3 type <method>
struct MethObj : Object {
    static TypeObj info;
    TypeObj& type () const override;

    MethObj (const MethodBase& m) : meth (m) {}

    Value call (int argc, Value argv[]) const override {
        return meth.call(argv[0], argc, argv);
    }

    template< typename M >
    static Method<M> wrap (M memberPtr) {
        return memberPtr;
    }

private:
    const MethodBase& meth;
};

//CG3 type <bound-meth>
struct BoundMethObj : Object {
    static TypeObj info;
    TypeObj& type () const override;

    BoundMethObj (Value f, Value o) : meth (f), self (o) {}

    Value call (int argc, Value argv[]) const override;

private:
    Value meth;
    Value self;
};

//CG3 type <bytecode>
struct BytecodeObj : Object {
    static TypeObj info;
    TypeObj& type () const override;

    ModuleObj& owner;

    OpPtrRO code;
    VecOf<Value> constObjs;
    int16_t stackSz;
    int16_t scope;
    int8_t excDepth;
    int8_t n_pos;
    int8_t n_kwonly;
    int8_t n_def_pos;
    int16_t hdrSz;
    int16_t size;
    int16_t nData;
    int16_t nCode;

    BytecodeObj (ModuleObj& mo) : owner (mo) {}

    int frameSize () const { return stackSz + 3 * excDepth; } // TODO three?

    void mark (void (*gc)(const Object&)) const override;

    Value call (int argc, Value argv[]) const override;
};

//CG3 type <module>
struct ModuleObj : DictObj {
    static TypeObj info;
    TypeObj& type () const override;

    const BytecodeObj* init = 0;

    ModuleObj (const LookupObj* lu =0) { chain = lu; }

    void mark (void (*gc)(const Object&)) const override;

    Value call (int argc, Value argv[]) const override;
    Value attr (const char*, Value&) const override;
};

//CG3 type <frame>
struct FrameObj : DictObj {
    static TypeObj info;
    TypeObj& type () const override;

    const BytecodeObj& bcObj;
    FrameObj* caller = 0;
    Context* ctx = 0;
    DictObj* locals;
    const Object* result = 0;
    uint8_t excTop = 0;

    FrameObj (const BytecodeObj& bc, int argc, Value argv[], DictObj* dp = 0);
    ~FrameObj () override;

    void mark (void (*gc)(const Object&)) const override;

    Value next () override;

    // TODO careful: bottom is only correct when this frame is the top one
    Value* bottom () const;
    void leave ();

    bool isCoro () const { return (bcObj.scope & 1) != 0; }
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

struct Context : Object, private VecOf<Value> {
    static TypeObj info;
    TypeObj& type () const override;

    Value* base () const { return &get(0); }
    Value* limit () const { return base() + length(); }

    int extend (int num);
    void shrink (int num);

    static void print (Value);

    Value nextPending ();
    static void raise (Value);
    static int setHandler (Value);

    FrameObj* flip (FrameObj*);

    static void suspend ();
    void resume (FrameObj*);

    static Value* prepareStack (FrameObj& fo, Value* av);

    static bool gcCheck ();     // see gc.c, called from outer vm loop
    static void gcTrigger ();   // see gc.c, called from outer vm loop

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
