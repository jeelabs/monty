// pyvm.h - virtual machine for bytecodes emitted by MicroPython 1.12

//CG: on op:print # set to "on" to enable per-opcode debug output

struct PyVM {
    Value* sp;
    OpPtrRO ip;
    Context& ctx;

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
        //return qPool->atIdx(fetchOffset() + 1);
        assert(false); return nullptr;
    }

    static bool opInRange (uint8_t op, Op from, int count) {
        return (uint8_t) from <= op && op < (uint8_t) from + count;
    }

    void instructionTrace () {
#if SHOW_INSTR_PTR
        auto base = ctx.stack.begin() + ctx.Extra;
        printf("\tip %p 0x%02x sp %2d e %d : ",
                ip, (uint8_t) *ip, (int) (sp - base), (int) ctx.stack[ctx.Ep]);
        if (sp >= base)
            sp->dump();
        printf("\n");
#endif
    }

    //CG: op-init

    //CG2 op
    void op_LoadNull () {
        printf("LoadNull\n");
        *++sp = Value {};
    }
    //CG2 op
    void op_LoadConstNone () {
        printf("LoadConstNone\n");
        *++sp = Null;
    }
    //CG2 op
    void op_LoadConstFalse () {
        printf("LoadConstFalse\n");
        *++sp = False;
    }
    //CG2 op
    void op_LoadConstTrue () {
        printf("LoadConstTrue\n");
        *++sp = True;
    }
    //CG2 op q
    void op_LoadConstString (char const* arg) {
        printf("LoadConstString %s\n", arg);
        *++sp = arg;
    }
    //CG2 op
    void op_LoadConstSmallInt () {
        printf("LoadConstSmallInt\n");
        *++sp = fetchVarInt(((uint8_t) *ip & 0x40) ? ~0 : 0);
    }
    //CG2 op v
    void op_LoadConstObj (uint32_t arg) {
        printf("LoadConstObj %u\n", (unsigned) arg);
        assert(false); // TODO
    }

    //CG2 op
    void op_DupTop () {
        printf("DupTop\n");
        ++sp; sp[0] = sp[-1];
    }
    //CG2 op
    void op_DupTopTwo () {
        printf("DupTopTwo\n");
        sp += 2; sp[0] = sp[-2]; sp[-1] = sp[-3];
    }
    //CG2 op
    void op_PopTop () {
        printf("PopTop\n");
        --sp;
    }
    //CG2 op
    void op_RotTwo () {
        printf("RotTwo\n");
        auto v = sp[0]; sp[0] = sp[-1]; sp[-1] = v;
    }
    //CG2 op
    void op_RotThree () {
        printf("RotThree\n");
        auto v = sp[0]; sp[0] = sp[-1]; sp[-1] = sp[-2]; sp[-2] = v;
    }

    //CG2 op s
    void op_Jump (int arg) {
        printf("Jump %d\n", arg);
        ip += arg;
    }
    //CG2 op s
    void op_PopJumpIfFalse (int arg) {
        printf("PopJumpIfFalse %d\n", arg);
        if (!sp->truthy())
            ip += arg;
        --sp;
    }
    //CG2 op s
    void op_PopJumpIfTrue (int arg) {
        printf("PopJumpIfTrue %d\n", arg);
        if (sp->truthy())
            ip += arg;
        --sp;
    }

    //CG2 op q
    void op_LoadName (char const* arg) {
        printf("LoadName %s\n", arg);
        *++sp = ctx.locals().at(arg);
        assert(!sp->isNil());
    }
    //CG2 op q
    void op_StoreName (char const* arg) {
        printf("StoreName %s\n", arg);
        ctx.locals().at(arg) = *sp--;
    }
    //CG2 op q
    void op_LoadGlobal (char const* arg) {
        printf("LoadGlobal %s\n", arg);
        *++sp = ctx.globals().at(arg);
        assert(!sp->isNil());
    }
    //CG2 op q
    void op_StoreGlobal (char const* arg) {
        printf("StoreGlobal %s\n", arg);
        ctx.globals().at(arg) = *sp--;
    }
    //CG2 op q
    void op_LoadAttr (char const* arg) {
        printf("LoadAttr %s\n", arg);
        assert(false); // TODO
    }
    //CG2 op q
    void op_StoreAttr (char const* arg) {
        printf("StoreAttr %s\n", arg);
        assert(false); // TODO
    }
    //CG2 op
    void op_LoadSubscr () {
        printf("LoadSubscr\n");
        assert(false); // TODO
    }
    //CG2 op
    void op_StoreSubscr () {
        printf("StoreSubscr\n");
        assert(false); // TODO
    }

    //CG2 op v
    void op_BuildSlice (uint32_t arg) {
        printf("BuildSlice %u\n", (unsigned) arg);
        sp -= arg - 1; // arg is 2 or 3
        *sp = Slice::create(ctx.asArgs(arg, sp));
    }
    //CG2 op v
    void op_BuildTuple (uint32_t arg) {
        printf("BuildTuple %u\n", (unsigned) arg);
        sp -= (int) arg - 1; // signed, if arg is 0
        *sp = Tuple::create(ctx.asArgs(arg, sp));
    }
    //CG2 op v
    void op_BuildList (uint32_t arg) {
        printf("BuildList %u\n", (unsigned) arg);
        sp -= (int) arg - 1; // signed, if arg is 0
        *sp = List::create(ctx.asArgs(arg, sp));
    }
    //CG2 op v
    void op_BuildSet (uint32_t arg) {
        printf("BuildSet %u\n", (unsigned) arg);
        sp -= (int) arg - 1; // signed, if arg is 0
        *sp = Set::create(ctx.asArgs(arg, sp));
    }
    //CG2 op v
    void op_BuildMap (uint32_t arg) {
        printf("BuildMap %u\n", (unsigned) arg);
        *++sp = Dict::create(ctx.asArgs(arg));
    }
    //CG2 op
    void op_StoreMap () {
        printf("StoreMap\n");
        assert(false); // TODO
    }

    //CG2 op o
    void op_SetupExcept (int arg) {
        printf("SetupExcept %d\n", arg);
        assert(false); // TODO
    }
    //CG2 op o
    void op_PopExceptJump (int arg) {
        printf("PopExceptJump %d\n", arg);
        assert(false); // TODO
    }
    //CG2 op
    void op_RaiseLast () {
        printf("RaiseLast\n");
        assert(false); // TODO
    }
    //CG2 op s
    void op_UnwindJump (int arg) {
        printf("UnwindJump %d\n", arg);
        assert(false); // TODO
    }

    //CG2 op
    void op_LoadBuildClass () {
        printf("LoadBuildClass\n");
        *++sp = Class::info;
    }
    //CG2 op q
    void op_LoadMethod (char const* arg) {
        printf("LoadMethod %s\n", arg);
        assert(false); // TODO
    }
    //CG2 op v
    void op_CallMethod (uint32_t arg) {
        printf("CallMethod %u\n", (unsigned) arg);
        assert(false); // TODO
    }
    //CG2 op v
    void op_MakeFunction (uint32_t arg) {
        printf("MakeFunction %u\n", (unsigned) arg);
        assert(false); // TODO
    }
    //CG2 op v
    void op_MakeFunctionDefargs (uint32_t arg) {
        printf("MakeFunctionDefargs %u\n", (unsigned) arg);
        assert(false); // TODO
    }
    //CG2 op v
    void op_CallFunction (uint32_t arg) {
        printf("CallFunction %u\n", (unsigned) arg);
        assert(false); // TODO
    }
    //CG2 op
    void op_ReturnValue () {
        printf("ReturnValue\n");
        assert(false); // TODO
    }
    //CG2 op
    void op_YieldValue () {
        printf("YieldValue\n");
        assert(false); // TODO
    }

    //CG2 op
    void op_GetIterStack () {
        printf("GetIterStack\n");
        assert(false); // TODO
    }
    //CG2 op o
    void op_ForIter (int arg) {
        printf("ForIter %d\n", arg);
        assert(false); // TODO
    }

    PyVM (Context& context) : ctx (context) {
        size_t spOff = ctx.stack[ctx.Sp];
        sp = &ctx.stack[spOff];
        ip = (OpPtrRO) ctx.ipBase() + ctx.stack[ctx.Ip];

        do {
            instructionTrace();
            switch (*ip++) {

                //CG< op-emit
                case Op::LoadNull:
                    op_LoadNull(); break;
                case Op::LoadConstNone:
                    op_LoadConstNone(); break;
                case Op::LoadConstFalse:
                    op_LoadConstFalse(); break;
                case Op::LoadConstTrue:
                    op_LoadConstTrue(); break;
                case Op::LoadConstString:
                    op_LoadConstString(fetchQstr()); break;
                case Op::LoadConstSmallInt:
                    op_LoadConstSmallInt(); break;
                case Op::LoadConstObj:
                    op_LoadConstObj(fetchVarInt()); break;
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
                case Op::Jump:
                    op_Jump(fetchOffset()-0x8000); break;
                case Op::PopJumpIfFalse:
                    op_PopJumpIfFalse(fetchOffset()-0x8000); break;
                case Op::PopJumpIfTrue:
                    op_PopJumpIfTrue(fetchOffset()-0x8000); break;
                case Op::LoadName:
                    op_LoadName(fetchQstr()); break;
                case Op::StoreName:
                    op_StoreName(fetchQstr()); break;
                case Op::LoadGlobal:
                    op_LoadGlobal(fetchQstr()); break;
                case Op::StoreGlobal:
                    op_StoreGlobal(fetchQstr()); break;
                case Op::LoadAttr:
                    op_LoadAttr(fetchQstr()); break;
                case Op::StoreAttr:
                    op_StoreAttr(fetchQstr()); break;
                case Op::LoadSubscr:
                    op_LoadSubscr(); break;
                case Op::StoreSubscr:
                    op_StoreSubscr(); break;
                case Op::BuildSlice:
                    op_BuildSlice(fetchVarInt()); break;
                case Op::BuildTuple:
                    op_BuildTuple(fetchVarInt()); break;
                case Op::BuildList:
                    op_BuildList(fetchVarInt()); break;
                case Op::BuildSet:
                    op_BuildSet(fetchVarInt()); break;
                case Op::BuildMap:
                    op_BuildMap(fetchVarInt()); break;
                case Op::StoreMap:
                    op_StoreMap(); break;
                case Op::SetupExcept:
                    op_SetupExcept(fetchOffset()); break;
                case Op::PopExceptJump:
                    op_PopExceptJump(fetchOffset()); break;
                case Op::RaiseLast:
                    op_RaiseLast(); break;
                case Op::UnwindJump:
                    op_UnwindJump(fetchOffset()-0x8000); break;
                case Op::LoadBuildClass:
                    op_LoadBuildClass(); break;
                case Op::LoadMethod:
                    op_LoadMethod(fetchQstr()); break;
                case Op::CallMethod:
                    op_CallMethod(fetchVarInt()); break;
                case Op::MakeFunction:
                    op_MakeFunction(fetchVarInt()); break;
                case Op::MakeFunctionDefargs:
                    op_MakeFunctionDefargs(fetchVarInt()); break;
                case Op::CallFunction:
                    op_CallFunction(fetchVarInt()); break;
                case Op::ReturnValue:
                    op_ReturnValue(); break;
                case Op::YieldValue:
                    op_YieldValue(); break;
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
                        assert(!ctx.fastSlot(v).isNil());
                        *++sp = ctx.fastSlot(v);
                    } else if (opInRange(v, Op::StoreFastMulti, 16)) {
                        v -= (uint8_t) Op::StoreFastMulti;
                        ctx.fastSlot(v) = *sp--;
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
    }

    void save (Context& ctx) const {
        // FIXME wrong when stack is not the same as on entry
        ctx.stack[ctx.Sp] = sp - &ctx.stack[0];
        ctx.stack[ctx.Ip] = ip - (OpPtrRO) ctx.ipBase();
    }
};
