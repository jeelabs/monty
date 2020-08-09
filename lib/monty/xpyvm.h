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

    //CG: op-init

    //CG2 op q
    void op_LoadConstString (char const* arg) {
        printf("LoadConstString %s\n", arg);
        *++sp = arg;
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
    //CG2 op
    void op_LoadConstSmallInt () {
        printf("LoadConstSmallInt\n");
        *++sp = fetchVarInt(((uint8_t) *ip & 0x40) ? ~0 : 0);
    }
    //CG2 op
    void op_LoadNull () {
        printf("LoadNull\n");
        *++sp = Value {};
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

    //CG2 op v
    void op_BuildSlice (uint32_t arg) {
        printf("BuildSlice %u\n", (unsigned) arg);
        sp -= arg - 1; // arg is 2 or 3
        *sp = ctx.create<Slice>(arg, sp);
    }
    //CG2 op v
    void op_BuildTuple (uint32_t arg) {
        printf("BuildTuple %u\n", (unsigned) arg);
        sp -= (int) arg - 1; // signed, if arg is 0
        *sp = ctx.create<Tuple>(arg, sp);
    }
    //CG2 op v
    void op_BuildList (uint32_t arg) {
        printf("BuildList %u\n", (unsigned) arg);
        sp -= (int) arg - 1; // signed, if arg is 0
        *sp = ctx.create<List>(arg, sp);
    }
    //CG2 op v
    void op_BuildSet (uint32_t arg) {
        printf("BuildSet %u\n", (unsigned) arg);
        sp -= (int) arg - 1; // signed, if arg is 0
        *sp = ctx.create<Set>(arg, sp);
    }
    //CG2 op v
    void op_BuildMap (uint32_t arg) {
        printf("BuildMap %u\n", (unsigned) arg);
        *++sp = ctx.create<Dict>(arg);
    }

    PyVM (Context& context) : ctx (context) {
        size_t spOff = ctx.stack[ctx.Sp];
        sp = &ctx.stack[spOff];
        ip = (OpPtrRO) ctx.ipBase() + ctx.stack[ctx.Ip];

        do {
            switch (*ip++) {

                //CG< op-emit
                case Op::LoadConstString:
                    op_LoadConstString(fetchQstr()); break;
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
