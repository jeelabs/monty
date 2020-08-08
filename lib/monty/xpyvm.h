// pyvm.h - virtual machine for bytecodes emitted by MicroPython 1.12

//CG: on op:print # set to "on" to enable per-opcode debug output

struct PyVM {
    Value* sp;
    OpPtrRO ip;
    volatile uint32_t pending;

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

    PyVM () {
        sp = nullptr;
        ip = nullptr;
        pending = 0;

        do {
            switch (*ip++) {

                //CG2 op-emit
                case Op::LoadConstString:
                    op_LoadConstString(fetchQstr()); break;

                default:
                    assert(false);
            }
        } while (pending == 0);

        // ... = sp; etc
    }
};
