// import.h - importing and loading bytecodes

#define VERBOSE_LOAD 0 // show .mpy load progress with detailed file info

#if VERBOSE_LOAD
#define debugf printf
#else
#define debugf(...)
#endif

struct CodePrefix {
    int16_t constOff;
    int16_t code;
    int16_t nInf;
    int8_t sTop;    // stack top
    int8_t nExc;    // number of exception levels
    int8_t flag;    // flag bits
    int8_t nPos;    // number of positional args
    int8_t nKwo;    // number of keyword-only args
    int8_t nDef;    // number of default args
    int8_t nCel;    // number of closure cells

    auto isGenerator () const -> bool { return flag & 1; }
    auto wantsMap () const -> bool { return (flag >> 1) & 1; }
    auto wantsVec () const -> bool { return (flag >> 2) & 1; }

    void decodePrelude (uint8_t const*& dp) {
        uint8_t z = *dp++; /* xSSSSEAA */
        sTop = (z >> 3) & 0x0F;
        nExc = (z >> 2) & 0x01;
        nPos = z & 0x3;
        flag = nKwo = nDef = 0;

        for (int n = 0; z & 0x80; ++n) {
            z = *dp++; /* xFSSKAED */
            sTop |= (z & 0x30) << (2 * n);
            nExc |= (z & 0x02) << n;
            flag |= ((z & 0x40) >> 6) << n;
            nPos |= (z & 0x4) << n;
            nKwo |= ((z & 0x08) >> 3) << n;
            nDef |= (z & 0x1) << n;
        }
        sTop += 1;

        nInf = 0;
        nCel = 0;
        z = 0x80;
        for (int n = 0; z & 0x80; ++n) {
            z = *dp++; /* xIIIIIIC */
            nCel |= (z & 1) << n;
            nInf |= ((z & 0x7e) >> 1) << (6 * n);
        }

        debugf("sTop %d nExc %d flag %d nPos %d nKwo %d nDef %d nInf %d nCel %d\n",
                    sTop, nExc, flag, nPos, nKwo, nDef, nInf, nCel);
    }
};

// was: CG3 type <bytecode>
struct Bytecode : List, CodePrefix {
    static Type info;
    auto type () const -> Type const& override { return info; }

    auto base () const -> uint8_t const* { return (uint8_t const*) (this+1); }
    auto start () const -> uint8_t const* { return base() + code; }

    // determine the source line number, given an offset into the bytecode
    auto findLine (uint32_t off) const -> uint32_t {
        uint32_t line = 1;
        for (auto p = base() + 4; *p != 0; ++p) {
            uint32_t b, l;
            if (*p & 0x80) { // 0b1LLLBBBB 0bLLLLLLLL
                b = *p & 0x0F;
                l = (*p & 0x70) << 4;
                l += *++p;
            } else { // 0b0LLBBBBB
                b = *p & 0x1F;
                l = *p >> 5;
            }
            if (off < b)
                break;
            off -= b;
            line += l;
        }
        return line;
    }

    // this function is abused for line lookup, keeping Bytecode a hidden type
    auto getAt (Value k) const -> Value override {
        assert(k.isInt());
        int i = k;
        if (i >= 0)
            return findLine(i);
        auto p = reinterpret_cast<uint16_t const*>(base());
        // index with -1 or -2 to obtain the file/function names, respectively
        if (i == -1)
            ++p;
        return Q(*p+1); // return first or second qstr in the bytecode body
    }

    static auto load (void const*, Value) -> Callable*;

    friend struct Loader;
};

struct Loader {
    uint8_t const* dp;
    VecOf<uint16_t> qWin;
    uint8_t* bcBuf;
    uint8_t* bcNext;
    uint8_t* bcLimit;

    VaryVec* vvec;          // convert: new module format if converting, else 0
    ByteVec constData;      // convert: all collected const data
    uint16_t constNext {0}; // convert: index of next unused const entry

    Loader (VaryVec* vv =nullptr) : vvec (vv) {}

    Callable* load (uint8_t const* data, Value nm) {
        assert(data != nullptr);
        dp  = data;
        if (*dp++ != 'M')
            return 0; // incorrect file format

        debugf("version %d features 0x%02x intbits %d\n", dp[0], dp[1], dp[2]);
        dp += 3;

        int n = varInt();
        debugf("qwin %d\n", n);
        qWin.insert(0, n); // qstr window

        auto& bc = loadRaw();

        if (vvec != nullptr) {
            auto n = vvec->size();
            vvec->insert(n, 2);
            constInsertVaryInt(0, constNext);
            debugf("vvec %d: %d consts in %d bytes\n",
                    n, constNext, constData.size());
            vvec->atSet(n, constData.begin(), (int) constData.size());
        }

        debugf("qLast %d %s\n", Q::last(), Q::str(Q::last()));

        auto mod = new Module (nm);
        return new Callable (bc, mod);
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

    // poor man's varyint emitter, works for up to 14-bit ints
    auto constInsertVaryInt (uint32_t pos, uint32_t val) -> uint32_t {
        assert(val < 16384);
        constData.insert(pos + (val < 128 ? 1 : 2));
        if (val < 128)
            constData[pos++] = val;
        else {
            constData[pos++] = val | 0x80;
            constData[pos++] = val >> 7;
        }
        return pos;
    }

    uint8_t const* skip (uint32_t n) {
        auto p = (uint8_t const*) dp;
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

        char qBuf [30]; // TODO yuck, temp buf to terminate with a null byte
        assert(len < sizeof qBuf - 1);
        for (int i = 0; i < len; ++i)
            qBuf[i] = *dp++;
        qBuf[len] = 0;

        auto n = Q::make(qBuf);
        debugf("q:%s <%d>\n", qBuf, n);
        if (vvec != nullptr) {
            assert(n > 256); // not in std set
            n = n & 0xFF;
            auto nq = vvec->atLen(0);
            if (n == nq) {
                vvec->insert(n);
                vvec->atSet(n, qBuf, len+1);
                uint8_t h = Q::hash(qBuf, len);
                vvec->atAdj(0, n+1);
                vvec->atGet(0)[n] = h;
                debugf("lq %02x %02x %s\n", n, h, qBuf);
            }
            n += 0x100;
        }

        qWin.pop();
        qWin.push(n);
        return n;
    }

    int winQstr (int i) {
        int n = qWin[i];
        qWin.remove(i);
        qWin.push(n);
        return n;
    }

    int storeQstr () {
        auto n = loadQstr();
        assert(n > 0);
        *bcNext++ = --n;
        *bcNext++ = n >> 8;
        return n;
    }

    Bytecode const& loadRaw () {
        auto typsiz = varInt();
        auto bCount = typsiz >> 2;
        debugf("type %d size %d (%d)\n", typsiz & 3, bCount, typsiz);

        // bytecode will be stored in extra bytes allocated after Bytecode
        auto& bc = *new (bCount) Bytecode;

        auto savedDp = dp;
        bc.decodePrelude(dp);
        int npre = dp - savedDp;
        auto nskip = bc.nInf + bc.nCel;

        bcBuf = bcNext = (uint8_t*) (&bc + 1);
        bcLimit = bcBuf + bCount - npre;

        auto n1 = storeQstr();
        auto n2 = storeQstr();
        (void) n1; (void) n2;
        debugf("qstr %d %d npre %d nskip %d\n", n1+1, n2+1, npre, nskip);

        for (int i = 4; i < nskip; ++i)
            *bcNext++ = *dp++;

        for (int i = -bc.nCel; i < 0; ++i)
            debugf("deref %d: %d\n", i, bcNext[i]);
        bc.code = bcNext - bcBuf;
        debugf("nCel %d code %d\n", bc.nCel, bc.code);

        loadOps();

        auto nData = varInt();
        auto nCode = varInt();
        debugf("nData %d nCode %d\n", nData, nCode);

        if (vvec != nullptr) {
            CodePrefix pfx = bc; // new bytecode prefix when converting
            pfx.constOff = constNext;

            int n = vvec->size();
            vvec->insert(n);
            vvec->atAdj(n, sizeof pfx + bCount);
            auto p = vvec->atGet(n);
            memcpy(p, &pfx, sizeof pfx);
            memcpy(p + sizeof pfx, (uint8_t const*) (&bc + 1), bCount);
        }

        bc.adj(bc.nPos + bc.nKwo + nData + nCode); // pre-alloc
        constNext += bc.nPos + bc.nKwo + nData + nCode;

        auto dpOff = dp;

        for (int i = 0; i < bc.nPos + bc.nKwo; ++i) {
            auto qs = loadQstr();
            debugf("bc %d qs %d: %s\n", bc.size(), qs, Q::str(qs));
            bc.append(Q(qs));
        }

        for (uint32_t i = 0; i < nData; ++i) {
            auto type = *dp++;
            (void) type;
            if (type == 'e') {
                bc.append({}); // TODO ellipsis
                continue;
            }
            auto sz = varInt();
            auto ptr = skip(sz);
            if (type == 'b') {
                auto p = new (sz) Bytes (ptr, sz);
                debugf("  obj %d = type %c %db @ %p\n", i, type, sz, p);
                bc.append(p);
            } else if (type == 's') {
                auto p = new Str ((char const*) ptr, sz);
                debugf("  obj %d = type %c %db = %s\n",
                        i, type, sz, (char const*) *p);
                bc.append(p);
            } else if (type == 'i') {
                assert(sz < 25);
                char buf [25];
                memcpy(buf, ptr, sz);
                buf[sz] = 0;
                bc.append(Int::conv(buf));
            } else {
                //assert(false); // TODO f)loat and c)omplex
                bc.append(new (sz) Bytes (ptr,sz));
            }
        }

        if (vvec != nullptr) {
            auto dpLen = dp - dpOff;
            auto cEnd = constData.size();
            constData.insert(cEnd, dpLen);
            memcpy(constData.begin() + cEnd, dpOff, dpLen);

            // store known positions of sub-bc's as const ints
            // careful with the fact that new qstrs still get added!
            auto pos = constData.size() - vvec->atLen(0);
            for (uint32_t i = 0; i < nCode; ++i) {
                debugf("  raw %d:\n", i+nData);
                pos += vvec->atLen(0);
                pos = constInsertVaryInt(pos, constNext);
                pos -= vvec->atLen(0);
                bc.append(loadRaw());
            }
        } else
            for (uint32_t i = 0; i < nCode; ++i) {
                debugf("  raw %d:\n", i+nData);
                bc.append(loadRaw());
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
                    debugf("  B: 0x%02x\n", op);
                    break;
                case MP_BC_FORMAT_QSTR: {
                    auto n = storeQstr() + 1;
                    (void) n; assert(n > 0);
                    debugf("  Q: 0x%02x (%d) %s\n", op, n, Q::str(n));
                    break;
                }
                case MP_BC_FORMAT_VAR_UINT: {
                    auto savedDp = dp;
                    auto n = varInt();
                    debugf("  V: 0x%02x %d\n", op, n);
                    (void) n;
                    while (savedDp < dp)
                        *bcNext++ = *savedDp++;
                    break;
                }
                case MP_BC_FORMAT_OFFSET: {
                    uint8_t op1 = *dp++;
                    uint8_t op2 = *dp++;
                    debugf("  O: 0x%02x %04x\n", op, op1 | (op2 << 8));
                    *bcNext++ = op1;
                    *bcNext++ = op2;
                    break;
                }
            }
            if (f != MP_BC_FORMAT_QSTR && (op & MP_BC_MASK_EXTRA_BYTE) == 0) {
                auto n = *dp++;
                *bcNext++ = n;
                debugf("   x 0x%02x\n", n);
            }
        }
    }
};

auto Bytecode::load (void const* p, Value name) -> Callable* {
    Loader loader;
    return loader.load((uint8_t const*) p, name);
}
