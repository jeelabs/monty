// import.cpp - importing and loading bytecodes

#define VERBOSE_LOAD 0 // show .mpy load progress with detailed file info

#include "monty.h"
#include "qstr.h"
#include <cassert>

#if VERBOSE_LOAD
#define debugf printf
#else
#define debugf(...)
#endif

namespace Monty {

struct QstrPool : Object {
    int len;
    uint16_t off [];

    static auto create (char const* d, int n, int b) -> QstrPool const* {
        static_assert (sizeof *off == 2, "off is not a uint16_t ?");
        return new (2*(n+1)+b) QstrPool (d, n, b);
    }

    QstrPool (char const* data, int num, int bytes) : len (num) {
        auto vec = (char*) off;
        off[0] = 2*(num+1);
        memcpy(vec + off[0], data, bytes);
        for (int i = 0; i < num; ++i) {
            auto pos = off[i];
            off[i+1] = pos + strlen(vec + pos) + 1;
        }
    }

    auto atIdx (int idx) const -> char const* {
        assert(idx >= qstrFrom);
        if (idx < (int) qstrNext)
            return qstrData + qstrPos[idx-qstrFrom];
        idx -= qstrNext;
        assert(idx < len);
        return (char const*) off + off[idx];
    }
};

struct Loader {
    const uint8_t* dp;
    VecOf<char> qBuf;
    Vector qVec;
    VecOf<uint16_t> qWin;
    uint8_t* bcBuf;
    uint8_t* bcNext;
    uint8_t* bcLimit;

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
            debugf("n_state        %d\n", n_state);
            debugf("n_exc_stack    %d\n", n_exc_stack);
            debugf("scope_flags    %d\n", scope_flags);
            debugf("n_pos_args     %d\n", n_pos_args);
            debugf("n_kwonly_args  %d\n", n_kwonly_args);
            debugf("n_def_pos_args %d\n", n_def_pos_args);

            n_info = 0;
            n_cell = 0;
            z = 0x80;
            for (int n = 0; z & 0x80; ++n) {
                z = *l.dp++; /* xIIIIIIC */
                n_cell |= (z & 1) << n;
                n_info |= ((z & 0x7e) >> 1) << (6 * n);
            }
            debugf("n_info         %d\n", n_info);
            debugf("n_cell         %d\n", n_cell);
            return n_info + n_cell;
        }
    } prelude;

    Callable* load (const uint8_t* data) {
        dp  = data;
        if (*dp++ != 'M')
            return 0; // incorrect file format

        debugf("version %d\n", dp[0]);
        debugf("features 0x%02x\n", dp[1]);
        debugf("intbits %d\n", dp[2]);
        dp += 3;
        int n = varInt();
        debugf("qwin %d\n", (int) n);
        qWin.insert(0, n); // qstr window

        qBuf.adj(500); // TODO avoid large over-alloc

        auto& bc = loadRaw();

        debugf("qUsed #%d %db\n", (int) qVec.fill, (int) qBuf.fill);
        auto pool = QstrPool::create((char const*) qBuf.begin(),
                                        qVec.fill, qBuf.fill);
        assert(pool != nullptr);

        qBuf.adj(0); // buffer no longer needed

        auto mod = new Module (&builtins, *pool);
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
        auto o = qBuf.fill;
        qBuf.insert(o, len + 1);
        auto s = (char*) qBuf.begin() + o; // TODO careful, can move
        for (int i = 0; i < len; ++i)
            s[i] = *dp++;
        s[len] = 0;
        debugf("q:%s\n", s);
        int n = qVec.fill;
        qVec.insert(n, 1); // make room
        qVec[n] = s;
        qWin.remove(qWin.fill-1);
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
        debugf("type %d size %d (%d)\n", typsiz & 3, bCount, typsiz);

        auto savedDp = dp;
        auto nskip = prelude.load(*this);
        auto npre = dp - savedDp;

        // bytecode will be stored in extra bytes allocated after Bytecode
        auto& bc = *new (bCount) Bytecode;
        bcBuf = bcNext = (uint8_t*) (&bc + 1);
        bcLimit = bcBuf + bCount - npre;

        bc.stackSz = prelude.n_state;
        bc.excDepth = prelude.n_exc_stack;
        bc.flags = prelude.scope_flags;
        bc.n_pos = prelude.n_pos_args;
        bc.n_kwonly = prelude.n_kwonly_args;
        bc.n_def_pos = prelude.n_def_pos_args;
        bc.hdrSz = prelude.n_info + prelude.n_cell;
        bc.n_cell = prelude.n_cell;
        bc.size = bCount;
        debugf("raw sc %d np %d hs %d sz %d ns %d nx %d ko %d dp %d\n",
                bc.flags, bc.n_pos, bc.hdrSz, bc.size,
                bc.stackSz, bc.excDepth, bc.n_kwonly, bc.n_def_pos);

        auto n1 = storeQstr();
        auto n2 = storeQstr();
        (void) n1; (void) n2;
        debugf("qstr %d %d npre %d nskip %d\n", n1+1, n2+1, (int) npre, nskip);

        for (int i = 4; i < nskip; ++i)
            *bcNext++ = *dp++;

        bc.code = bcNext - bcBuf;

        loadOps();

        debugf("subs %08x\n", *(const uint32_t*) dp);
        //debugf("jump %08x\n", *(const uint32_t*) bc.code);

        bc.nData = varInt();
        bc.nCode = varInt();
        debugf("nData %d nCode %d\n", bc.nData, bc.nCode);

        bc.adj(bc.n_pos + bc.n_kwonly + bc.nData + bc.nCode); // pre-alloc

        for (int i = 0; i < bc.n_pos + bc.n_kwonly; ++i)
            bc.append(loadQstr());

        for (int i = 0; i < bc.nData; ++i) {
            auto type = *dp++;
            (void) type;
            assert(type != 'e'); // TODO ellipsis
            auto sz = varInt();
            auto ptr = skip(sz);
            if (type == 'b') {
                auto p = new (sz) Bytes (ptr, sz);
                debugf("  obj %d = type %c %db @ %p\n", i, type, (int) sz, p);
                bc.append(p);
            } else if (type == 's') {
                auto p = new Str ((char const*) ptr, sz);
                debugf("  obj %d = type %c %db = %s\n",
                        i, type, (int) sz, (char const*) *p);
                bc.append(p);
            } else
                assert(false); // TODO
        }
        for (int i = 0; i < bc.nCode; ++i) {
            debugf("  raw %d:\n", i+bc.nData);
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
                    assert(n > 0);
                    auto s = n < (int) qstrNext ? qstrData + qstrPos[n-1] :
                                        (char const*) qVec[n-qstrNext];
                    (void) s;
                    debugf("  Q: 0x%02x (%d) %s\n", op, (int) n, s);
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

} // namespace Monty

using namespace Monty;

// TODO placed here iso in call.cpp because it needs to know the QstrPool type
auto Callable::qStrAt (size_t i) const -> char const* {
    return mo.qp.asType<QstrPool>().atIdx(i);
}

auto Monty::loader (char const* name, uint8_t const* addr) -> Callable* {
    Loader ldr;
    auto* init = ldr.load(addr);
    if (init != nullptr)
        init->mo.at(Q( 23,"__name__")) = name;
    return init;
}
