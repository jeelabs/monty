// import.cpp - importing, loading, and bytecode objects

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "xmonty.h"

using namespace Monty;

struct Monty::Bytecode : Object {
    static Type const info;
    auto type () const -> Type const& override;

    Bytecode () {}

    void marker () const override {} // TODO

    uint16_t code;
    VecOf<Value> constObjsVec;
    ChunkOf<Value> constObjs {constObjsVec};
    int16_t stackSz;
    int16_t flags;
    int8_t excDepth;
    int8_t n_pos;
    int8_t n_kwonly;
    int8_t n_def_pos;
    int16_t hdrSz;
    int16_t size;
    int16_t nData;
    int16_t nCode;

};

Type const Bytecode::info ("<bytecode>");
auto Bytecode::type () const -> Type const& { return info; }

auto Callable::frameSize () const -> size_t {
    return 100; // TODO
}

void Callable::marker () const {
    callee.marker();
}

Module::Module (Value v) {
    auto& bc = v.asType<Bytecode>();

    (void) bc; // TODO
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if NATIVE
#define VERBOSE_LOAD 1
#endif

namespace Monty {

#include "qstr.h"

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

#if VERBOSE_LOAD // see main.cpp
#include <cstdio>
#define loaderf printf
#else
#define loaderf(...)
#endif

struct Loader {
    const QstrPool* qPool;
    const uint8_t* dp;
    VecOf<char> qBufVec;
    ChunkOf<char> qBuf {qBufVec};
    VecOf<Value> qVecRaw;
    ChunkOf<Value> qVec {qVecRaw};
    uint8_t* bcBuf;
    uint8_t* bcNext;
    uint8_t* bcLimit;
    VecOf<uint16_t> qWinVec;
    ChunkOf<uint16_t> qWin {qWinVec};

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
            loaderf("n_state        %d\n", n_state);
            loaderf("n_exc_stack    %d\n", n_exc_stack);
            loaderf("scope_flags    %d\n", scope_flags);
            loaderf("n_pos_args     %d\n", n_pos_args);
            loaderf("n_kwonly_args  %d\n", n_kwonly_args);
            loaderf("n_def_pos_args %d\n", n_def_pos_args);

            n_info = 0;
            n_cell = 0;
            z = 0x80;
            for (int n = 0; z & 0x80; ++n) {
                z = *l.dp++; /* xIIIIIIC */
                n_cell |= (z & 1) << n;
                n_info |= ((z & 0x7e) >> 1) << (6 * n);
            }
            loaderf("n_info         %d\n", n_info);
            loaderf("n_cell         %d\n", n_cell);
            return n_info + n_cell;
        }
    } prelude;

    Module* load (const uint8_t* data, int index =0) {
        dp  = data;
        if (*dp++ != 'M')
            return 0; // incorrect file format

        loaderf("version %d\n", dp[0]);
        loaderf("features 0x%02x\n", dp[1]);
        loaderf("intbits %d\n", dp[2]);
        dp += 3;
        int n = varInt();
        loaderf("qwin %d\n", (int) n);
        qWin.insert(0, n); // qstr window

        qBuf.insert(0, 500); // TODO create space to avoid constant resizing
        qBuf.remove(0, 500);

        auto mo = new Module (loadRaw());

        loaderf("qUsed #%d %db\n", (int) qVec.length(), (int) qBuf.length());
        qPool = QstrPool::create((const char*) qBufVec.ptr(), qVec.length(), qBuf.length());

        qBuf.remove(0, qBuf.length()); // buffer no longer needed
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
        auto o = qBuf.length();
        qBuf.insert(o, len + 1);
        auto s = (char*) qBufVec.ptr() + o; // TODO careful, can move
        for (int i = 0; i < len; ++i)
            s[i] = *dp++;
        s[len] = 0;
        loaderf("q:%s\n", s);
        int n = qVec.length();
        qVec[n] = s;
        qWin.remove(qWin.length()-1);
        qWin.insert(0);
        n += qstrNext;
        qWin[0] = n;
        return n;
    }

    int winQstr (int i) {
        int n = qWin[i];
        qWin.remove(i);
        qWin.insert(0);
        qWin[0] = n;
        return n;
    }

    int storeQstr () {
        auto n = loadQstr();
        assert(n > 0);
        *bcNext++ = --n;
        *bcNext++ = n >> 8;
        return n;
    }

    const Bytecode& loadRaw () {
        auto typsiz = varInt();
        auto bCount = typsiz >> 2;
        loaderf("type %d size %d (%d)\n", typsiz & 3, bCount, typsiz);

        auto savedDp = dp;
        auto nskip = prelude.load(*this);
        auto npre = dp - savedDp;

        // bytecode will be stored in extra bytes allocated after Bytecode
        auto& bc = *new (bCount + 1) Bytecode; // FIXME +1 for stm32 ???
        bcBuf = bcNext = (uint8_t*) (&bc + 1);
        bcLimit = bcBuf + bCount - npre;

        bc.stackSz = prelude.n_state;
        bc.excDepth = prelude.n_exc_stack;
        bc.flags = prelude.scope_flags;
        bc.n_pos = prelude.n_pos_args;
        bc.n_kwonly = prelude.n_kwonly_args;
        bc.n_def_pos = prelude.n_def_pos_args;
        bc.hdrSz = prelude.n_info + prelude.n_cell;
        bc.size = bCount;
        loaderf("raw sc %d np %d hs %d sz %d ns %d nx %d ko %d dp %d\n",
                bc.flags, bc.n_pos, bc.hdrSz, bc.size,
                bc.stackSz, bc.excDepth, bc.n_kwonly, bc.n_def_pos);

        auto n1 = storeQstr();
        auto n2 = storeQstr();
        (void) n1; (void) n2;
        loaderf("qstr %d %d npre %d nskip %d\n", n1+1, n2+1, (int) npre, nskip);

        for (int i = 4; i < nskip; ++i)
            *bcNext++ = *dp++;

        bc.code = bcNext - bcBuf;

        loadOps();

        loaderf("subs %08x\n", *(const uint32_t*) dp);
        //loaderf("jump %08x\n", *(const uint32_t*) bc.code);

        bc.nData = varInt();
        bc.nCode = varInt();
        loaderf("nData %d nCode %d\n", bc.nData, bc.nCode);

        int ct = 0;
        for (int i = 0; i < bc.n_pos + bc.n_kwonly; ++i)
            bc.constObjs[ct++] = loadQstr();

        for (int i = 0; i < bc.nData; ++i) {
            auto type = *dp++;
            (void) type;
            assert(type != 'e'); // TODO ellipsis
            auto sz = varInt();
            auto ptr = skip(sz);
#if 1 //TODO
            (void) ptr;
#else
            if (type == 'b') {
                auto p = new (sz) BytesObj (ptr, sz);
                loaderf("  obj %d = type %c %db @ %p\n", i, type, (int) sz, p);
                bc.constObjs.set(ct++, p);
            } else if (type == 's') {
                auto buf = (char*) malloc(sz+1);
                memcpy(buf, ptr, sz);
                buf[sz] = 0;
                loaderf("  obj %d = type %c %db = %s\n", i, type, (int) sz, buf);
                bc.constObjs.set(ct++, buf);
            } else
#endif
                assert(false); // TODO
        }
        for (int i = 0; i < bc.nCode; ++i) {
            loaderf("  raw %d:\n", i+bc.nData);
            bc.constObjs[ct++] = loadRaw();
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
                    loaderf("  B: 0x%02x\n", op);
                    break;
                case MP_BC_FORMAT_QSTR: {
                    auto n = storeQstr() + 1;
                    assert(n > 0);
                    auto s = n < (int) qstrNext ? qstrData + qstrPos[n-1] :
                                        (const char*) qVec[n-qstrNext];
                    (void) s;
                    loaderf("  Q: 0x%02x (%d) %s\n", op, (int) n, s);
                    break;
                }
                case MP_BC_FORMAT_VAR_UINT: {
                    auto savedDp = dp;
                    auto n = varInt();
                    loaderf("  V: 0x%02x %d\n", op, n);
                    (void) n;
                    while (savedDp < dp)
                        *bcNext++ = *savedDp++;
                    break;
                }
                case MP_BC_FORMAT_OFFSET: {
                    uint8_t op1 = *dp++;
                    uint8_t op2 = *dp++;
                    loaderf("  O: 0x%02x %04x\n", op, op1 | (op2 << 8));
                    *bcNext++ = op1;
                    *bcNext++ = op2;
                    break;
                }
            }
            if (f != MP_BC_FORMAT_QSTR && (op & MP_BC_MASK_EXTRA_BYTE) == 0) {
                auto n = *dp++;
                *bcNext++ = n;
                loaderf("   x 0x%02x\n", n);
                while (n-- > 0)
                    *bcNext++ = *dp++;
            }
        }
    }
};

#undef loaderf // !VERBOSE_LOAD

} // namespace Monty
