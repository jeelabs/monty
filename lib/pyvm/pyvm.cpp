// pyvm.cpp - virtual machine for bytecodes emitted by MicroPython 1.14

#include "monty.h"
#include <cassert>

#ifndef NOPYVM

#define SHOW_INSTR_PTR 0 // show instr ptr each time through inner loop
//CG: off op_print # set to "on" to enable per-opcode debug output

#if NATIVE
extern void timerHook ();
#define INNER_HOOK  { timerHook(); }
#else
#define INNER_HOOK
#endif

using namespace monty;

enum Op : uint8_t {
    //CG< opcodes ../../git/micropython/py/bc0.h
    LoadConstString        = 0x10,
    LoadName               = 0x11,
    LoadGlobal             = 0x12,
    LoadAttr               = 0x13,
    LoadMethod             = 0x14,
    LoadSuperMethod        = 0x15,
    StoreName              = 0x16,
    StoreGlobal            = 0x17,
    StoreAttr              = 0x18,
    DeleteName             = 0x19,
    DeleteGlobal           = 0x1A,
    ImportName             = 0x1B,
    ImportFrom             = 0x1C,
    MakeClosure            = 0x20,
    MakeClosureDefargs     = 0x21,
    LoadConstSmallInt      = 0x22,
    LoadConstObj           = 0x23,
    LoadFastN              = 0x24,
    LoadDeref              = 0x25,
    StoreFastN             = 0x26,
    StoreDeref             = 0x27,
    DeleteFast             = 0x28,
    DeleteDeref            = 0x29,
    BuildTuple             = 0x2A,
    BuildList              = 0x2B,
    BuildMap               = 0x2C,
    BuildSet               = 0x2D,
    BuildSlice             = 0x2E,
    StoreComp              = 0x2F,
    UnpackSequence         = 0x30,
    UnpackEx               = 0x31,
    MakeFunction           = 0x32,
    MakeFunctionDefargs    = 0x33,
    CallFunction           = 0x34,
    CallFunctionVarKw      = 0x35,
    CallMethod             = 0x36,
    CallMethodVarKw        = 0x37,
    UnwindJump             = 0x40,
    Jump                   = 0x42,
    PopJumpIfTrue          = 0x43,
    PopJumpIfFalse         = 0x44,
    JumpIfTrueOrPop        = 0x45,
    JumpIfFalseOrPop       = 0x46,
    SetupWith              = 0x47,
    SetupExcept            = 0x48,
    SetupFinally           = 0x49,
    PopExceptJump          = 0x4A,
    ForIter                = 0x4B,
    LoadConstFalse         = 0x50,
    LoadConstNone          = 0x51,
    LoadConstTrue          = 0x52,
    LoadNull               = 0x53,
    LoadBuildClass         = 0x54,
    LoadSubscr             = 0x55,
    StoreSubscr            = 0x56,
    DupTop                 = 0x57,
    DupTopTwo              = 0x58,
    PopTop                 = 0x59,
    RotTwo                 = 0x5A,
    RotThree               = 0x5B,
    WithCleanup            = 0x5C,
    EndFinally             = 0x5D,
    GetIter                = 0x5E,
    GetIterStack           = 0x5F,
    StoreMap               = 0x62,
    ReturnValue            = 0x63,
    RaiseLast              = 0x64,
    RaiseObj               = 0x65,
    RaiseFrom              = 0x66,
    YieldValue             = 0x67,
    YieldFrom              = 0x68,
    ImportStar             = 0x69,
    //CG>
    LoadConstSmallIntMulti = 0x70,
    LoadFastMulti          = 0xB0,
    StoreFastMulti         = 0xC0,
    UnaryOpMulti           = 0xD0,
    BinaryOpMulti          = 0xD7,
};

struct Bytecode; // forward decl

struct Callable : Object {
    static Type info;
    auto type () const -> Type const& override { return info; }

    Callable (Bytecode const& callee, Value pos, Value kw)
            : Callable (callee, nullptr, pos.ifType<Tuple>(), kw.ifType<Dict>()) {}
    Callable (Bytecode const&, Module* =nullptr, Tuple* =nullptr, Dict* =nullptr);

    auto call (ArgVec const&) const -> Value override;

    void marker () const override;

    auto funcAt (int) const -> Bytecode const&;

    Module& _mo;
    Bytecode const& _bc;
    Tuple* _pos;
    Dict* _kw;
};

#include "loader.h"

void Callable::marker () const {
    _mo.marker();
    mark(_bc);
    mark(_pos);
    mark(_kw);
}

auto Callable::funcAt (int n) const -> Bytecode const& {
    return _bc[n].asType<Bytecode>();
}

// was: CG3 type <cell>
struct Cell : Object {
    static Type info;
    auto type () const -> Type const& override { return info; }

    Cell (Value val) : _val (val) {}

    void marker () const override { _val.marker(); }

    Value _val;
};

// was: CG3 type <boundmeth>
struct BoundMeth : Object {
    static Type info;
    auto type () const -> Type const& override { return info; }

    BoundMeth (Object const& f, Value o) : _meth (f), _self (o) {}

    auto call (ArgVec const& args) const -> Value override {
        assert(args._num > 0 && this == &args[-1].obj());
        args[-1] = _self; // overwrites the entry before first arg
        return _meth.call({args._vec, (int) args._num + 1, (int) args._off - 1});
    }

    void marker () const override { mark(_meth); _self.marker(); }
private:
    Object const& _meth;
    Value _self;
};

// was: CG3 type <closure>
struct Closure : List {
    static Type info;
    auto type () const -> Type const& override { return info; }
    void repr (Buffer& buf) const override;

    Closure (Object const& func, ArgVec const& args) : _func (func) {
        insert(0, args._num);
        for (int i = 0; i < args._num; ++i)
            begin()[i] = args[i];
    }

    auto call (ArgVec const& args) const -> Value override {
        int n = size();
        assert(n > 0);
        Vector v;
        v.insert(0, n + args._num);
        for (int i = 0; i < n; ++i)
            v[i] = begin()[i];
        for (int i = 0; i < args._num; ++i)
            v[n+i] = args[i];
        return _func.call({v, n + args._num});
    }

    void marker () const override { List::marker(); mark(_func); }
private:
    Object const& _func;
};

void Closure::repr (Buffer& buf) const {
    Object::repr(buf); // don't print as a list
}

// was: CG3 type <pyvm>
struct PyVM : Stacklet {
    static Type info;
    auto type () const -> Type const& override { return info; }
    static Lookup const attrs;

    struct Frame {
        //    <------- previous ------->  <---- actual ---->
        Value base, spOff, ipOff, callee, ep, locals, result, stack [];
        // result must be just below stack for proper module/class init
    };

    auto frame () const -> Frame& { return *(Frame*) (begin() + _base); }

    auto spBase () const -> Value* { return frame().stack; }
    auto ipBase () const -> uint8_t const* { return _callee->_bc.start(); }

    auto fastSlot (uint32_t i) const -> Value& {
        return spBase()[_callee->_bc.sTop + ~i];
    }
    auto derefSlot (uint32_t i) const -> Value& {
        return fastSlot(i).asType<Cell>()._val;
    }

    static constexpr int EXC_STEP = 3; // use 3 entries per exception
    static constexpr auto FinallyFlag = 1U<<20;
    static constexpr auto FinallyMask = FinallyFlag - 1;

    auto globals () const -> Module& { return _callee->_mo; }

    void marker () const override {
        Stacklet::marker();
        mark(_callee);
        mark(_caller);
        _signal.marker();
    }

    // previous values are saved in current stack frame
    uint16_t _base = 0;
    uint16_t _spOff = 0;
    uint16_t _ipOff = 0;
    Callable const* _callee = nullptr;

    Stacklet* _caller = nullptr;
    Value _signal = {};
    Value* _sp = nullptr;
    uint8_t const* _ip = nullptr;

    auto fetchV (uint32_t v =0) -> uint32_t {
        uint8_t b = 0x80;
        while (b & 0x80) {
            b = *_ip++;
            v = (v << 7) | (b & 0x7F);
        }
        return v;
    }

    auto fetchV64 () -> uint64_t {
        uint64_t v = *_ip & 0x40 ? -1 : 0;
        uint8_t b = 0x80;
        while (b & 0x80) {
            b = *_ip++;
            v = (v << 7) | (b & 0x7F);
        }
        return v;
    }

    auto fetchO () -> int {
        int n = *_ip++;
        return n | (*_ip++ << 8);
    }

    auto fetchQ () -> Q {
        return fetchO() + 1; // TODO get rid of this off-by-one stuff
    }

    void switchTo (Stacklet* ctx) {
        assert(current == this);
        assert(ctx != this);
        current = ctx;
        setPending(0);
    }

    // special wrapper to deal with context changes vs cached sp/ip values
    template< typename T >
    auto contextAdjuster (T fun) -> Value {
        _spOff = _sp - begin();
        _ipOff = _ip - ipBase();
        Value v = fun();
        _sp = begin() + _spOff;
        _ip = ipBase() + _ipOff;
        return v;
    }

    // most common use of contextAdjuster, wraps call and saves result (or nil)
    void wrappedCall (Value callee, ArgVec const& args) {
        auto v = contextAdjuster([=]() -> Value {
            return callee->call(args);
        });
        *_sp = v;
    }

    void instructionTrace () {
#if SHOW_INSTR_PTR
        static PyVM* prevCtx;
        if (prevCtx != this) {
            if (prevCtx != nullptr) {
                printf("\tip %04d sp %2d e ? ", prevCtx->_ipOff,
                                                prevCtx->_spOff - 9);
                printf("op 0x%02x : ", *(prevCtx->_ipOff + prevCtx->ipBase()));
                if (prevCtx->_spOff >= 9)
                    (prevCtx->_spOff + prevCtx->spBase())->dump();
                printf("\n");
            }
            printf("### context changed from %p to %p ###\n", prevCtx, this);
            prevCtx = this;
        }
        printf("\tip %04d sp %2d e %d ", (int) (_ip - ipBase()),
                                         (int) (_sp - spBase()),
                                         (int) frame().ep);
        printf("op 0x%02x : ", *_ip);
        if (_sp >= spBase())
            _sp->dump();
        printf("\n");
#endif
        assert(_ip >= ipBase() && _sp >= spBase() - 1);
    }

    // check and trigger gc on backwards jumps, i.e. inside all loops
    static void loopCheck (int arg) {
        if (arg < 0 && gcCheck())
            setPending(0);
    }

    //CG: op-init

    //CG1 op
    void opLoadNull () {
        *++_sp = {};
    }
    //CG1 op
    void opLoadConstNone () {
        *++_sp = Null;
    }
    //CG1 op
    void opLoadConstFalse () {
        *++_sp = False;
    }
    //CG1 op
    void opLoadConstTrue () {
        *++_sp = True;
    }
    //CG1 op q
    void opLoadConstString (Q arg) {
        *++_sp = arg;
    }
    //CG1 op
    void opLoadConstSmallInt () {
        *++_sp = Int::make(fetchV64());
    }
    //CG1 op v
    void opLoadConstObj (int arg) {
        *++_sp = _callee->_bc[arg];
    }
    //CG1 op v
    void opLoadFastN (int arg) {
        *++_sp = fastSlot(arg);
    }
    //CG1 op v
    void opStoreFastN (int arg) {
        fastSlot(arg) = *_sp--;
    }
    //CG1 op v
    void opDeleteFast (int arg) {
        fastSlot(arg) = {};
    }

    //CG1 op
    void opDupTop () {
        ++_sp; _sp[0] = _sp[-1];
    }
    //CG1 op
    void opDupTopTwo () {
        _sp += 2; _sp[0] = _sp[-2]; _sp[-1] = _sp[-3];
    }
    //CG1 op
    void opPopTop () {
        --_sp;
    }
    //CG1 op
    void opRotTwo () {
        auto v = _sp[0]; _sp[0] = _sp[-1]; _sp[-1] = v;
    }
    //CG1 op
    void opRotThree () {
        auto v = _sp[0]; _sp[0] = _sp[-1]; _sp[-1] = _sp[-2]; _sp[-2] = v;
    }

    //CG1 op s
    void opJump (int arg) {
        _ip += arg;
    }
    //CG1 op s
    void opPopJumpIfFalse (int arg) {
        if (!_sp->truthy())
            _ip += arg;
        --_sp;
    }
    //CG1 op s
    void opJumpIfFalseOrPop (int arg) {
        if (!_sp->truthy())
            _ip += arg;
        else
            --_sp;
    }
    //CG1 op s
    void opPopJumpIfTrue (int arg) {
        if (_sp->truthy())
            _ip += arg;
        --_sp;
    }
    //CG1 op s
    void opJumpIfTrueOrPop (int arg) {
        if (_sp->truthy())
            _ip += arg;
        else
            --_sp;
    }

    //CG1 op q
    void opLoadName (Q arg) {
        *++_sp = frame().locals->getAt(arg);
        if (_sp->isNil())
            *_sp = {E::NameError, arg};
    }
    //CG1 op q
    void opStoreName (Q arg) {
        auto& l = frame().locals;
        if (!l.isObj())
            l = new Dict (&globals());
        l->setAt(arg, *_sp--);
    }
    //CG1 op q
    void opDeleteName (Q arg) {
        frame().locals->setAt(arg, {});
    }
    //CG1 op q
    void opLoadGlobal (Q arg) {
        *++_sp = globals().at(arg);
        assert(!_sp->isNil());
    }
    //CG1 op q
    void opStoreGlobal (Q arg) {
        globals().at(arg) = *_sp--;
    }
    //CG1 op q
    void opDeleteGlobal (Q arg) {
        globals().at(arg) = {};
    }
    //CG1 op q
    void opLoadAttr (Q arg) {
        Value self;
        Value v = _sp->obj().attr(arg, self);
        if (v.isNil())
            *_sp = {E::AttributeError, arg, _sp->obj().type()._name};
        else {
            *_sp = v;
            // TODO should this be moved into Inst::attr ???
            auto f = _sp->ifType<Callable>();
            if (!self.isNil() && f != 0)
                *_sp = new BoundMeth (*f, self);
        }
    }
    //CG1 op q
    void opStoreAttr (Q arg) {
        _sp->obj().setAt(arg, _sp[-1]);
        _sp -= 2;
    }
    //CG1 op
    void opLoadSubscr () {
        --_sp;
        *_sp = _sp->asObj().getAt(_sp[1]);
        if (_sp->isNil())
            *_sp = {E::KeyError, _sp[1]};
    }
    //CG1 op
    void opStoreSubscr () {
        --_sp; // val [obj] key
        assert(_sp->isObj());
        _sp->obj().setAt(_sp[1], _sp[-1]);
        _sp -= 2;
    }

    //CG1 op v
    void opBuildSlice (int arg) {
        _sp -= arg - 1;
        *_sp = Slice::create({*this, arg, (int) (_sp - begin())});
    }
    //CG1 op v
    void opBuildTuple (int arg) {
        _sp -= arg - 1;
        *_sp = Tuple::create({*this, arg, (int) (_sp - begin())});
    }
    //CG1 op v
    void opBuildList (int arg) {
        _sp -= arg - 1;
        *_sp = List::create({*this, arg, (int) (_sp - begin())});
    }
    //CG1 op v
    void opBuildSet (int arg) {
        _sp -= arg - 1;
        *_sp = Set::create({*this, arg, (int) (_sp - begin())});
    }
    //CG1 op v
    void opBuildMap (int arg) {
        *++_sp = Dict::create({*this, arg});
    }
    //CG1 op
    void opStoreMap () {
        _sp -= 2;
        _sp->obj().setAt(_sp[2], _sp[1]); // TODO optimise later: no key check
    }
    //CG1 op v
    void opStoreComp (int arg) {
        (void) arg; assert(false); // TODO
    }
    //CG1 op v
    void opUnpackSequence (int arg) {
        auto& seq = _sp->obj(); // TODO iterators
        if ((int) seq.len() != arg)
            *_sp = {E::ValueError, "unpack count mismatch", (int) seq.len()};
        else {
            for (int i = 0; i < arg; ++i)
                _sp[arg-i-1] = seq.getAt(i);
            _sp += arg - 1;
        }
    }
    //CG1 op v
    void opUnpackEx (int arg) {
        auto& seq = _sp->asType<List>(); // TODO tuples and iterators
        uint8_t left = arg, right = arg >> 8;
        int got = seq.len();
        if (got < left + right)
            *_sp = {E::ValueError, "unpack needs more items", got};
        else {
            for (int i = 0; i < right; ++i)
                _sp[i] = seq.getAt(got-i-1);
            _sp[right] = List::create({seq, got - left - right, left});
            for (int i = 0; i < left; ++i)
                _sp[right+1+i] = seq.getAt(left-i-1);
            _sp += left + right;
        }
    }

    //CG1 op o
    void opSetupExcept (int arg) {
        auto exc = excBase(1);
        exc[0] = _ip - ipBase() + arg;
        exc[1] = _sp - begin();
        exc[2] = {};
    }
    //CG1 op o
    void opSetupFinally (int arg) {
        opSetupExcept(arg | FinallyFlag);
    }
    //CG1 op
    void opEndFinally () {
        excBase(-1);
        if (_sp->isNone())
            --_sp;
        else if (!_sp->isInt())
            opRaiseObj();
        else if (*_sp < 0) {
            --_sp;
            opReturnValue();
        } else
            assert(false); // TODO unwind jump
    }
    //CG1 op o
    void opSetupWith (int arg) {
        auto exit = Q( 13,"__exit__");
        _sp[1] = {};
        *_sp = _sp->obj().attr(exit, _sp[1]);
        if (_sp->isNil()) {
            *_sp = {E::AttributeError, exit};
            return;
        }

        auto entry = Q( 12,"__enter__");
        _sp[2] = _sp[1]->attr(entry, _sp[3]);
        if (_sp->isNil()) {
            _sp[2] = {E::AttributeError, entry};
            return;
        }

        ++_sp;
        opSetupExcept(arg);
        ++_sp;

        wrappedCall(*_sp, {*this, 1, _sp});
    }
    //CG1 op
    void opWithCleanup () {
        assert(_sp->isNone()); // TODO other cases
        _sp[1] = Null;
        _sp[2] = Null;
        _sp -= 2;

        wrappedCall(*_sp, {*this, 4, _sp + 1});
    }
    //CG1 op o
    void opPopExceptJump (int arg) {
        excBase(-1);
        _ip += arg;
    }
    //CG1 op
    void opRaiseLast () {
        // TODO re-raise previous exception, if there is one, else:
        Value ({E::RuntimeError, "no active exception"});
    }
    //CG1 op
    void opRaiseObj () {
        raise(*_sp);
    }
    //CG1 op
    void opRaiseFrom () {
        raise(*--_sp); // exception chaining is not supported
    }
    //CG1 op s
    void opUnwindJump (int arg) {
        int ep = frame().ep;
        frame().ep = ep - *_ip; // TODO hardwired for simplest case
        _ip += arg;
    }

    //CG1 op
    void opLoadBuildClass () {
        *++_sp = Class::info;
    }
    //CG1 op q
    void opLoadMethod (Q arg) {
        _sp[1] = {};
        auto v = _sp->asObj().attr(arg, _sp[1]);
        if (v.isNil()) // TODO duplicate code, move test & exception into attr?
            *_sp = {E::AttributeError, arg, _sp->asObj().type()._name};
        else
            *_sp = v;
        ++_sp;
    }
    //CG1 op q
    void opLoadSuperMethod (Q arg) {
        --_sp;
        _sp[-1] = _sp->obj().getAt(arg);
        *_sp = _sp[1];
    }
    //CG1 op v
    void opCallMethod (int arg) {
        uint8_t npos = arg, nkw = arg >> 8;
        _sp -= npos + 2 * nkw + 1;
        auto skip = _sp[1].isNil();
        wrappedCall(*_sp, {*this, arg + 1 - skip, _sp + 1 + skip});
    }
    //CG1 op v
    void opCallMethodVarKw (int arg) {
        // TODO some duplication w.r.t. opCallMethod
        uint8_t npos = arg, nkw = arg >> 8;
        _sp -= npos + 2 * nkw + 3;
        auto skip = _sp[1].isNil();
        wrappedCall(*_sp, {*this, arg + 1 - skip + (1<<16), _sp + 1 + skip});
    }
    //CG1 op v
    void opMakeFunction (int arg) {
        *++_sp = new Callable (_callee->funcAt(arg));
    }
    //CG1 op v
    void opMakeFunctionDefargs (int arg) {
        --_sp;
        *_sp = new Callable (_callee->funcAt(arg), _sp[0], _sp[1]);
    }
    //CG1 op v
    void opCallFunction (int arg) {
        uint8_t npos = arg, nkw = arg >> 8;
        _sp -= npos + 2 * nkw;
        auto isClass = &_sp->obj() == &Class::info;
        wrappedCall(*_sp, {*this, arg, _sp + 1});
        // TODO yuck, special cased because Class doesn't have access to PyVM
        if (isClass)
            frame().locals = *_sp;
    }
    //CG1 op v
    void opCallFunctionVarKw (int arg) {
        // TODO some duplication w.r.t. opCallFunction
        uint8_t npos = arg, nkw = arg >> 8;
        _sp -= npos + 2 * nkw + 2;
        wrappedCall(*_sp, {*this, arg + (1<<16), _sp + 1});
    }

    //CG1 op v
    void opMakeClosure (int arg) {
        int num = *_ip++;
        _sp -= num - 1;
        auto f = new Callable (_callee->funcAt(arg));
        *_sp = new Closure (*f, {*this, num, _sp});
    }
    //CG1 op v
    void opMakeClosureDefargs (int arg) {
        int num = *_ip++;
        _sp -= 2 + num - 1;
        auto f = new Callable (_callee->funcAt(arg), _sp[0], _sp[1]);
        *_sp = new Closure (*f, {*this, num, _sp + 2});
    }
    //CG1 op v
    void opLoadDeref (int arg) {
        *++_sp = derefSlot(arg);
        assert(!_sp->isNil());
    }
    //CG1 op v
    void opStoreDeref (int arg) {
        derefSlot(arg) = *_sp--;
    }
    //CG1 op v
    void opDeleteDeref (int arg) {
        derefSlot(arg) = {};
    }

    //CG1 op
    void opYieldValue () {
        assert(_caller != nullptr);
        assert(_caller != this);
        auto& myCaller = Value(_caller).asType<PyVM>(); // TODO non-PyVM caller ?
        _caller = nullptr;
        // TODO messy: result needs to be stored in another PyVM instance
        //  might be better to store in its signal slot, then pick up on resume
        myCaller[myCaller._spOff] = *_sp;
        frame().result = *_sp; // TODO one of these two is redundant
        switchTo(&myCaller);
    }
    //CG1 op
    void opYieldFrom () {
        --_sp;
        auto& child = _sp->asType<PyVM>();
        assert(child._caller == nullptr);
#if 1
        // FIXME not sure, shouldn't "yield from" pull out of the call chain?
        child._caller = this;
#else
        assert(_caller != nullptr);
        child._caller = _caller;
        _caller = nullptr;
#endif
        switchTo(nullptr);
    }
    //CG1 op
    void opReturnValue () {
        auto v = contextAdjuster([=]() -> Value {
            return leave(*_sp);
        });
        *_sp = v;
    }

    //CG1 op
    void opGetIter () {
        auto v = _sp->obj().iter();
        if (v.isInt())
            v = new Iterator (_sp->obj(), v);
        *_sp = v;
    }
    //CG1 op
    void opGetIterStack () {
        // hard-coded to use 4 entries, layout [seq,(idx|iter),nil,nil]
        *_sp = _sp->asObj(); // for qstrs, etc
        auto v = _sp->obj().iter(); // will be 0 for indexed iteration
        *++_sp = v;
        *++_sp = {};
        *++_sp = {};
    }
    //CG1 op o
    void opForIter (int arg) {
        Value v;
        auto& pos = _sp[-2];
        if (pos.isInt()) {
            assert(_sp[-3].isObj());
            auto& seq = _sp[-3].obj();
            int n = pos;
            if (n < (int) seq.len()) {
                if (&seq.type() == &Dict::info || &seq.type() == &Set::info)
                    v = ((List&) seq)[n]; // avoid keyed access
                else
                    v = seq.getAt(n);
                pos = n + 1;
            }
        } else
            v = pos->next();
        if (v.isNil()) {
            _sp -= 4;
            _ip += arg;
        } else
            *++_sp = v;
    }

    //CG1 op q
    void opImportName (Q arg) {
        --_sp; // TODO ignore fromlist for now, *_sp level also ignored
        Value v = Module::loaded.at(arg);
        if (v.isNil()) {
            auto data = vmImport(arg);
            if (data != nullptr) {
                auto init = Bytecode::load(data, arg);
                assert(init != nullptr);
                wrappedCall(init, {*this, 0});
                frame().locals = v = init->_mo;
                Module::loaded.at(arg) = &init->_mo;
            } else
                v = {E::ImportError, arg};
        }
        *_sp = v;
    }
    //CG1 op q
    void opImportFrom (Q arg) {
        Value v = _sp->obj().getAt(arg);
        *++_sp = v;
    }
    //CG1 op
    void opImportStar () {
        auto& dest = globals();
        auto& from = _sp->asType<Module>();
        for (uint8_t i = 0; i < from.len(); ++i) {
            auto k = from[i];
            assert(k.isStr());
            if (*((char const*) k) != '_')
                dest.setAt(k, from.at(k));
        }
        --_sp;
    }

    //CG1 op m 64
    void opLoadConstSmallIntMulti (uint32_t arg) {
        *++_sp = arg - 16;
    }
    //CG1 op m 16
    void opLoadFastMulti (uint32_t arg) {
        *++_sp = fastSlot(arg);
        assert(!_sp->isNil());
    }
    //CG1 op m 16
    void opStoreFastMulti (uint32_t arg) {
        fastSlot(arg) = *_sp--;
    }
    //CG1 op m 7
    void opUnaryOpMulti (uint32_t arg) {
        *_sp = _sp->unOp((UnOp) arg);
    }
    //CG1 op m 35
    void opBinaryOpMulti (uint32_t arg) {
        --_sp;
        *_sp = _sp->binOp((BinOp) arg, _sp[1]);
    }

    void inner () {
        _sp = begin() + _spOff;
        _ip = ipBase() + _ipOff;

        do {
            INNER_HOOK  // used for simulated time in native builds
            instructionTrace();
            switch ((Op) *_ip++) {

                //CG< op-emit d
                case Op::LoadNull:
                    opLoadNull();
                    break;
                case Op::LoadConstNone:
                    opLoadConstNone();
                    break;
                case Op::LoadConstFalse:
                    opLoadConstFalse();
                    break;
                case Op::LoadConstTrue:
                    opLoadConstTrue();
                    break;
                case Op::LoadConstString: {
                    Q arg = fetchQ();
                    opLoadConstString(arg);
                    break;
                }
                case Op::LoadConstSmallInt:
                    opLoadConstSmallInt();
                    break;
                case Op::LoadConstObj: {
                    int arg = fetchV();
                    opLoadConstObj(arg);
                    break;
                }
                case Op::LoadFastN: {
                    int arg = fetchV();
                    opLoadFastN(arg);
                    break;
                }
                case Op::StoreFastN: {
                    int arg = fetchV();
                    opStoreFastN(arg);
                    break;
                }
                case Op::DeleteFast: {
                    int arg = fetchV();
                    opDeleteFast(arg);
                    break;
                }
                case Op::DupTop:
                    opDupTop();
                    break;
                case Op::DupTopTwo:
                    opDupTopTwo();
                    break;
                case Op::PopTop:
                    opPopTop();
                    break;
                case Op::RotTwo:
                    opRotTwo();
                    break;
                case Op::RotThree:
                    opRotThree();
                    break;
                case Op::Jump: {
                    int arg = fetchO()-0x8000;
                    opJump(arg);
                    loopCheck(arg);
                    break;
                }
                case Op::PopJumpIfFalse: {
                    int arg = fetchO()-0x8000;
                    opPopJumpIfFalse(arg);
                    loopCheck(arg);
                    break;
                }
                case Op::JumpIfFalseOrPop: {
                    int arg = fetchO()-0x8000;
                    opJumpIfFalseOrPop(arg);
                    loopCheck(arg);
                    break;
                }
                case Op::PopJumpIfTrue: {
                    int arg = fetchO()-0x8000;
                    opPopJumpIfTrue(arg);
                    loopCheck(arg);
                    break;
                }
                case Op::JumpIfTrueOrPop: {
                    int arg = fetchO()-0x8000;
                    opJumpIfTrueOrPop(arg);
                    loopCheck(arg);
                    break;
                }
                case Op::LoadName: {
                    Q arg = fetchQ();
                    opLoadName(arg);
                    break;
                }
                case Op::StoreName: {
                    Q arg = fetchQ();
                    opStoreName(arg);
                    break;
                }
                case Op::DeleteName: {
                    Q arg = fetchQ();
                    opDeleteName(arg);
                    break;
                }
                case Op::LoadGlobal: {
                    Q arg = fetchQ();
                    opLoadGlobal(arg);
                    break;
                }
                case Op::StoreGlobal: {
                    Q arg = fetchQ();
                    opStoreGlobal(arg);
                    break;
                }
                case Op::DeleteGlobal: {
                    Q arg = fetchQ();
                    opDeleteGlobal(arg);
                    break;
                }
                case Op::LoadAttr: {
                    Q arg = fetchQ();
                    opLoadAttr(arg);
                    break;
                }
                case Op::StoreAttr: {
                    Q arg = fetchQ();
                    opStoreAttr(arg);
                    break;
                }
                case Op::LoadSubscr:
                    opLoadSubscr();
                    break;
                case Op::StoreSubscr:
                    opStoreSubscr();
                    break;
                case Op::BuildSlice: {
                    int arg = fetchV();
                    opBuildSlice(arg);
                    break;
                }
                case Op::BuildTuple: {
                    int arg = fetchV();
                    opBuildTuple(arg);
                    break;
                }
                case Op::BuildList: {
                    int arg = fetchV();
                    opBuildList(arg);
                    break;
                }
                case Op::BuildSet: {
                    int arg = fetchV();
                    opBuildSet(arg);
                    break;
                }
                case Op::BuildMap: {
                    int arg = fetchV();
                    opBuildMap(arg);
                    break;
                }
                case Op::StoreMap:
                    opStoreMap();
                    break;
                case Op::StoreComp: {
                    int arg = fetchV();
                    opStoreComp(arg);
                    break;
                }
                case Op::UnpackSequence: {
                    int arg = fetchV();
                    opUnpackSequence(arg);
                    break;
                }
                case Op::UnpackEx: {
                    int arg = fetchV();
                    opUnpackEx(arg);
                    break;
                }
                case Op::SetupExcept: {
                    int arg = fetchO();
                    opSetupExcept(arg);
                    break;
                }
                case Op::SetupFinally: {
                    int arg = fetchO();
                    opSetupFinally(arg);
                    break;
                }
                case Op::EndFinally:
                    opEndFinally();
                    break;
                case Op::SetupWith: {
                    int arg = fetchO();
                    opSetupWith(arg);
                    break;
                }
                case Op::WithCleanup:
                    opWithCleanup();
                    break;
                case Op::PopExceptJump: {
                    int arg = fetchO();
                    opPopExceptJump(arg);
                    break;
                }
                case Op::RaiseLast:
                    opRaiseLast();
                    break;
                case Op::RaiseObj:
                    opRaiseObj();
                    break;
                case Op::RaiseFrom:
                    opRaiseFrom();
                    break;
                case Op::UnwindJump: {
                    int arg = fetchO()-0x8000;
                    opUnwindJump(arg);
                    loopCheck(arg);
                    break;
                }
                case Op::LoadBuildClass:
                    opLoadBuildClass();
                    break;
                case Op::LoadMethod: {
                    Q arg = fetchQ();
                    opLoadMethod(arg);
                    break;
                }
                case Op::LoadSuperMethod: {
                    Q arg = fetchQ();
                    opLoadSuperMethod(arg);
                    break;
                }
                case Op::CallMethod: {
                    int arg = fetchV();
                    opCallMethod(arg);
                    break;
                }
                case Op::CallMethodVarKw: {
                    int arg = fetchV();
                    opCallMethodVarKw(arg);
                    break;
                }
                case Op::MakeFunction: {
                    int arg = fetchV();
                    opMakeFunction(arg);
                    break;
                }
                case Op::MakeFunctionDefargs: {
                    int arg = fetchV();
                    opMakeFunctionDefargs(arg);
                    break;
                }
                case Op::CallFunction: {
                    int arg = fetchV();
                    opCallFunction(arg);
                    break;
                }
                case Op::CallFunctionVarKw: {
                    int arg = fetchV();
                    opCallFunctionVarKw(arg);
                    break;
                }
                case Op::MakeClosure: {
                    int arg = fetchV();
                    opMakeClosure(arg);
                    break;
                }
                case Op::MakeClosureDefargs: {
                    int arg = fetchV();
                    opMakeClosureDefargs(arg);
                    break;
                }
                case Op::LoadDeref: {
                    int arg = fetchV();
                    opLoadDeref(arg);
                    break;
                }
                case Op::StoreDeref: {
                    int arg = fetchV();
                    opStoreDeref(arg);
                    break;
                }
                case Op::DeleteDeref: {
                    int arg = fetchV();
                    opDeleteDeref(arg);
                    break;
                }
                case Op::YieldValue:
                    opYieldValue();
                    break;
                case Op::YieldFrom:
                    opYieldFrom();
                    break;
                case Op::ReturnValue:
                    opReturnValue();
                    break;
                case Op::GetIter:
                    opGetIter();
                    break;
                case Op::GetIterStack:
                    opGetIterStack();
                    break;
                case Op::ForIter: {
                    int arg = fetchO();
                    opForIter(arg);
                    break;
                }
                case Op::ImportName: {
                    Q arg = fetchQ();
                    opImportName(arg);
                    break;
                }
                case Op::ImportFrom: {
                    Q arg = fetchQ();
                    opImportFrom(arg);
                    break;
                }
                case Op::ImportStar:
                    opImportStar();
                    break;
                //CG>

                default: {
                    //CG< op-emit m
                    if ((uint32_t) (_ip[-1] - Op::LoadConstSmallIntMulti) < 64) {
                        uint32_t arg = _ip[-1] - Op::LoadConstSmallIntMulti;
                        opLoadConstSmallIntMulti(arg);
                        break;
                    }
                    if ((uint32_t) (_ip[-1] - Op::LoadFastMulti) < 16) {
                        uint32_t arg = _ip[-1] - Op::LoadFastMulti;
                        opLoadFastMulti(arg);
                        break;
                    }
                    if ((uint32_t) (_ip[-1] - Op::StoreFastMulti) < 16) {
                        uint32_t arg = _ip[-1] - Op::StoreFastMulti;
                        opStoreFastMulti(arg);
                        break;
                    }
                    if ((uint32_t) (_ip[-1] - Op::UnaryOpMulti) < 7) {
                        uint32_t arg = _ip[-1] - Op::UnaryOpMulti;
                        opUnaryOpMulti(arg);
                        break;
                    }
                    if ((uint32_t) (_ip[-1] - Op::BinaryOpMulti) < 35) {
                        uint32_t arg = _ip[-1] - Op::BinaryOpMulti;
                        opBinaryOpMulti(arg);
                        break;
                    }
                    //CG>
                    assert(false);
                }
            }

        } while (pending == 0);

        _spOff = _sp - begin();
        _ipOff = _ip - ipBase();

        if (pending & (1<<0))
            caught();

        if (current != this)
            return; // last frame popped, there's no context left
    }

    void enter (Callable const& func) {
        auto frameSize = func._bc.sTop + EXC_STEP * func._bc.nExc;
        int need = (frame().stack + frameSize) - (begin() + _base);

        auto curr = _base;          // current frame offset
        _base = _fill;              // new frame offset
        insert(_fill, need);        // make room

        auto& f = frame();          // new frame
        f.base = curr;              // index of (now previous) frame
        f.spOff = _spOff;           // previous stack index
        f.ipOff = _ipOff;           // previous instruction index
        f.callee = _callee;         // previous callable

        _spOff = f.stack-begin()-1; // stack starts out empty
        _ipOff = 0;                 // code starts at first opcode
        _callee = &func;            // new callable context
        f.ep = 0;                   // no exceptions pending
    }

    Value leave (Value v ={}) {
        auto& f = frame();
        auto r = f.result;          // stored result
        if (r.isNil())              // use return result if set
            r = v;                  // ... else arg

        if (_base > 0) {
            int prev = f.base;      // previous frame offset
            _spOff = f.spOff;       // restore stack index
            _ipOff = f.ipOff;       // restore instruction index
            _callee = &f.callee.asType<Callable>(); // restore callee

            assert(_fill > _base);
            remove(_base, _fill - _base); // delete current frame

            assert(prev >= 0);
            _base = prev;            // new lower frame offset
        } else {
            // last frame, drop context, restore caller
            _fill = 0; // delete stack entries
            adj(1); // release vector FIXME crashes when set to 0 (???)
            switchTo(_caller);
        }

        return r;
    }

    auto excBase (int incr) -> Value* {
        uint32_t ep = frame().ep;
        frame().ep = ep + incr;
        if (incr <= 0)
            --ep;
        return frame().stack + _callee->_bc.sTop + EXC_STEP * ep;
    }

    void caught () {
        auto e = _signal.take();
        if (e.isNil())
            return; // there was no exception, just an inner loop exit

        // quirky: "assert" without 2nd arg does not construct the exception
        // and "raise Exception" is also allowed, iso "raise Exception()" ...
        if (e.ifType<Function>())
            e = e->call({}); // so construct it now

        auto& einfo = e.asType<Exception>();

        // finally clauses and re-raises must not extend the trace
        // _ipOff can be zero if the error comes from inside Callable::call
        if (_ip[-1] != EndFinally && _ip[-1] != RaiseLast && _ipOff > 0)
            einfo.addTrace(_ipOff - 1, _callee->_bc);

        if (frame().ep > 0) { // simple exception, no stack unwind
            auto ep = excBase(0);
            _ipOff = ep[0] & FinallyMask;
            _spOff = ep[1];
            begin()[++_spOff] = e.isNil() ? ep[2] : e;
        } else {
            leave();
            if (current != nullptr)
                current->raise(e);
            else {
                Buffer buf;
                buf.print("uncaught exception: ");
                e->repr(buf);
                buf.puts("\n  ");
                einfo.trace()->repr(buf);
                buf.putc('\n');
            }
        }
    }

    PyVM (Callable const& clb) {
        enter(clb);
        frame().locals = &clb._mo;
    }

    auto run () -> bool override {
        while (current == this) {
            inner();
            if ((pending & (1<<0)) && gcCheck())
                gcAll();
        }
        return false;
    }

    auto iter () const -> Value override { return this; }

    auto next () -> Value override { return send(); }

    //CG: wrap PyVM send
    auto send (Value arg =Null) -> Value {
        if (_fill == 0)
            return {};
        assert(current != nullptr);
        assert(current != this);
        assert(_caller == nullptr);
        _caller = current;
        current = nullptr;
        ready.push(this);
        setPending(0);
        // TODO messy: arg needs to be stored at the top of the stack
        //  see also YieldValue
        (*this)[_spOff] = arg;
        return {}; // no result yet
    }

    void raise (Value exc) override {
        uint32_t num = 0;
        if (exc.isInt())
            num = exc;      // trigger soft-irq 1..31 (interrupt-safe)
        else
            _signal = exc;  // trigger exception or other outer-loop req
        setPending(num);    // force inner loop exit
    }

    auto argSetup (ArgVec const& args) -> Value {
        auto& _bc = _callee->_bc;
        auto& _pos = _callee->_pos;
        auto& _kw = _callee->_kw;

        auto nPos = _bc.nPos;
        auto nDef = _bc.nDef;
        auto nKwo = _bc.nKwo;
        auto nCel = _bc.nCel;
        bool hva = _bc.hasVarArgs();

        auto aPos = args._num & 0xFF;
        auto aKwd = (args._num >> 8) & 0xFF;
        auto more = args._num & (1<<16); // hidden args for "*seq" and "**map"

        Value xSeq, xMap;
        if (more) {
            xSeq = args[aPos+2*aKwd];
            xMap = args[aPos+2*aKwd+1];
        }
#if 0
        printf("args 0x%x nPos %d nDef %d nKwo %d nCel %d _pos %p _kw %p hva %d\n",
                args._num, nPos, nDef, nKwo, nCel, _pos, _kw, hva);
        if (more) {
            xSeq.dump("xSeq");
            xMap.dump("xMap");
        }
#endif
        if (!hva && aPos > nPos + nCel)
            return {E::TypeError, "too many positional args", aPos};

        auto posVec = hva ? new Tuple : nullptr;
        int idx = 0;

        auto addArg = [&](Value v) {
            if (idx < nPos + nCel)
                fastSlot(idx) = v;
            else
                posVec->append(v);
            ++idx;
        };

        if (_kw != nullptr) // preset args with kw defaults
            for (int j = 0; j < nPos + nKwo; ++j) {
                assert(_bc[j].isStr());
                Value v = _kw->at(_bc[j]);
                if (!v.isNil())
                    fastSlot(j) = v;
            }

        while (idx < aPos)
            addArg(args[idx]);

        if (xSeq.isObj()) {
            Value vit = xSeq->iter();
            Iterator it (xSeq.obj());
            auto& vob = vit.isInt() ? it : xSeq.obj();
            while (true) {
                auto v = vob.next();
                if (v.isNil()) {
                    current = this;
                    suspend();
                    // TODO messy result passing, perhaps suspend should do it?
                    v = _sp->take();
                    if (v.isNil())
                        break;
                }
                addArg(v);
            }
        }
        for (auto i = idx; i < nPos + nCel; ++i)
            if (_pos != nullptr && nPos-nDef <= i && i < nDef+(int)_pos->_fill)
                fastSlot(i) = (*_pos)[i+nDef-nPos];

        uint32_t seen = 0; assert(aKwd <= 32); // due to using "seen" as a bitmap

        for (int i = 0; i < aKwd; ++i) {
            auto off = aPos + 2*i;
            auto name = args[off];
            assert(name.isStr()); // TODO is this certain to be a qstr?
            for (int j = 0; j < nPos + nKwo; ++j) {
                assert(_bc[j].isStr());
                if (name.id() == _bc[j].id()) {
                    fastSlot(j) = args[off+1];
                    seen |= 1 << i; // key has been used, forget about it
                    break;
                }
            }
        }

        for (int i = 0; i < nPos + nKwo; ++i)
            if (fastSlot(i).isNil())
                return {E::TypeError, "required arg missing", _bc[i]};

        if (hva)
            fastSlot(nPos+nKwo) = posVec;

        if (aKwd > nKwo) {
            auto d = new Dict;
            for (int i = 0; i < aKwd; ++i)
                if ((seen & (1<<i)) == 0) {
                    auto off = aPos + 2*i;
                    d->at(args[off]) = args[off+1];
                }
            fastSlot(nPos+nKwo+hva) = d;
        }

        uint8_t const* cellMap = _bc.start() - nCel;
        for (int i = 0; i < nCel; ++i) {
            auto slot = cellMap[i];
            fastSlot(slot) = new Cell (fastSlot(slot));
        }

        return _bc.isGenerator() ? this : Value {};
    }
};

static auto currentVM () -> PyVM& {
    Value v = Stacklet::current;
    return v.asType<PyVM>(); // TODO yuck
}

Callable::Callable (Bytecode const& callee, Module* mod, Tuple* t, Dict* d)
        : _mo (mod != nullptr ? *mod : currentVM().globals()),
          _bc (callee), _pos (t), _kw (d) {
}

auto Callable::call (ArgVec const& args) const -> Value {
    PyVM* ctx;
    if (_bc.isGenerator())
        ctx = new PyVM (*this);
    else {
        ctx = &currentVM();
        ctx->enter(*this);
    }
    return ctx->argSetup(args);
}

Type  Bytecode::info (Q(181,"<bytecode>"));
Type  Callable::info (Q(182,"<callable>"));
Type      Cell::info (Q(183,"<cell>"));
Type BoundMeth::info (Q(184,"<boundmeth>"));
Type   Closure::info (Q(185,"<closure>"));

//CG< wrappers PyVM
static auto const m_pyvm_send = Method::wrap(&PyVM::send);
static Method const mo_pyvm_send (m_pyvm_send);

static Lookup::Item const pyvm_map [] = {
    { Q(138,"send"), mo_pyvm_send },
};
Lookup const PyVM::attrs (pyvm_map);
//CG>

Type PyVM::info (Q(186,"<pyvm>"), &PyVM::attrs);

auto monty::vmLaunch (void const* data) -> Stacklet* {
    if (data == nullptr)
        return nullptr;
    auto init = Bytecode::load(data, Q( 21,"__main__"));
    if (init == nullptr) {
        auto mpy = vmImport((char const*) data);
        if (mpy != nullptr)
            init = Bytecode::load(mpy, Q( 21,"__main__"));
        if (init == nullptr)
            return nullptr;
    }
    return new PyVM (*init);
}

#else
auto monty::vmLaunch (void const* data) -> Stacklet* {
    return nullptr;
}
#endif
