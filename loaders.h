// Bytecode loaders: Loader loads .mpy files, Linker is for an internal test.

#if !VERBOSE_LOAD // see main.cpp
#define printf(...)
#endif

struct Loader {
    const LookupObj* modules; // TODO cleanup when?
    const QstrPool* qPool;
    const uint8_t* dp;
    char* qBuf;
    char* qNext;
    VecOf<const char*> qVec;
    uint8_t* bcBuf;
    uint8_t* bcNext;
    uint8_t* bcLimit;
    VecOf<uint16_t> qWin;

    struct Prelude {
        uint32_t n_state;
        uint32_t n_exc_stack;
        uint32_t scope_flags;
        uint32_t n_pos_args;
        uint32_t n_kwonly_args;
        uint32_t n_def_pos_args;
        uint32_t n_info = 0;
        uint32_t n_cell = 0;

        int load (Loader& l) {
            uint8_t z = *l.dp++; /* xSSSSEAA */
            n_state = (z >> 3) & 0xf;
            n_exc_stack = (z >> 2) & 0x1;
            scope_flags = 0;
            n_pos_args = z & 0x3;
            n_kwonly_args = 0;
            n_def_pos_args = 0;
            for (int n = 0; z & 0x80; ++n) {
                z = *l.dp++; /* xFSSKAED */
                n_state |= (z & 0x30) << (2 * n);
                n_exc_stack |= (z & 0x02) << n;
                scope_flags |= ((z & 0x40) >> 6) << n;
                n_pos_args |= (z & 0x4) << n;
                n_kwonly_args |= ((z & 0x08) >> 3) << n;
                n_def_pos_args |= (z & 0x1) << n;
            }
            n_state += 1;
            printf("n_state        %d\n", n_state);
            printf("n_exc_stack    %d\n", n_exc_stack);
            printf("scope_flags    %d\n", scope_flags);
            printf("n_pos_args     %d\n", n_pos_args);
            printf("n_kwonly_args  %d\n", n_kwonly_args);
            printf("n_def_pos_args %d\n", n_def_pos_args);

            n_info = 0;
            n_cell = 0;
            z = 0x80;
            for (int n = 0; z & 0x80; ++n) {
                z = *l.dp++; /* xIIIIIIC */
                n_cell |= (z & 1) << n;
                n_info |= ((z & 0x7e) >> 1) << (6 * n);
            }
            printf("n_info         %d\n", n_info);
            printf("n_cell         %d\n", n_cell);
            return n_info + n_cell;
        }
    } prelude;

    ModuleObj* load (const uint8_t* data, int index =0) {
        dp  = data;
        if (*dp++ != 'M')
            return 0; // incorrect file format

        printf("version %d\n", dp[0]);
        printf("features 0x%02X\n", dp[1]);
        printf("intbits %d\n", dp[2]);
        dp += 3;
        int n = varInt();
        printf("qwin %u\n", n);
        qWin.ins(0, n); // qstr window

        qBuf = qNext = (char*) malloc(500); // TODO need to handle pre-loaded

        auto mo = new ModuleObj;
        mo->init = &loadRaw(*mo); // circular: Module -> Bytecode -> Module

        printf("qUsed #%d %db\n", (int) qVec.length(), (int) (qNext-qBuf));
        qPool = QstrPool::create((uint8_t*) qBuf, qVec.length());

        return mo;
    }

    uint32_t varInt () {
        uint32_t v = 0;
        uint8_t b = 0x80;
        while (b & 0x80) {
            b = *dp++;
            v = (v << 7) | (b & 0x7F);
        }
        return v;
    }

    const uint8_t* skip (uint32_t n) {
        auto p = (const uint8_t*) dp;
        dp += n;
        return p;
    }

    int loadQstr () {
        uint8_t len = varInt();
        if (len == 0)
            return *dp++;
        if (len & 1)
            return winQstr(len>>1);
        len >>= 1;
        auto s = qNext;
        for (int i = 0; i < len; ++i)
            *qNext++ = *dp++;
        *qNext++ = 0;
        printf("q:%s\n", s);
        int n = qVec.length();
        qVec.set(n, s);
        qWin.del(qWin.length()-1);
        qWin.ins(0);
        n += qstrNext;
        qWin.set(0, n);
        return n;
    }

    void addQstr (int i, const char* s) {
        qVec.set(i, qNext);
        auto n = strlen(s) + 1;
        memcpy(qNext, s, n);
        qNext += n;
    }

    int winQstr (int i) {
        int n = qWin.get(i);
        qWin.del(i);
        qWin.ins(0);
        qWin.set(0, n);
        return n;
    }

    int storeQstr () {
        auto n = loadQstr();
        assert(n > 0);
        *bcNext++ = --n;
        *bcNext++ = n >> 8;
        return n;
    }

    const BytecodeObj& loadRaw (ModuleObj& modobj) {
        auto& bc = *new BytecodeObj (modobj);
        printf("ha\n");

        auto typsiz = varInt();
        printf("type %d size %d (%d)\n", typsiz & 3, typsiz >> 2, typsiz);

        auto savedDp = dp;
        auto nskip = prelude.load(*this);
        auto npre = dp - savedDp;

        bcBuf = bcNext = (uint8_t*) malloc(typsiz>>2);
        bcLimit = bcBuf + (typsiz>>2) - npre;

        bc.stackSz = prelude.n_state;
        bc.excDepth = prelude.n_exc_stack;
        bc.scope = prelude.scope_flags;
        bc.n_pos = prelude.n_pos_args;
        bc.n_kwonly = prelude.n_kwonly_args;
        bc.n_def_pos = prelude.n_def_pos_args;
        bc.hdrSz = prelude.n_info + prelude.n_cell;
        bc.size = typsiz >> 2;
        printf("raw sc %u np %u hs %u sz %u ns %u nx %u ko %u dp %u\n",
                bc.scope, bc.n_pos, bc.hdrSz, bc.size,
                bc.stackSz, bc.excDepth, bc.n_kwonly, bc.n_def_pos);

        auto n1 = storeQstr();
        auto n2 = storeQstr();
        (void) n1; (void) n2;
        printf("qstr %d %d npre %d nskip %d\n", n1+1, n2+1, (int) npre, nskip);

        for (int i = 4; i < nskip; ++i)
            *bcNext++ = *dp++;

        bc.code = (OpPtrRO) bcNext;

        loadOps();

        printf("subs %08x\n", *(const uint32_t*) dp);
        printf("jump %08x\n", *(const uint32_t*) bc.code);

        bc.nData = varInt();
        bc.nCode = varInt();
        printf("nData %d nCode %d\n", bc.nData, bc.nCode);

        for (int i = 0; i < bc.n_pos + bc.n_kwonly; ++i)
            loadQstr(); // is this to adjust qWin ?

        for (int i = 0; i < bc.nData; ++i) {
            auto type = *dp++;
            (void) type;
            assert(type != 'e'); // TODO ellipsis
            auto sz = varInt();
            auto ptr = skip(sz);
            printf("  obj %d = type %c %ub = %s\n", i, type, sz, ptr);
            bc.constObjs.set(i, (const char*) ptr);
        }
        for (int i = 0; i < bc.nCode; ++i) {
            printf("  raw %d:\n", i+bc.nData);
            bc.constObjs.set(i+bc.nData, loadRaw(modobj));
        }

        return bc;
    }

    void loadOps () {
        constexpr auto MP_BC_MASK_EXTRA_BYTE = 0x9e;
        constexpr auto MP_BC_FORMAT_BYTE     = 0;
        constexpr auto MP_BC_FORMAT_QSTR     = 1;
        constexpr auto MP_BC_FORMAT_VAR_UINT = 2;
        constexpr auto MP_BC_FORMAT_OFFSET   = 3;

        while (bcNext < bcLimit) {
            uint8_t op = *dp++;
            *bcNext++ = op;
            // see #define MP_BC_FORMAT(op) in py/pc0.h
            int f = (0x000003A4 >> 2*(op>>4)) & 3;
            switch (f) {
                case MP_BC_FORMAT_BYTE:
                    printf("  B: 0x%02X\n", op);
                    break;
                case MP_BC_FORMAT_QSTR: {
                    auto n = storeQstr() + 1;
                    assert(n > 0);
                    auto s = n < (int) qstrNext ? qstrData + qstrPos[n-1]
                                                : qVec.get(n - qstrNext);
                    (void) s;
                    printf("  Q: 0x%02X (%u) %s\n", op, n, s);
                    break;
                }
                case MP_BC_FORMAT_VAR_UINT: {
                    auto savedDp = dp;
                    auto n = varInt();
                    printf("  V: 0x%02X %d\n", op, n);
                    (void) n;
                    while (savedDp < dp)
                        *bcNext++ = *savedDp++;
                    break;
                }
                case MP_BC_FORMAT_OFFSET: {
                    uint8_t op1 = *dp++;
                    uint8_t op2 = *dp++;
                    printf("  O: 0x%02X %04X\n", op, op1 | (op2 << 8));
                    *bcNext++ = op1;
                    *bcNext++ = op2;
                    break;
                }
            }
            if (f != MP_BC_FORMAT_QSTR && (op & MP_BC_MASK_EXTRA_BYTE) == 0) {
                auto n = *dp++;
                *bcNext++ = n;
                printf("   x 0x%02X\n", n);
                while (n-- > 0)
                    *bcNext++ = *dp++;
            }
        }
    }
};

struct Linker {
    const LookupObj* modules; // TODO cleanup when?
    const QstrPool* qPool;
    const uint8_t* dp;

    ModuleObj* load (const uint8_t* data, int index) {
        dp  = data;
        if (varInt() != 123)
            return 0; // incorrect file format

        auto nMods = varInt();
        (void) varInt(); // modLen, implicit in modCount

        auto mods = (LookupObj::Item*) malloc(nMods * sizeof (LookupObj::Item));
        for (uint32_t i = 0; i < nMods; ++i) {
            printf("module %d: %s\n", i, (const char*) dp);
            mods[i].k = (const char*) dp;
            dp += strlen((const char*) dp) + 1;
        }

        auto qCount = varInt();
        auto qBytes = varInt();
        printf("qPool size #%d, %db\n", qCount, qBytes);
        qPool = QstrPool::create(skip(qBytes), qCount);

        ModuleObj* mainMod = 0;
        for (int i = 0; i < (int) nMods; ++i) {
            auto mo = new ModuleObj;
            if (i <= index)
                mainMod = mo;
            mo->init = &loadRaw(*mo); // circular: Module -> Bytecode -> Module
            mods[i].v = mo;
        }

        modules = new LookupObj (mods, nMods); // TODO only one module used!
        return mainMod;
    }

    uint32_t varInt () {
        uint32_t v = 0;
        uint8_t b = 0x80;
        while (b & 0x80) {
            b = (uint8_t) *dp++;
            v = (v << 7) | (b & 0x7F);
        }
        return v;
    }

    const uint8_t* skip (uint32_t n) {
        auto p = (const uint8_t*) dp;
        dp += n;
        return p;
    }

    const BytecodeObj& loadRaw(ModuleObj& modobj) {
        auto& bc = *new BytecodeObj (modobj);
        bc.stackSz = varInt();
        bc.excDepth = varInt();
        bc.scope = varInt();
        bc.n_pos = varInt();
        bc.n_kwonly = varInt();
        bc.n_def_pos = varInt();
        bc.hdrSz = varInt();
        bc.size = varInt();
        bc.code = (OpPtrRO) skip(bc.size) + bc.hdrSz;
        bc.nData = varInt();
        bc.nCode = varInt();
        printf("raw sc %u np %u ns %u len %u no %u nr %u",
                bc.scope, bc.n_pos, bc.hdrSz, bc.size, bc.nData, bc.nCode);
        printf(" ns %u nx %u ko %u dp %u\n",
                bc.stackSz, bc.excDepth, bc.n_kwonly, bc.n_def_pos);
        for (int i = 0; i < bc.nData; ++i) {
            auto sz = varInt();
            auto ptr = skip(sz);
            printf("  obj %d = %ub = %s\n", i, sz, ptr);
            bc.constObjs.set(i, (const char*) ptr);
        }
        for (int i = 0; i < bc.nCode; ++i) {
            printf("  raw %d:\n", i+bc.nData);
            bc.constObjs.set(i+bc.nData, loadRaw(modobj));
        }
        return bc;
    }
};

#undef printf // !VERBOSE_LOAD
