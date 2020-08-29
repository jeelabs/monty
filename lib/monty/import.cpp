// import.cpp - importing and loading bytecodes

#define VERBOSE_LOAD 0 // show .mpy load progress with detailed file info

#include "monty.h"
#include <cassert>

#if VERBOSE_LOAD
#define debugf printf
#else
#define debugf(...)
#endif

namespace Monty {

struct Loader {
    const uint8_t* dp;
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

        auto& bc = loadRaw();

        debugf("qLast %d %s\n", Q::last(), Q::str(Q::last()));

        auto mod = new Module (&builtins);
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

        char qBuf [30]; // TODO yuck, temp buf to terminate with a null byte
        assert(len < sizeof qBuf);
        for (int i = 0; i < len; ++i)
            qBuf[i] = *dp++;
        qBuf[len] = 0;
        debugf("q:%s\n", qBuf);

        int n = Q::make(qBuf);

        qWin.remove(qWin.fill-1);
        qWin.insert(0);
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
            } else if (type == 'i') {
                assert(sz < 25);
                char buf [25];
                memcpy(buf, ptr, sz);
                buf[sz] = 0;
                int64_t v = strtoll(buf, nullptr, 10);
                bc.append(Int::make(v));
            } else
                assert(false); // TODO e, f, c
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
                    (void) n; assert(n > 0);
                    debugf("  Q: 0x%02x (%d) %s\n", op, (int) n, Q::str(n));
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

uint8_t const* Monty::fsBase;

auto Monty::fsLookup (char const* name) -> uint8_t const* {
    auto addr = fsBase;

    if (addr != nullptr)
        while (true) {
            if (memcmp(addr, "mty0", 4) != 0) {
                printf("%s: not found\n", name);
                return nullptr;
            }
            if (strcmp(name, (char const*) addr + 28) == 0)
                break;
            auto size = ((uint32_t const*) addr)[1];
            while (size % 8 != 4)
                ++size;
            addr += size + 4;
        }

    // found MRFS entry, the actual payload starts a little further
    auto vend = *(uint16_t*) (addr + 14);   // end of varyvec
    vend += 8;                              // skip header
    vend += -vend & 7;                      // round up to multiple of 8
    //printf("loading 20%s @ %p\n", (char const*) addr + 16, addr + vend);
    return addr + vend;                     // start of real payload
}

auto Monty::loader (Value name, uint8_t const* addr) -> Callable* {
    Loader ldr;
    auto* init = ldr.load(addr);
    if (init != nullptr)
        init->mo.at(Q( 23,"__name__")) = name;
    return init;
}
