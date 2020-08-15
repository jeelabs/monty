// pyvm.h - virtual machine for bytecodes emitted by MicroPython 1.12

#define SHOW_INSTR_PTR 0 // show instr ptr each time through loop (in pyvm.h)
//CG: off op:print # set to "on" to enable per-opcode debug output

struct PyVM : Runner {
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

    Value* sp;
    uint8_t const* ip;
    Context* ctx;

    uint32_t fetchVarInt (uint32_t v =0) {
        uint8_t b = 0x80;
        while (b & 0x80) {
            b = *ip++;
            v = (v << 7) | (b & 0x7F);
        }
        return v;
    }

    int fetchOffset () {
        int n = *ip++;
        return n | (*ip++ << 8);
    }

    const char* fetchQstr () { return ctx->getQstr(fetchOffset() + 1); }

    auto spAsPtr () const -> Value* { return ctx->begin() + ctx->spIdx; }
    auto spAsOff () const -> uint32_t { return sp - ctx->begin(); }

    // special wrapper to deal with context changes vs cached ctx/sp values
    template< typename T >
    auto contextAdjuster (T fun) -> Value {
        ctx->spIdx = spAsOff();
        ctx->ipIdx = ip - ctx->ipBase();
        Value v = fun();
        if (active == nullptr) {
            ctx->raise(); // there's nothing to do, exit inner loop
            ctx = nullptr;
        } else {
            ctx = active;
            sp = spAsPtr();
            ip = ctx->ipBase() + ctx->ipIdx;
        }
        return v;
    }

    void instructionTrace () {
#if SHOW_INSTR_PTR
        printf("\tip %04d sp %2d e %d ", (int) (ip - ctx->ipBase()),
                                         (int) (sp - ctx->spBase()),
                                         (int) ctx->epIdx);
        printf("op 0x%02x : ", *ip);
        if (sp >= ctx->spBase())
            sp->dump();
        printf("\n");
#endif
        assert(ip >= ctx->ipBase() && sp >= ctx->spBase() - 1);
    }

    //CG: op-init

    //CG1 op
    void op_LoadNull () {
        *++sp = {};
    }
    //CG1 op
    void op_LoadConstNone () {
        *++sp = Null;
    }
    //CG1 op
    void op_LoadConstFalse () {
        *++sp = False;
    }
    //CG1 op
    void op_LoadConstTrue () {
        *++sp = True;
    }
    //CG1 op q
    void op_LoadConstString (char const* arg) {
        *++sp = arg;
    }
    //CG1 op
    void op_LoadConstSmallInt () {
        *++sp = fetchVarInt((*ip & 0x40) ? ~0 : 0);
    }
    //CG1 op v
    void op_LoadConstObj (int arg) {
        *++sp = ctx->getConst(arg);
    }
    //CG1 op v
    void op_LoadFastN (int arg) {
        *++sp = ctx->fastSlot(arg);
    }
    //CG1 op v
    void op_StoreFastN (int arg) {
        ctx->fastSlot(arg) = *sp--;
    }
    //CG1 op v
    void op_DeleteFast (int arg) {
        ctx->fastSlot(arg) = {};
    }

    //CG1 op
    void op_DupTop () {
        ++sp; sp[0] = sp[-1];
    }
    //CG1 op
    void op_DupTopTwo () {
        sp += 2; sp[0] = sp[-2]; sp[-1] = sp[-3];
    }
    //CG1 op
    void op_PopTop () {
        --sp;
    }
    //CG1 op
    void op_RotTwo () {
        auto v = sp[0]; sp[0] = sp[-1]; sp[-1] = v;
    }
    //CG1 op
    void op_RotThree () {
        auto v = sp[0]; sp[0] = sp[-1]; sp[-1] = sp[-2]; sp[-2] = v;
    }

    //CG1 op s
    void op_Jump (int arg) {
        ip += arg;
    }
    //CG1 op s
    void op_PopJumpIfFalse (int arg) {
        if (!sp->truthy())
            ip += arg;
        --sp;
    }
    //CG1 op s
    void op_JumpIfFalseOrPop (int arg) {
        if (!sp->truthy())
            ip += arg;
        else
            --sp;
    }
    //CG1 op s
    void op_PopJumpIfTrue (int arg) {
        if (sp->truthy())
            ip += arg;
        --sp;
    }
    //CG1 op s
    void op_JumpIfTrueOrPop (int arg) {
        if (sp->truthy())
            ip += arg;
        else
            --sp;
    }

    //CG1 op q
    void op_LoadName (char const* arg) {
        assert(ctx->locals != nullptr);
        *++sp = ctx->locals->at(arg);
        assert(!sp->isNil());
    }
    //CG1 op q
    void op_StoreName (char const* arg) {
        if (ctx->locals == nullptr)
            ctx->locals = new Dict (&ctx->globals());
        ctx->locals->at(arg) = *sp--;
    }
    //CG1 op q
    void op_DeleteName (char const* arg) {
        assert(ctx->locals != nullptr);
        ctx->locals->at(arg) = {};
    }
    //CG1 op q
    void op_LoadGlobal (char const* arg) {
        *++sp = ctx->globals().at(arg);
        assert(!sp->isNil());
    }
    //CG1 op q
    void op_StoreGlobal (char const* arg) {
        ctx->globals().at(arg) = *sp--;
    }
    //CG1 op q
    void op_DeleteGlobal (char const* arg) {
        ctx->globals().at(arg) = {};
    }
    //CG1 op q
    void op_LoadAttr (char const* arg) {
        Value self;
        *sp = sp->obj().attr(arg, self);
        assert(!sp->isNil());
        if (!self.isNil() && sp->ifType<Callable>() != 0)
            *sp = new BoundMeth (*sp, self);
    }
    //CG1 op q
    void op_StoreAttr (char const* arg) {
        assert(&sp->obj().type().type() == &Class::info);
        sp->obj().setAt(arg, sp[-1]);
        sp -= 2;
    }
    //CG1 op
    void op_LoadSubscr () {
        --sp;
        *sp = sp->asObj().getAt(sp[1]);
    }
    //CG1 op
    void op_StoreSubscr () {
        --sp; // val [obj] key
        assert(sp->isObj());
        sp->obj().setAt(sp[1], sp[-1]);
        sp -= 2;
    }

    //CG1 op v
    void op_BuildSlice (int arg) {
        sp -= arg - 1;
        *sp = Slice::create(*ctx, arg, spAsOff());
    }
    //CG1 op v
    void op_BuildTuple (int arg) {
        sp -= arg - 1;
        *sp = Tuple::create(*ctx, arg, spAsOff());
    }
    //CG1 op v
    void op_BuildList (int arg) {
        sp -= arg - 1;
        *sp = List::create(*ctx, arg, spAsOff());
    }
    //CG1 op v
    void op_BuildSet (int arg) {
        sp -= arg - 1;
        *sp = Set::create(*ctx, arg, spAsOff());
    }
    //CG1 op v
    void op_BuildMap (int arg) {
        *++sp = Dict::create(*ctx, arg, 0);
    }
    //CG1 op
    void op_StoreMap () {
        sp -= 2;
        sp->obj().setAt(sp[2], sp[1]); // TODO optimise later: no key check
    }

    //CG1 op o
    void op_SetupExcept (int arg) {
        auto exc = ctx->excBase(1);
        exc[0] = ip - ctx->ipBase() + arg;
        exc[1] = spAsOff();
        exc[2] = {};
    }
    //CG1 op o
    void op_PopExceptJump (int arg) {
        ctx->excBase(-1);
        ip += arg;
    }
    //CG1 op
    void op_RaiseLast () {
        ctx->raise(""); // TODO
    }
    //CG1 op
    void op_RaiseObj () {
        ctx->raise(*sp);
    }
    //CG1 op
    void op_RaiseFrom () {
        // TODO exception chaining
        ctx->raise(*--sp);
    }
    //CG1 op s
    void op_UnwindJump (int arg) {
        ctx->epIdx -= *ip; // TODO hardwired for simplest case
        ip += arg;
    }

    //CG1 op
    void op_LoadBuildClass () {
        *++sp = Class::info;
    }
    //CG1 op q
    void op_LoadMethod (char const* arg) {
        sp[1] = *sp;
        *sp = sp->asObj().attr(arg, sp[1]);
        ++sp;
        assert(!sp->isNil());
    }
    //CG1 op v
    void op_CallMethod (int arg) {
        uint8_t nargs = arg, nkw = arg >> 8;
        sp -= nargs + 2 * nkw + 1;
        auto v = contextAdjuster([=]() -> Value {
            return sp->obj().call(*ctx, arg + 1, ctx->spIdx + 1);
        });
        if (!v.isNil())
            *sp = v;
    }
    //CG1 op v
    void op_CallMethodVarKw (int arg) {
        assert(false); // TODO
    }
    //CG1 op v
    void op_MakeFunction (int arg) {
        auto v = ctx->getConst(arg);
        *++sp = new Callable (ctx->globals(), v.asType<Bytecode>());
    }
    //CG1 op v
    void op_MakeFunctionDefargs (int arg) {
        auto v = ctx->getConst(arg);
        --sp;
        *sp = new Callable (ctx->globals(), v.asType<Bytecode>(),
                                sp[0].ifType<Tuple>(), sp[1].ifType<Dict>());
    }
    //CG1 op v
    void op_CallFunction (int arg) {
        uint8_t nargs = arg, nkw = arg >> 8;
        sp -= nargs + 2 * nkw;
        auto v = contextAdjuster([=]() -> Value {
            return sp->obj().call(*ctx, arg, ctx->spIdx + 1);
        });
        if (!v.isNil() && sp >= ctx->spBase())
            *sp = v;
    }
    //CG1 op v
    void op_CallFunctionVarKw (int arg) {
        assert(false); // TODO
    }
    //CG1 op
    void op_YieldValue () {
        auto v = contextAdjuster([=]() -> Value {
            active = ctx->caller;
            return *sp;
        });
        if (ctx != nullptr)
            *sp = v;
    }
    //CG1 op
    void op_ReturnValue () {
        auto v = contextAdjuster([=]() -> Value {
            return ctx->leave(*sp);
        });
        if (ctx != nullptr)
            *sp = v;
    }

    //CG1 op
    void op_GetIter () {
        assert(false); // TODO
    }
    //CG1 op
    void op_GetIterStack () {
        // TODO yuck, the compiler assumes 4 stack entries are used!
        //  layout [seq,idx,nil,nil]
        *++sp = 0;
        *++sp = {};
        *++sp = {};
    }
    //CG1 op o
    void op_ForIter (int arg) {
        assert(sp->isNil());
        int n = sp[-2];
        Value v = sp[-3].obj().getAt(n++);
        if (v.isNil()) {
            sp -= 4;
            ip += arg;
        } else {
            sp[-2] = n;
            *++sp = v;
        }
    }

    //CG1 op m 64
    void op_LoadConstSmallIntMulti (uint32_t arg) {
        *++sp = arg - 16;
    }
    //CG1 op m 16
    void op_LoadFastMulti (uint32_t arg) {
        *++sp = ctx->fastSlot(arg);
        assert(!sp->isNil());
    }
    //CG1 op m 16
    void op_StoreFastMulti (uint32_t arg) {
        ctx->fastSlot(arg) = *sp--;
    }
    //CG1 op m 7
    void op_UnaryOpMulti (uint32_t arg) {
        *sp = sp->unOp((UnOp) arg);
    }
    //CG1 op m 35
    void op_BinaryOpMulti (uint32_t arg) {
        --sp;
        *sp = sp->binOp((BinOp) arg, sp[1]);
    }

    PyVM () {
        ctx = active;
        assert(ctx != nullptr);
        sp = spAsPtr();
        ip = ctx->ipBase() + ctx->ipIdx;

        do {
            instructionTrace();
            switch ((Op) *ip++) {

                //CG< op-emit d
                case Op::LoadNull: 
                    op_LoadNull();
                    break;
                case Op::LoadConstNone: 
                    op_LoadConstNone();
                    break;
                case Op::LoadConstFalse: 
                    op_LoadConstFalse();
                    break;
                case Op::LoadConstTrue: 
                    op_LoadConstTrue();
                    break;
                case Op::LoadConstString: {
                    char const* arg = fetchQstr();
                    op_LoadConstString(arg);
                    break;
                }
                case Op::LoadConstSmallInt: 
                    op_LoadConstSmallInt();
                    break;
                case Op::LoadConstObj: {
                    int arg = fetchVarInt();
                    op_LoadConstObj(arg);
                    break;
                }
                case Op::LoadFastN: {
                    int arg = fetchVarInt();
                    op_LoadFastN(arg);
                    break;
                }
                case Op::StoreFastN: {
                    int arg = fetchVarInt();
                    op_StoreFastN(arg);
                    break;
                }
                case Op::DeleteFast: {
                    int arg = fetchVarInt();
                    op_DeleteFast(arg);
                    break;
                }
                case Op::DupTop: 
                    op_DupTop();
                    break;
                case Op::DupTopTwo: 
                    op_DupTopTwo();
                    break;
                case Op::PopTop: 
                    op_PopTop();
                    break;
                case Op::RotTwo: 
                    op_RotTwo();
                    break;
                case Op::RotThree: 
                    op_RotThree();
                    break;
                case Op::Jump: {
                    int arg = fetchOffset()-0x8000;
                    op_Jump(arg);
                    break;
                }
                case Op::PopJumpIfFalse: {
                    int arg = fetchOffset()-0x8000;
                    op_PopJumpIfFalse(arg);
                    break;
                }
                case Op::JumpIfFalseOrPop: {
                    int arg = fetchOffset()-0x8000;
                    op_JumpIfFalseOrPop(arg);
                    break;
                }
                case Op::PopJumpIfTrue: {
                    int arg = fetchOffset()-0x8000;
                    op_PopJumpIfTrue(arg);
                    break;
                }
                case Op::JumpIfTrueOrPop: {
                    int arg = fetchOffset()-0x8000;
                    op_JumpIfTrueOrPop(arg);
                    break;
                }
                case Op::LoadName: {
                    char const* arg = fetchQstr();
                    op_LoadName(arg);
                    break;
                }
                case Op::StoreName: {
                    char const* arg = fetchQstr();
                    op_StoreName(arg);
                    break;
                }
                case Op::DeleteName: {
                    char const* arg = fetchQstr();
                    op_DeleteName(arg);
                    break;
                }
                case Op::LoadGlobal: {
                    char const* arg = fetchQstr();
                    op_LoadGlobal(arg);
                    break;
                }
                case Op::StoreGlobal: {
                    char const* arg = fetchQstr();
                    op_StoreGlobal(arg);
                    break;
                }
                case Op::DeleteGlobal: {
                    char const* arg = fetchQstr();
                    op_DeleteGlobal(arg);
                    break;
                }
                case Op::LoadAttr: {
                    char const* arg = fetchQstr();
                    op_LoadAttr(arg);
                    break;
                }
                case Op::StoreAttr: {
                    char const* arg = fetchQstr();
                    op_StoreAttr(arg);
                    break;
                }
                case Op::LoadSubscr: 
                    op_LoadSubscr();
                    break;
                case Op::StoreSubscr: 
                    op_StoreSubscr();
                    break;
                case Op::BuildSlice: {
                    int arg = fetchVarInt();
                    op_BuildSlice(arg);
                    break;
                }
                case Op::BuildTuple: {
                    int arg = fetchVarInt();
                    op_BuildTuple(arg);
                    break;
                }
                case Op::BuildList: {
                    int arg = fetchVarInt();
                    op_BuildList(arg);
                    break;
                }
                case Op::BuildSet: {
                    int arg = fetchVarInt();
                    op_BuildSet(arg);
                    break;
                }
                case Op::BuildMap: {
                    int arg = fetchVarInt();
                    op_BuildMap(arg);
                    break;
                }
                case Op::StoreMap: 
                    op_StoreMap();
                    break;
                case Op::SetupExcept: {
                    int arg = fetchOffset();
                    op_SetupExcept(arg);
                    break;
                }
                case Op::PopExceptJump: {
                    int arg = fetchOffset();
                    op_PopExceptJump(arg);
                    break;
                }
                case Op::RaiseLast: 
                    op_RaiseLast();
                    break;
                case Op::RaiseObj: 
                    op_RaiseObj();
                    break;
                case Op::RaiseFrom: 
                    op_RaiseFrom();
                    break;
                case Op::UnwindJump: {
                    int arg = fetchOffset()-0x8000;
                    op_UnwindJump(arg);
                    break;
                }
                case Op::LoadBuildClass: 
                    op_LoadBuildClass();
                    break;
                case Op::LoadMethod: {
                    char const* arg = fetchQstr();
                    op_LoadMethod(arg);
                    break;
                }
                case Op::CallMethod: {
                    int arg = fetchVarInt();
                    op_CallMethod(arg);
                    break;
                }
                case Op::CallMethodVarKw: {
                    int arg = fetchVarInt();
                    op_CallMethodVarKw(arg);
                    break;
                }
                case Op::MakeFunction: {
                    int arg = fetchVarInt();
                    op_MakeFunction(arg);
                    break;
                }
                case Op::MakeFunctionDefargs: {
                    int arg = fetchVarInt();
                    op_MakeFunctionDefargs(arg);
                    break;
                }
                case Op::CallFunction: {
                    int arg = fetchVarInt();
                    op_CallFunction(arg);
                    break;
                }
                case Op::CallFunctionVarKw: {
                    int arg = fetchVarInt();
                    op_CallFunctionVarKw(arg);
                    break;
                }
                case Op::YieldValue: 
                    op_YieldValue();
                    break;
                case Op::ReturnValue: 
                    op_ReturnValue();
                    break;
                case Op::GetIter: 
                    op_GetIter();
                    break;
                case Op::GetIterStack: 
                    op_GetIterStack();
                    break;
                case Op::ForIter: {
                    int arg = fetchOffset();
                    op_ForIter(arg);
                    break;
                }
                //CG>

                // TODO
                case Op::DeleteDeref:
                case Op::EndFinally:
                case Op::ImportFrom:
                case Op::ImportName:
                case Op::ImportStar:
                case Op::LoadDeref:
                case Op::LoadSuperMethod:
                case Op::MakeClosure:
                case Op::MakeClosureDefargs:
                case Op::SetupFinally:
                case Op::SetupWith:
                case Op::StoreComp:
                case Op::StoreDeref:
                case Op::UnpackEx:
                case Op::UnpackSequence:
                case Op::WithCleanup:
                case Op::YieldFrom:

                default: {
                    //CG< op-emit m
                    if ((uint32_t) (ip[-1] - Op::LoadConstSmallIntMulti) < 64) {
                        uint32_t arg = ip[-1] - Op::LoadConstSmallIntMulti;
                        op_LoadConstSmallIntMulti(arg);
                        break;
                    }
                    if ((uint32_t) (ip[-1] - Op::LoadFastMulti) < 16) {
                        uint32_t arg = ip[-1] - Op::LoadFastMulti;
                        op_LoadFastMulti(arg);
                        break;
                    }
                    if ((uint32_t) (ip[-1] - Op::StoreFastMulti) < 16) {
                        uint32_t arg = ip[-1] - Op::StoreFastMulti;
                        op_StoreFastMulti(arg);
                        break;
                    }
                    if ((uint32_t) (ip[-1] - Op::UnaryOpMulti) < 7) {
                        uint32_t arg = ip[-1] - Op::UnaryOpMulti;
                        op_UnaryOpMulti(arg);
                        break;
                    }
                    if ((uint32_t) (ip[-1] - Op::BinaryOpMulti) < 35) {
                        uint32_t arg = ip[-1] - Op::BinaryOpMulti;
                        op_BinaryOpMulti(arg);
                        break;
                    }
                    //CG>
                    assert(false);
                }
            }
        } while (pending == 0);

        if (ctx == nullptr)
            return; // task returned, there's no context left

        ctx->spIdx = spAsOff();
        ctx->ipIdx = ip - ctx->ipBase();

        if (Context::wasPending(0)) {
            auto e = ctx->event;
            ctx->event = {};
            if (!e.isNil())
                ctx->caught(e);
        }
    }
};
