// pyvm.cpp - virtual machine for bytecodes emitted by MicroPython 1.13

#include "monty.h"
#include <cassert>

#define SHOW_INSTR_PTR 0 // show instr ptr each time through inner loop
//CG: off op:print # set to "on" to enable per-opcode debug output

#if NATIVE
namespace machine { void timerHook (); }
#define INNER_HOOK  { machine::timerHook(); }
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
    auto type () const -> Type const& override;

    Callable (Bytecode const& callee, Value pos, Value kw)
            : Callable (callee, nullptr, pos.ifType<Tuple>(), kw.ifType<Dict>()) {}
    Callable (Bytecode const&, Module* =nullptr, Tuple* =nullptr, Dict* =nullptr);

    auto call (ArgVec const&) const -> Value override;

    void marker () const override;

    auto funcAt (int) const -> Bytecode const&;

    Module& mo;
    Bytecode const& bc;
    Tuple* pos;
    Dict* kw;
};

#include "import.h"

void Callable::marker () const {
    mo.marker();
    mark(bc);
    mark(pos);
    mark(kw);
}

auto Callable::funcAt (int n) const -> Bytecode const& {
    return bc[n].asType<Bytecode>();
}

struct PyVM : Stacklet {
    static Type info;
    auto type () const -> Type const& override;

    struct Frame {
        //    <------- previous ------->  <---- actual ---->
        Value base, spOff, ipOff, callee, ep, locals, result, stack [];
        // result must be just below stack for proper module/class init
    };

    auto frame () const -> Frame& { return *(Frame*) (begin() + base); }

    auto spBase () const -> Value* { return frame().stack; }
    auto ipBase () const -> uint8_t const* { return callee->bc.start(); }

    auto fastSlot (uint32_t i) const -> Value& {
        return spBase()[callee->bc.fastSlotTop() + ~i];
    }
    auto derefSlot (uint32_t i) const -> Value& {
        return fastSlot(i).asType<Cell>().val;
    }

    static constexpr int EXC_STEP = 3; // use 3 entries per exception
    static constexpr auto FinallyTag = 1<<20;

    auto globals () const -> Module& { return callee->mo; }

    void marker () const override {
        Stacklet::marker();
        mark(callee);
        mark(caller);
        signal.marker();
    }

    // previous values are saved in current stack frame
    uint16_t base = 0;
    uint16_t spOff = 0;
    uint16_t ipOff = 0;
    Callable const* callee = nullptr;
    Stacklet* caller = nullptr;
    Value signal = {};

    Value* sp = nullptr;
    uint8_t const* ip = nullptr;

    auto fetchV (uint32_t v =0) -> uint32_t {
        uint8_t b = 0x80;
        while (b & 0x80) {
            b = *ip++;
            v = (v << 7) | (b & 0x7F);
        }
        return v;
    }

    auto fetchV64 () -> uint64_t {
        uint64_t v = *ip & 0x40 ? -1 : 0;
        uint8_t b = 0x80;
        while (b & 0x80) {
            b = *ip++;
            v = (v << 7) | (b & 0x7F);
        }
        return v;
    }

    auto fetchO () -> int {
        int n = *ip++;
        return n | (*ip++ << 8);
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
        spOff = sp - begin();
        ipOff = ip - ipBase();
        Value v = fun();
        sp = begin() + spOff;
        ip = ipBase() + ipOff;
        return v;
    }

    // most common use of contextAdjuster, wraps call and saves result (or nil)
    void wrappedCall (Value callee, ArgVec const& args) {
        auto v = contextAdjuster([=]() -> Value {
            return callee.obj().call(args);
        });
        *sp = v;
    }

    void instructionTrace () {
#if SHOW_INSTR_PTR
        static PyVM* prevCtx;
        if (prevCtx != this) {
            if (prevCtx != nullptr) {
                printf("\tip %04d sp %2d e ? ", prevCtx->ipOff,
                                                prevCtx->spOff - 9);
                printf("op 0x%02x : ", *(prevCtx->ipOff + prevCtx->ipBase()));
                if (prevCtx->spOff >= 9)
                    (prevCtx->spOff + prevCtx->spBase())->dump();
                printf("\n");
            }
            printf("### context changed from %p to %p ###\n", prevCtx, this);
            prevCtx = this;
        }
        printf("\tip %04d sp %2d e %d ", (int) (ip - ipBase()),
                                         (int) (sp - spBase()),
                                         (int) frame().ep);
        printf("op 0x%02x : ", *ip);
        if (sp >= spBase())
            sp->dump();
        printf("\n");
#endif
        assert(ip >= ipBase() && sp >= spBase() - 1);
    }

    // check and trigger gc on backwards jumps, i.e. inside all loops
    static void loopCheck (int arg) {
        if (arg < 0 && gcCheck())
            setPending(0);
    }

    //CG: op-init

    //CG1 op
    void opLoadNull () {
        *++sp = {};
    }
    //CG1 op
    void opLoadConstNone () {
        *++sp = Null;
    }
    //CG1 op
    void opLoadConstFalse () {
        *++sp = False;
    }
    //CG1 op
    void opLoadConstTrue () {
        *++sp = True;
    }
    //CG1 op q
    void opLoadConstString (Q arg) {
        *++sp = arg;
    }
    //CG1 op
    void opLoadConstSmallInt () {
        *++sp = Int::make(fetchV64());
    }
    //CG1 op v
    void opLoadConstObj (int arg) {
        *++sp = callee->bc[arg];
    }
    //CG1 op v
    void opLoadFastN (int arg) {
        *++sp = fastSlot(arg);
    }
    //CG1 op v
    void opStoreFastN (int arg) {
        fastSlot(arg) = *sp--;
    }
    //CG1 op v
    void opDeleteFast (int arg) {
        fastSlot(arg) = {};
    }

    //CG1 op
    void opDupTop () {
        ++sp; sp[0] = sp[-1];
    }
    //CG1 op
    void opDupTopTwo () {
        sp += 2; sp[0] = sp[-2]; sp[-1] = sp[-3];
    }
    //CG1 op
    void opPopTop () {
        --sp;
    }
    //CG1 op
    void opRotTwo () {
        auto v = sp[0]; sp[0] = sp[-1]; sp[-1] = v;
    }
    //CG1 op
    void opRotThree () {
        auto v = sp[0]; sp[0] = sp[-1]; sp[-1] = sp[-2]; sp[-2] = v;
    }

    //CG1 op s
    void opJump (int arg) {
        ip += arg;
    }
    //CG1 op s
    void opPopJumpIfFalse (int arg) {
        if (!sp->truthy())
            ip += arg;
        --sp;
    }
    //CG1 op s
    void opJumpIfFalseOrPop (int arg) {
        if (!sp->truthy())
            ip += arg;
        else
            --sp;
    }
    //CG1 op s
    void opPopJumpIfTrue (int arg) {
        if (sp->truthy())
            ip += arg;
        --sp;
    }
    //CG1 op s
    void opJumpIfTrueOrPop (int arg) {
        if (sp->truthy())
            ip += arg;
        else
            --sp;
    }

    //CG1 op q
    void opLoadName (Q arg) {
        assert(frame().locals.isObj());
        *++sp = frame().locals.obj().getAt(arg);
        if (sp->isNil())
            *sp = {E::NameError, arg};
    }
    //CG1 op q
    void opStoreName (Q arg) {
        auto& l = frame().locals;
        if (!l.isObj())
            l = new Dict (&globals());
        l.obj().setAt(arg, *sp--);
    }
    //CG1 op q
    void opDeleteName (Q arg) {
        assert(frame().locals.isObj());
        frame().locals.obj().setAt(arg, {});
    }
    //CG1 op q
    void opLoadGlobal (Q arg) {
        *++sp = globals().at(arg);
        assert(!sp->isNil());
    }
    //CG1 op q
    void opStoreGlobal (Q arg) {
        globals().at(arg) = *sp--;
    }
    //CG1 op q
    void opDeleteGlobal (Q arg) {
        globals().at(arg) = {};
    }
    //CG1 op q
    void opLoadAttr (Q arg) {
        Value self;
        Value v = sp->obj().attr(arg, self);
        if (v.isNil())
            *sp = {E::AttributeError, arg, sp->asObj().type().name};
        else {
            *sp = v;
            // TODO should this be moved into Inst::attr ???
            auto f = sp->ifType<Callable>();
            if (!self.isNil() && f != 0)
                *sp = new BoundMeth (*f, self);
        }
    }
    //CG1 op q
    void opStoreAttr (Q arg) {
        sp->obj().setAt(arg, sp[-1]);
        sp -= 2;
    }
    //CG1 op
    void opLoadSubscr () {
        --sp;
        *sp = sp->asObj().getAt(sp[1]);
        if (sp->isNil())
            *sp = {E::KeyError, sp[1]};
    }
    //CG1 op
    void opStoreSubscr () {
        --sp; // val [obj] key
        assert(sp->isObj());
        sp->obj().setAt(sp[1], sp[-1]);
        sp -= 2;
    }

    //CG1 op v
    void opBuildSlice (int arg) {
        sp -= arg - 1;
        *sp = Slice::create({*this, arg, (int) (sp - begin())});
    }
    //CG1 op v
    void opBuildTuple (int arg) {
        sp -= arg - 1;
        *sp = Tuple::create({*this, arg, (int) (sp - begin())});
    }
    //CG1 op v
    void opBuildList (int arg) {
        sp -= arg - 1;
        *sp = List::create({*this, arg, (int) (sp - begin())});
    }
    //CG1 op v
    void opBuildSet (int arg) {
        sp -= arg - 1;
        *sp = Set::create({*this, arg, (int) (sp - begin())});
    }
    //CG1 op v
    void opBuildMap (int arg) {
        *++sp = Dict::create({*this, arg});
    }
    //CG1 op
    void opStoreMap () {
        sp -= 2;
        sp->obj().setAt(sp[2], sp[1]); // TODO optimise later: no key check
    }
    //CG1 op v
    void opStoreComp (int arg) {
        (void) arg; assert(false); // TODO
    }
    //CG1 op v
    void opUnpackSequence (int arg) {
        auto& seq = sp->asObj(); // TODO iterators
        if ((int) seq.len() != arg)
            *sp = {E::ValueError, "unpack count mismatch", (int) seq.len()};
        else {
            for (int i = 0; i < arg; ++i)
                sp[arg-i-1] = seq.getAt(i);
            sp += arg - 1;
        }
    }
    //CG1 op v
    void opUnpackEx (int arg) {
        auto& seq = sp->asType<List>(); // TODO tuples and iterators
        uint8_t left = arg, right = arg >> 8;
        int got = seq.len();
        if (got < left + right)
            *sp = {E::ValueError, "unpack needs more items", got};
        else {
            for (int i = 0; i < right; ++i)
                sp[i] = seq.getAt(got-i-1);
            sp[right] = List::create({seq, got - left - right, left});
            for (int i = 0; i < left; ++i)
                sp[right+1+i] = seq.getAt(left-i-1);
            sp += left + right;
        }
    }

    //CG1 op o
    void opSetupExcept (int arg) {
        auto exc = excBase(1);
        exc[0] = ip - ipBase() + arg;
        exc[1] = sp - begin();
        exc[2] = {};
    }
    //CG1 op o
    void opSetupFinally (int arg) {
        opSetupExcept(arg + FinallyTag);
    }
    //CG1 op
    void opEndFinally () {
        excBase(-1);
        if (sp->isNone())
            --sp;
        else if (!sp->isInt())
            opRaiseObj();
        else if (*sp < 0) {
            --sp;
            opReturnValue();
        } else
            assert(false); // TODO unwind jump
    }
    //CG1 op o
    void opSetupWith (int arg) {
        auto exit = Q( 13,"__exit__");
        sp[1] = {};
        *sp = sp->asObj().attr(exit, sp[1]);
        if (sp->isNil()) {
            *sp = {E::AttributeError, exit};
            return;
        }

        auto entry = Q( 12,"__enter__");
        sp[2] = sp[1].asObj().attr(entry, sp[3]);
        if (sp->isNil()) {
            sp[2] = {E::AttributeError, entry};
            return;
        }

        ++sp;
        opSetupExcept(arg);
        ++sp;

        wrappedCall(*sp, {*this, 1, sp});
    }
    //CG1 op
    void opWithCleanup () {
        assert(sp->isNone()); // TODO other cases
        sp[1] = Null;
        sp[2] = Null;
        sp -= 2;

        wrappedCall(*sp, {*this, 4, sp + 1});
    }
    //CG1 op o
    void opPopExceptJump (int arg) {
        excBase(-1);
        ip += arg;
    }
    //CG1 op
    void opRaiseLast () {
        // TODO re-raise previous exception, if there is one, else:
        Value ({E::RuntimeError, "no active exception"});
    }
    //CG1 op
    void opRaiseObj () {
        raise(*sp);
    }
    //CG1 op
    void opRaiseFrom () {
        raise(*--sp); // exception chaining is not supported
    }
    //CG1 op s
    void opUnwindJump (int arg) {
        int ep = frame().ep;
        frame().ep = ep - *ip; // TODO hardwired for simplest case
        ip += arg;
    }

    //CG1 op
    void opLoadBuildClass () {
        *++sp = Class::info;
    }
    //CG1 op q
    void opLoadMethod (Q arg) {
        sp[1] = {};
        auto v = sp->asObj().attr(arg, sp[1]);
        if (v.isNil()) // TODO duplicate code, move test & exception into attr?
            *sp = {E::AttributeError, arg, sp->asObj().type().name};
        else
            *sp = v;
        ++sp;
    }
    //CG1 op q
    void opLoadSuperMethod (Q arg) {
        --sp;
        sp[-1] = sp->obj().getAt(arg);
        *sp = sp[1];
    }
    //CG1 op v
    void opCallMethod (int arg) {
        uint8_t npos = arg, nkw = arg >> 8;
        sp -= npos + 2 * nkw + 1;
        auto skip = sp[1].isNil();
        wrappedCall(*sp, {*this, arg + 1 - skip, sp + 1 + skip});
    }
    //CG1 op v
    void opCallMethodVarKw (int arg) {
        (void) arg; assert(false); // TODO
    }
    //CG1 op v
    void opMakeFunction (int arg) {
        auto& bc = callee->funcAt(arg);
        *++sp = new Callable (bc);
    }
    //CG1 op v
    void opMakeFunctionDefargs (int arg) {
        --sp;
        auto& bc = callee->funcAt(arg);
        *sp = new Callable (bc, sp[0], sp[1]);
    }
    //CG1 op v
    void opCallFunction (int arg) {
        uint8_t npos = arg, nkw = arg >> 8;
        sp -= npos + 2 * nkw;
        auto& fun = sp->obj();
        wrappedCall(*sp, {*this, arg, sp + 1});
        // TODO yuck, special cased because Class doesn't have access to PyVM
        if (&fun == &Class::info)
            frame().locals = *sp;
    }
    //CG1 op v
    void opCallFunctionVarKw (int arg) {
        (void) arg; assert(false); // TODO
    }

    //CG1 op v
    void opMakeClosure (int arg) {
        int num = *ip++;
        sp -= num - 1;
        auto& bc = callee->funcAt(arg);
        auto f = new Callable (bc);
        *sp = new Closure (*f, {*this, num, sp});
    }
    //CG1 op v
    void opMakeClosureDefargs (int arg) {
        int num = *ip++;
        sp -= 2 + num - 1;
        auto& bc = callee->funcAt(arg);
        auto f = new Callable (bc, sp[0], sp[1]);
        *sp = new Closure (*f, {*this, num, sp + 2});
    }
    //CG1 op v
    void opLoadDeref (int arg) {
        *++sp = derefSlot(arg);
        assert(!sp->isNil());
    }
    //CG1 op v
    void opStoreDeref (int arg) {
        derefSlot(arg) = *sp--;
    }
    //CG1 op v
    void opDeleteDeref (int arg) {
        derefSlot(arg) = {};
    }

    //CG1 op
    void opYieldValue () {
        assert(caller != nullptr);
        assert(caller != this);
        auto& myCaller = Value(caller).asType<PyVM>(); // TODO non-PyVM caller ?
        caller = nullptr;
        // TODO messy: result needs to be stored in another PyVM instance
        myCaller[myCaller.spOff] = *sp;
        switchTo(&myCaller);
    }
    //CG1 op
    void opYieldFrom () {
        --sp;
        auto& child = sp->asType<PyVM>();
        assert(child.caller == nullptr);
        child.caller = this;
        switchTo(nullptr);
    }
    //CG1 op
    void opReturnValue () {
        auto v = contextAdjuster([=]() -> Value {
            return leave(*sp);
        });
        *sp = v;
    }

    //CG1 op
    void opGetIter () {
        auto v = sp->asObj().iter();
        if (v.isInt())
            v = new Iterator (sp->asObj(), v);
        *sp = v;
    }
    //CG1 op
    void opGetIterStack () {
        // TODO the compiler assumes 4 stack entries are used!
        //  layout [seq,(idx|iter),nil,nil]
        *sp = sp->asObj(); // for qstrs, etc
        auto v = sp->obj().iter(); // will be 0 for indexed iteration
        *++sp = v;
        *++sp = {};
        *++sp = {};
    }
    //CG1 op o
    void opForIter (int arg) {
        Value v;
        auto& pos = sp[-2];
        if (pos.isInt()) {
            assert(sp[-3].isObj());
            auto& seq = sp[-3].obj();
            int n = pos;
            if (n < (int) seq.len()) {
                if (&seq.type() == &Dict::info || &seq.type() == &Set::info)
                    v = ((List&) seq)[n]; // avoid keyed access
                else
                    v = seq.getAt(n);
                pos = n + 1;
            }
        } else
            v = pos.obj().next();
        if (v.isNil()) {
            sp -= 4;
            ip += arg;
        } else
            *++sp = v;
    }

    //CG1 op q
    void opImportName (Q arg) {
        --sp; // TODO ignore fromlist for now, *sp level also ignored
        Value mod = Module::loaded.at(arg);
        if (mod.isNil()) { // TODO this code should be placed elsewhere
            auto data = vmImport(arg);
            assert(data != nullptr);
            Loader loader;
            auto init = loader.load(data);
            assert(init != nullptr);
            mod = init->mo;
            init->mo.at(Q( 23,"__name__")) = arg;
            Module::loaded.at(arg) = mod;
            wrappedCall(init, {*this, 0});
            frame().locals = mod;
        }
        *sp = mod;
    }
    //CG1 op q
    void opImportFrom (Q arg) {
        Value v = sp->obj().getAt(arg);
        *++sp = v;
    }
    //CG1 op
    void opImportStar () {
        auto& dest = globals();
        auto& from = sp->asType<Module>();
        for (uint8_t i = 0; i < from.len(); ++i) {
            auto k = from[i];
            assert(k.isStr());
            if (*((char const*) k) != '_')
                dest.setAt(k, from.at(k));
        }
        --sp;
    }

    //CG1 op m 64
    void opLoadConstSmallIntMulti (uint32_t arg) {
        *++sp = arg - 16;
    }
    //CG1 op m 16
    void opLoadFastMulti (uint32_t arg) {
        *++sp = fastSlot(arg);
        assert(!sp->isNil());
    }
    //CG1 op m 16
    void opStoreFastMulti (uint32_t arg) {
        fastSlot(arg) = *sp--;
    }
    //CG1 op m 7
    void opUnaryOpMulti (uint32_t arg) {
        *sp = sp->unOp((UnOp) arg);
    }
    //CG1 op m 35
    void opBinaryOpMulti (uint32_t arg) {
        --sp;
        *sp = sp->binOp((BinOp) arg, sp[1]);
    }

    void inner () {
        sp = begin() + spOff;
        ip = ipBase() + ipOff;

        do {
            INNER_HOOK  // used for simulated time in native builds
            instructionTrace();
            switch ((Op) *ip++) {

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
                    if ((uint32_t) (ip[-1] - Op::LoadConstSmallIntMulti) < 64) {
                        uint32_t arg = ip[-1] - Op::LoadConstSmallIntMulti;
                        opLoadConstSmallIntMulti(arg);
                        break;
                    }
                    if ((uint32_t) (ip[-1] - Op::LoadFastMulti) < 16) {
                        uint32_t arg = ip[-1] - Op::LoadFastMulti;
                        opLoadFastMulti(arg);
                        break;
                    }
                    if ((uint32_t) (ip[-1] - Op::StoreFastMulti) < 16) {
                        uint32_t arg = ip[-1] - Op::StoreFastMulti;
                        opStoreFastMulti(arg);
                        break;
                    }
                    if ((uint32_t) (ip[-1] - Op::UnaryOpMulti) < 7) {
                        uint32_t arg = ip[-1] - Op::UnaryOpMulti;
                        opUnaryOpMulti(arg);
                        break;
                    }
                    if ((uint32_t) (ip[-1] - Op::BinaryOpMulti) < 35) {
                        uint32_t arg = ip[-1] - Op::BinaryOpMulti;
                        opBinaryOpMulti(arg);
                        break;
                    }
                    //CG>
                    assert(false);
                }
            }

        } while (pending == 0);

        spOff = sp - begin();
        ipOff = ip - ipBase();

        if (pending & (1<<0))
            caught();

        if (current != this)
            return; // last frame popped, there's no context left
    }

    void enter (Callable const& func) {
        auto frameSize = func.bc.fastSlotTop() + EXC_STEP * func.bc.excLevel();
        int need = (frame().stack + frameSize) - (begin() + base);

        auto curr = base;           // current frame offset
        base = fill;                // new frame offset
        insert(fill, need);         // make room

        auto& f = frame();          // new frame
        f.base = curr;              // index of (now previous) frame
        f.spOff = spOff;            // previous stack index
        f.ipOff = ipOff;            // previous instruction index
        f.callee = callee;          // previous callable

        spOff = f.stack-begin()-1;  // stack starts out empty
        ipOff = 0;                  // code starts at first opcode
        callee = &func;             // new callable context
        f.ep = 0;                   // no exceptions pending
    }

    Value leave (Value v ={}) {
        auto& f = frame();
        auto r = f.result;          // stored result
        if (r.isNil())              // use return result if set
            r = v;                  // ... else arg

        if (base > 0) {
            int prev = f.base;      // previous frame offset
            spOff = f.spOff;        // restore stack index
            ipOff = f.ipOff;        // restore instruction index
            callee = &f.callee.asType<Callable>(); // restore callee

            assert(fill > base);
            remove(base, fill - base); // delete current frame

            assert(prev >= 0);
            base = prev;            // new lower frame offset
        } else {
            // last frame, drop context, restore caller
            fill = 0; // delete stack entries
            adj(1); // release vector FIXME crashes when set to 0 (???)
            switchTo(caller);
        }

        return r;
    }

    auto excBase (int incr) -> Value* {
        uint32_t ep = frame().ep;
        frame().ep = ep + incr;
        if (incr <= 0)
            --ep;
        return frame().stack + callee->bc.fastSlotTop() + EXC_STEP * ep;
    }

    void caught () {
        auto e = signal;
        if (e.isNil())
            return; // there was no exception, just an inner loop exit
        signal = {};

        auto& einfo = e.asType<Exception>().extra();
        einfo.ipOff = ipOff;
        einfo.callee = callee;

        if (frame().ep > 0) { // simple exception, no stack unwind
            auto ep = excBase(0);
            ipOff = ep[0] & (FinallyTag - 1);
            spOff = ep[1];
            begin()[++spOff] = e.isNil() ? ep[2] : e;
        } else {
            leave();
            if (current != nullptr)
                current->raise(e);
            else {
                Buffer buf;
                buf.print("uncaught exception: ");
                e.obj().repr(buf);
                buf.putc('\n');
            }
        }
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

    auto next () -> Value override {
        assert(fill > 0); // can only resume if not ended
        assert(current != nullptr);
        assert(current != this);
        assert(caller == nullptr);
        caller = &Value(current).asType<PyVM>();
        current = this;
        setPending(0);
        return {}; // no result yet
    }

    void raise (Value exc) override {
        uint32_t num = 0;
        if (exc.isInt())
            num = exc;      // trigger soft-irq 1..31 (interrupt-safe)
        else
            signal = exc;  // trigger exception or other outer-loop req
        setPending(num);    // force inner loop exit
    }
};

static auto currentVM () -> PyVM& {
    Value v = Stacklet::current;
    return v.asType<PyVM>(); // TODO yuck
}

Callable::Callable (Bytecode const& callee, Module* mod, Tuple* t, Dict* d)
        : mo (mod != nullptr ? *mod : currentVM().globals()),
          bc (callee), pos (t), kw (d) {
}

auto Callable::call (ArgVec const& args) const -> Value {
    auto ctx = &currentVM();
    auto coro = bc.isGenerator();
    if (coro)
        ctx = new PyVM;
    ctx->enter(*this);
    if (coro)
        ctx->frame().locals = &mo;

    int nPos = bc.numArgs(0);
    int nDef = bc.numArgs(1);
    int nKwo = bc.numArgs(2);
    int nc = bc.numCells();

    for (int i = 0; i < nPos + nc; ++i)
        if (i < args.num)
            ctx->fastSlot(i) = args[i];
        else if (pos != nullptr && (uint32_t) i < nDef + pos->fill)
            ctx->fastSlot(i) = (*pos)[i+nDef-nPos];

    if (bc.hasVarArgs())
        ctx->fastSlot(nPos+nKwo) =
            Tuple::create({args.vec, args.num-nPos, args.off+nPos});

    uint8_t const* cellMap = bc.start() - nc;
    for (int i = 0; i < nc; ++i) {
        auto slot = cellMap[i];
        ctx->fastSlot(slot) = new Cell (ctx->fastSlot(slot));
    }

    return coro ? ctx : Value {};
}

Type Bytecode::info (Q(196,"<bytecode>"));
auto Bytecode::type () const -> Type const& { return info; }

Type Callable::info (Q(197,"<callable>"));
auto Callable::type () const -> Type const& { return info; }

Type PyVM::info (Q(198,"<pyvm>"));
auto PyVM::type () const -> Type const& { return info; }

auto monty::vmLaunch (uint8_t const* data) -> Stacklet* {
    Loader loader;
    auto init = loader.load(data);
    if (init == nullptr) // try doing an MRFS lookup if it's a name
        init = loader.load(vmImport((char const*) data));
    if (init == nullptr)
        return nullptr;
    auto vm = new PyVM;
    vm->enter(*init);
    vm->frame().locals = &init->mo;
    return vm;
}
