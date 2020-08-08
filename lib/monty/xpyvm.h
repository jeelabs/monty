// pyvm.h - virtual machine for bytecodes emitted by MicroPython 1.12

//CG: on op:print # set to "on" to enable per-opcode debug output

struct PyVM {
    Value* sp;
    OpPtrRO ip;

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
    void op_LoadConstString (const char* arg) {
        printf("LoadConstString %s\n", arg);
        *++sp = arg;
    }
    //CG2 op
    void op_LoadConstNone () {
        printf("LoadConstNone\n");
        *++sp = Value::None;
    }
    //CG2 op
    void op_LoadConstFalse () {
        printf("LoadConstFalse\n");
        *++sp = Value::False;
    }
    //CG2 op
    void op_LoadConstTrue () {
        printf("LoadConstTrue\n");
        *++sp = Value::True;
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
        if (!(*sp--).truthy())
            ip += arg;
    }
    //CG2 op s
    void op_PopJumpIfTrue (int arg) {
        printf("PopJumpIfTrue %d\n", arg);
        if ((*sp--).truthy())
            ip += arg;
    }

    PyVM (Context& ctx) {
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
