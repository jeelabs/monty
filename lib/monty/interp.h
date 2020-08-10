// Main interpreter loop. Only run() is public, it cannot be called recursively.

//CG: off op:print # set to "on" to enable per-opcode debug output

struct QstrPool : Object {
    int len;
    uint16_t off [];

    static const QstrPool* create (const char* d, int n, int b) {
        static_assert (sizeof *off == 2, "off is not a uint16_t ?");
        return new (2*(n+1)+b) QstrPool (d, n, b);
    }

    QstrPool (const char* data, int num, int bytes) : len (num) {
        auto vec = (char*) off;
        off[0] = 2*(num+1);
        memcpy(vec + off[0], data, bytes);
        for (int i = 0; i < num; ++i) {
            auto pos = off[i];
            off[i+1] = pos + strlen(vec + pos) + 1;
        }
    }

    const char* atIdx (int idx) const {
        assert(idx >= qstrFrom);
        if (idx < (int) qstrNext)
            return qstrData + qstrPos[idx-qstrFrom];
        idx -= qstrNext;
        assert(idx < len);
        return (const char*) off + off[idx];
    }
};

struct Interp : Context {
    const QstrPool* qPool = 0;

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

    Interp () {
        vm = this;
    }

    ~Interp () {
        Object::gcStats();
        delete qPool;
        gcCheck(true);
        vm = 0;
    }

    void mark (void (*gc)(const Object&)) const override {
        Context::mark(gc);
        if (qPool != 0) gc(*qPool);
    }

    void run () {
        while (true) {
            outer();
            if (tasks.len() == 0)
                break;
            resume(tasks.at(0));
        }
    }

private:
    void outer () {
        while (true) {
            Value h = nextPending();

            if (h.isNil() && ip != 0)
                inner();                // go process lots of bytecodes
            else if (h.isObj())
                h.obj().next();         // resume the triggered handler
            else if (ip != 0)
                exception(h);           // handle the exception
            else
                break;                  // no runnable context left

            gcCheck(true);              // collect garbage, if needed
        }
    }

    void exception (Value h) {
        restoreState();
        assert(fp->excTop > 0); // simple exception, no stack unwind
        auto exc = fp->exceptionPushPop(0);
        ip = fp->bcObj.code + (int) exc[0];
        sp = fp->bottom() + (int) exc[1];
        *++sp = h;
        saveState();
    }

    uint32_t fetchVarInt (uint32_t v =0) {
        uint8_t b = 0x80;
        while (b & 0x80) {
            b = (uint8_t) *ip++;
            v = (v << 7) | (b & 0x7F);
        }
        return v;
    }

    int fetchOffset () {
        int n = (uint8_t) *ip++;
        return n | ((uint8_t) *ip++ << 8);
    }

    const char* fetchQstr () {
        return qPool->atIdx(fetchOffset() + 1);
    }

    static bool opInRange (uint8_t op, Op from, int count) {
        return (uint8_t) from <= op && op < (uint8_t) from + count;
    }

    //CG: op-init

    //CG1 op q
    void op_LoadConstString (char const* arg) {
        *++sp = arg;
    }

    //CG1 op q
    void op_LoadName (char const* arg) {
        *++sp = fp->locals->at(arg);
        assert(!sp->isNil());
    }

    //CG1 op q
    void op_StoreName (char const* arg) {
        fp->locals->atKey(arg, DictObj::Set) = *sp--;
    }

    //CG1 op
    void op_LoadConstNone () {
        *++sp = Value::None;
    }

    //CG1 op
    void op_LoadConstFalse () {
        *++sp = Value::False;
    }

    //CG1 op
    void op_LoadConstTrue () {
        *++sp = Value::True;
    }

    //CG1 op
    void op_LoadConstSmallInt () {
        *++sp = fetchVarInt(((uint8_t) *ip & 0x40) ? ~0 : 0);
    }

    //CG1 op
    void op_LoadNull () {
        *++sp = Value (); // TODO wrong
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

    //CG1 op o
    void op_SetupExcept (int arg) {
        auto exc = fp->exceptionPushPop(1);
        exc[0] = (ip - fp->bcObj.code) + arg; // int offset iso pointer
        exc[1] = sp - fp->bottom(); // again as offset, as sp is not a Value
        exc[2] = Value (); // no previous exception
        assert(exc[0].isInt() && exc[1].isInt());
    }

    //CG1 op o
    void op_PopExceptJump (int arg) {
        (void) fp->exceptionPushPop(-1);
        ip += arg;
    }

    //CG1 op
    void op_RaiseLast () {
        raise(""); // TODO
    }

    //CG1 op q
    void op_LoadAttr (char const* arg) {
        Value self;
        *sp = sp->obj().attr(arg, self);
        assert(!sp->isNil());
        if (!self.isNil() && sp->ifType<CallArgsObj>() != 0)
            *sp = new BoundMethObj (*sp, self);
    }

    //CG1 op q
    void op_StoreAttr (char const* arg) {
        assert(&sp->obj().type().type() == &ClassObj::info);
        auto& io = (InstanceObj&) sp->obj(); // can't use asType<>
        io.atKey(arg, DictObj::Set) = sp[-1];
        sp -= 2;
    }

    //CG1 op q
    void op_LoadMethod (char const* arg) {
        sp[1] = *sp;
        *sp = sp->objPtr()->attr(arg, sp[1]);
        ++sp;
        assert(!sp->isNil());
    }

    //CG1 op v
    void op_CallMethod (uint32_t arg) {
        uint8_t nargs = arg, nkw = arg >> 8; // TODO kwargs
        sp -= nargs + 2 * nkw + 1;
        doCall(*sp, nargs + 1, sp + 1);
    }

    //CG1 op s
    void op_Jump (int arg) {
        ip += arg;
    }

    //CG1 op s
    void op_UnwindJump (int arg) {
        fp->excTop -= (uint8_t) *ip; // TODO hardwired for simplest case
        ip += arg;
    }

    //CG1 op s
    void op_PopJumpIfFalse (int arg) {
        if (!(*sp--).truthy())
            ip += arg;
    }

    //CG1 op s
    void op_PopJumpIfTrue (int arg) {
        if ((*sp--).truthy())
            ip += arg;
    }

    //CG1 op v
    void op_LoadConstObj (uint32_t arg) {
        *++sp = fp->bcObj.constObjs.get(arg);
    }

    //CG1 op v
    void op_MakeFunction (uint32_t arg) {
        *++sp = new CallArgsObj (fp->bcObj.constObjs.get(arg));
    }

    //CG1 op v
    void op_MakeFunctionDefargs (uint32_t arg) {
        --sp;
        *sp = new CallArgsObj (fp->bcObj.constObjs.get(arg),
                        sp[0].ifType<TupleObj>(), sp[1].ifType<DictObj>());
    }

    //CG1 op v
    void op_CallFunction (uint32_t arg) {
        uint8_t nargs = arg, nkw = arg >> 8;
        sp -= nargs + 2 * nkw;
        doCall(*sp, nargs, sp + 1);
    }

    //CG1 op
    void op_YieldValue () {
        Value v = *sp;
        popState();
        if (!v.isNil() && !v.isNone()) { // TODO None is a valid yield result!
            assert(sp != 0 && sp >= fp->bottom());
            *sp = v;
        }
    }

    //CG1 op
    void op_ReturnValue () {
        auto ofp = fp; // fp may become invalid
        Value v = fp->result != 0 ? fp->result : *sp;
        popState();
        ofp->leave();
        if (v.ifType<ResumableObj>() != 0)
            printf("resuming!\n");
        if (sp != 0) { // null when returning from main, i.e. top level
            assert(sp >= fp->bottom());
            *sp = v;
        }
    }

    //CG1 op v
    void op_BuildTuple (uint32_t arg) {
        sp -= (int) arg - 1; // in case arg is 0
        *sp = TupleObj::create(TupleObj::info, arg, sp);
    }

    //CG1 op v
    void op_BuildList (uint32_t arg) {
        sp -= (int) arg - 1; // in case arg is 0
        *sp = new ListObj (arg, sp);
    }

    //CG1 op v
    void op_BuildSet (uint32_t arg) {
        sp -= (int) arg - 1; // in case arg is 0
        *sp = new SetObj (arg, sp);
    }

    //CG1 op v
    void op_BuildMap (uint32_t arg) {
        *++sp = new DictObj (arg);
    }

    //CG1 op v
    void op_BuildSlice (uint32_t arg) {
        Value v = arg > 2 ? *sp : Value ();
        sp -= arg - 1; // arg is 2 or 3
        *sp = new SliceObj (sp[0], sp[1], v);
    }

    //CG1 op
    void op_StoreMap () {
        sp -= 2;
        sp->asType<DictObj>().addPair(sp[2], sp[1]); // no key check
    }

    //CG1 op q
    void op_LoadGlobal (char const* arg) {
        *++sp = fp->bcObj.owner.at(arg);
        assert(!sp->isNil());
    }

    //CG1 op q
    void op_StoreGlobal (char const* arg) {
        fp->bcObj.owner.atKey(arg, DictObj::Set) = *sp--;
    }

    //CG1 op
    void op_LoadSubscr () {
        Value index = *sp--;
        *sp = sp->objPtr()->at(index);
    }

    //CG1 op
    void op_StoreSubscr () {
        auto dp = sp[-1].ifType<DictObj>();
        if (dp != 0)
            dp->atKey(sp[0], DictObj::Set) = sp[-2];
        else {
            assert(sp[0].isInt());
            auto& ao = sp[-1].asType<ArrayObj>();
            ao.set(sp[0], sp[-2]);
        }
        sp -= 3;
    }

    //CG1 op
    void op_LoadBuildClass () {
        *++sp = ClassObj::info;
    }

    //CG1 op
    void op_GetIterStack () {
        Value seq = *sp;
        sp += 3; // TODO yuck, the compiler assumes 4 stack entries are used!
        *sp = new IterObj (seq);
    }

    //CG1 op o
    void op_ForIter (int arg) {
        Value v = sp->obj().next();
        if (v.isNil()) {
            delete &sp->obj(); // IterObj
            sp -= 4;
            ip += arg;
        } else
            *++sp = v;
    }

    void inner () {
        restoreState();
        do {
#ifdef INNER_HOOK
            INNER_HOOK
#endif
            assert(ip != 0 && sp != 0 && fp != 0);
            Value* bottom = fp->bottom(); (void) bottom;
#if SHOW_INSTR_PTR
            printf("\tip %p 0x%02x sp %2d e %d ",
                    ip, (uint8_t) *ip, (int) (sp - bottom), fp->excTop);
            if (sp >= bottom)
                sp->dump(":");
            else
                printf("\n");
#endif
            assert(bottom - 1 <= sp && sp < bottom + fp->bcObj.stackSz);
            assert(fp->excTop <= fp->bcObj.excDepth);
            auto& bco = fp->bcObj; (void) bco;
            assert(bco.code <= ip && ip < bco.code + bco.size - bco.hdrSz);
            switch (*ip++) {

                //CG< op-emit
                case Op::LoadConstString:
                    op_LoadConstString(fetchQstr()); break;
                case Op::LoadName:
                    op_LoadName(fetchQstr()); break;
                case Op::StoreName:
                    op_StoreName(fetchQstr()); break;
                case Op::LoadConstNone:
                    op_LoadConstNone(); break;
                case Op::LoadConstFalse:
                    op_LoadConstFalse(); break;
                case Op::LoadConstTrue:
                    op_LoadConstTrue(); break;
                case Op::LoadConstSmallInt:
                    op_LoadConstSmallInt(); break;
                case Op::LoadNull:
                    op_LoadNull(); break;
                case Op::DupTop:
                    op_DupTop(); break;
                case Op::DupTopTwo:
                    op_DupTopTwo(); break;
                case Op::PopTop:
                    op_PopTop(); break;
                case Op::RotTwo:
                    op_RotTwo(); break;
                case Op::RotThree:
                    op_RotThree(); break;
                case Op::SetupExcept:
                    op_SetupExcept(fetchOffset()); break;
                case Op::PopExceptJump:
                    op_PopExceptJump(fetchOffset()); break;
                case Op::RaiseLast:
                    op_RaiseLast(); break;
                case Op::LoadAttr:
                    op_LoadAttr(fetchQstr()); break;
                case Op::StoreAttr:
                    op_StoreAttr(fetchQstr()); break;
                case Op::LoadMethod:
                    op_LoadMethod(fetchQstr()); break;
                case Op::CallMethod:
                    op_CallMethod(fetchVarInt()); break;
                case Op::Jump:
                    op_Jump(fetchOffset()-0x8000); break;
                case Op::UnwindJump:
                    op_UnwindJump(fetchOffset()-0x8000); break;
                case Op::PopJumpIfFalse:
                    op_PopJumpIfFalse(fetchOffset()-0x8000); break;
                case Op::PopJumpIfTrue:
                    op_PopJumpIfTrue(fetchOffset()-0x8000); break;
                case Op::LoadConstObj:
                    op_LoadConstObj(fetchVarInt()); break;
                case Op::MakeFunction:
                    op_MakeFunction(fetchVarInt()); break;
                case Op::MakeFunctionDefargs:
                    op_MakeFunctionDefargs(fetchVarInt()); break;
                case Op::CallFunction:
                    op_CallFunction(fetchVarInt()); break;
                case Op::YieldValue:
                    op_YieldValue(); break;
                case Op::ReturnValue:
                    op_ReturnValue(); break;
                case Op::BuildTuple:
                    op_BuildTuple(fetchVarInt()); break;
                case Op::BuildList:
                    op_BuildList(fetchVarInt()); break;
                case Op::BuildSet:
                    op_BuildSet(fetchVarInt()); break;
                case Op::BuildMap:
                    op_BuildMap(fetchVarInt()); break;
                case Op::BuildSlice:
                    op_BuildSlice(fetchVarInt()); break;
                case Op::StoreMap:
                    op_StoreMap(); break;
                case Op::LoadGlobal:
                    op_LoadGlobal(fetchQstr()); break;
                case Op::StoreGlobal:
                    op_StoreGlobal(fetchQstr()); break;
                case Op::LoadSubscr:
                    op_LoadSubscr(); break;
                case Op::StoreSubscr:
                    op_StoreSubscr(); break;
                case Op::LoadBuildClass:
                    op_LoadBuildClass(); break;
                case Op::GetIterStack:
                    op_GetIterStack(); break;
                case Op::ForIter:
                    op_ForIter(fetchOffset()); break;
                //CG>

                default: {
                    auto v = (uint8_t) ip[-1];
                    if (opInRange(v, Op::LoadConstSmallIntMulti , 64)) {
                        v -= (uint8_t) Op::LoadConstSmallIntMulti ;
                        *++sp = v - 16;
                    } else if (opInRange(v, Op::LoadFastMulti, 16)) {
                        v -= (uint8_t) Op::LoadFastMulti ;
                        assert(!fp->fastSlot(v).isNil());
                        *++sp = fp->fastSlot(v);
                    } else if (opInRange(v, Op::StoreFastMulti, 16)) {
                        v -= (uint8_t) Op::StoreFastMulti;
                        fp->fastSlot(v) = *sp--;
                    } else if (opInRange(v, Op::UnaryOpMulti , 7)) {
                        v -= (uint8_t) Op::UnaryOpMulti;
                        *sp = sp->unOp((UnOp) v);
                    } else if (opInRange(v, Op::BinaryOpMulti , 35)) {
                        v -= (uint8_t) Op::BinaryOpMulti;
                        Value rhs = *sp--;
                        *sp = sp->binOp((BinOp) v, rhs);
                    } else
                        assert(false);
                }
            }
        } while (pending == 0);
        saveState();
    }
};
