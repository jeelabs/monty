// load.cpp - importing and loading bytecodes

#define VERBOSE_LOAD 0 // show .mpy load progress with detailed file info

#include "monty.h"
#include <cassert>

#if VERBOSE_LOAD
#define debugf printf
#else
#define debugf(...)
#endif

namespace Monty {
    constexpr auto qstrFrom = 1; // see qstrNext at the end

    char const qstrData [] =
        //CG< qstr 1
        ""                    "\0" // 1
        "__dir__"             "\0" // 2
        "\x0a"                "\0" // 3
        " "                   "\0" // 4
        "*"                   "\0" // 5
        "/"                   "\0" // 6
        "<module>"            "\0" // 7
        "_"                   "\0" // 8
        "__call__"            "\0" // 9
        "__class__"           "\0" // 10
        "__delitem__"         "\0" // 11
        "__enter__"           "\0" // 12
        "__exit__"            "\0" // 13
        "__getattr__"         "\0" // 14
        "__getitem__"         "\0" // 15
        "__hash__"            "\0" // 16
        "__init__"            "\0" // 17
        "__int__"             "\0" // 18
        "__iter__"            "\0" // 19
        "__len__"             "\0" // 20
        "__main__"            "\0" // 21
        "__module__"          "\0" // 22
        "__name__"            "\0" // 23
        "__new__"             "\0" // 24
        "__next__"            "\0" // 25
        "__qualname__"        "\0" // 26
        "__repr__"            "\0" // 27
        "__setitem__"         "\0" // 28
        "__str__"             "\0" // 29
        "ArithmeticError"     "\0" // 30
        "AssertionError"      "\0" // 31
        "AttributeError"      "\0" // 32
        "BaseException"       "\0" // 33
        "EOFError"            "\0" // 34
        "Ellipsis"            "\0" // 35
        "Exception"           "\0" // 36
        "GeneratorExit"       "\0" // 37
        "ImportError"         "\0" // 38
        "IndentationError"    "\0" // 39
        "IndexError"          "\0" // 40
        "KeyError"            "\0" // 41
        "KeyboardInterrupt"   "\0" // 42
        "LookupError"         "\0" // 43
        "MemoryError"         "\0" // 44
        "NameError"           "\0" // 45
        "NoneType"            "\0" // 46
        "NotImplementedError" "\0" // 47
        "OSError"             "\0" // 48
        "OverflowError"       "\0" // 49
        "RuntimeError"        "\0" // 50
        "StopIteration"       "\0" // 51
        "SyntaxError"         "\0" // 52
        "SystemExit"          "\0" // 53
        "TypeError"           "\0" // 54
        "ValueError"          "\0" // 55
        "ZeroDivisionError"   "\0" // 56
        "abs"                 "\0" // 57
        "all"                 "\0" // 58
        "any"                 "\0" // 59
        "append"              "\0" // 60
        "args"                "\0" // 61
        "bool"                "\0" // 62
        "builtins"            "\0" // 63
        "bytearray"           "\0" // 64
        "bytecode"            "\0" // 65
        "bytes"               "\0" // 66
        "callable"            "\0" // 67
        "chr"                 "\0" // 68
        "classmethod"         "\0" // 69
        "clear"               "\0" // 70
        "close"               "\0" // 71
        "const"               "\0" // 72
        "copy"                "\0" // 73
        "count"               "\0" // 74
        "dict"                "\0" // 75
        "dir"                 "\0" // 76
        "divmod"              "\0" // 77
        "end"                 "\0" // 78
        "endswith"            "\0" // 79
        "eval"                "\0" // 80
        "exec"                "\0" // 81
        "extend"              "\0" // 82
        "find"                "\0" // 83
        "format"              "\0" // 84
        "from_bytes"          "\0" // 85
        "get"                 "\0" // 86
        "getattr"             "\0" // 87
        "globals"             "\0" // 88
        "hasattr"             "\0" // 89
        "hash"                "\0" // 90
        "id"                  "\0" // 91
        "index"               "\0" // 92
        "insert"              "\0" // 93
        "int"                 "\0" // 94
        "isalpha"             "\0" // 95
        "isdigit"             "\0" // 96
        "isinstance"          "\0" // 97
        "islower"             "\0" // 98
        "isspace"             "\0" // 99
        "issubclass"          "\0" // 100
        "isupper"             "\0" // 101
        "items"               "\0" // 102
        "iter"                "\0" // 103
        "join"                "\0" // 104
        "key"                 "\0" // 105
        "keys"                "\0" // 106
        "len"                 "\0" // 107
        "list"                "\0" // 108
        "little"              "\0" // 109
        "locals"              "\0" // 110
        "lower"               "\0" // 111
        "lstrip"              "\0" // 112
        "main"                "\0" // 113
        "map"                 "\0" // 114
        "micropython"         "\0" // 115
        "next"                "\0" // 116
        "object"              "\0" // 117
        "open"                "\0" // 118
        "ord"                 "\0" // 119
        "pop"                 "\0" // 120
        "popitem"             "\0" // 121
        "pow"                 "\0" // 122
        "print"               "\0" // 123
        "range"               "\0" // 124
        "read"                "\0" // 125
        "readinto"            "\0" // 126
        "readline"            "\0" // 127
        "remove"              "\0" // 128
        "replace"             "\0" // 129
        "repr"                "\0" // 130
        "reverse"             "\0" // 131
        "rfind"               "\0" // 132
        "rindex"              "\0" // 133
        "round"               "\0" // 134
        "rsplit"              "\0" // 135
        "rstrip"              "\0" // 136
        "self"                "\0" // 137
        "send"                "\0" // 138
        "sep"                 "\0" // 139
        "set"                 "\0" // 140
        "setattr"             "\0" // 141
        "setdefault"          "\0" // 142
        "sort"                "\0" // 143
        "sorted"              "\0" // 144
        "split"               "\0" // 145
        "start"               "\0" // 146
        "startswith"          "\0" // 147
        "staticmethod"        "\0" // 148
        "step"                "\0" // 149
        "stop"                "\0" // 150
        "str"                 "\0" // 151
        "strip"               "\0" // 152
        "sum"                 "\0" // 153
        "super"               "\0" // 154
        "throw"               "\0" // 155
        "to_bytes"            "\0" // 156
        "tuple"               "\0" // 157
        "type"                "\0" // 158
        "update"              "\0" // 159
        "upper"               "\0" // 160
        "utf-8"               "\0" // 161
        "value"               "\0" // 162
        "values"              "\0" // 163
        "write"               "\0" // 164
        "zip"                 "\0" // 165
        //CG>
    ;

    uint16_t const qstrPos [] = {
        //CG< qstr-emit
           0, // 1
           1, // 2
           9, // 3
          11, // 4
          13, // 5
          15, // 6
          17, // 7
          26, // 8
          28, // 9
          37, // 10
          47, // 11
          59, // 12
          69, // 13
          78, // 14
          90, // 15
         102, // 16
         111, // 17
         120, // 18
         128, // 19
         137, // 20
         145, // 21
         154, // 22
         165, // 23
         174, // 24
         182, // 25
         191, // 26
         204, // 27
         213, // 28
         225, // 29
         233, // 30
         249, // 31
         264, // 32
         279, // 33
         293, // 34
         302, // 35
         311, // 36
         321, // 37
         335, // 38
         347, // 39
         364, // 40
         375, // 41
         384, // 42
         402, // 43
         414, // 44
         426, // 45
         436, // 46
         445, // 47
         465, // 48
         473, // 49
         487, // 50
         500, // 51
         514, // 52
         526, // 53
         537, // 54
         547, // 55
         558, // 56
         576, // 57
         580, // 58
         584, // 59
         588, // 60
         595, // 61
         600, // 62
         605, // 63
         614, // 64
         624, // 65
         633, // 66
         639, // 67
         648, // 68
         652, // 69
         664, // 70
         670, // 71
         676, // 72
         682, // 73
         687, // 74
         693, // 75
         698, // 76
         702, // 77
         709, // 78
         713, // 79
         722, // 80
         727, // 81
         732, // 82
         739, // 83
         744, // 84
         751, // 85
         762, // 86
         766, // 87
         774, // 88
         782, // 89
         790, // 90
         795, // 91
         798, // 92
         804, // 93
         811, // 94
         815, // 95
         823, // 96
         831, // 97
         842, // 98
         850, // 99
         858, // 100
         869, // 101
         877, // 102
         883, // 103
         888, // 104
         893, // 105
         897, // 106
         902, // 107
         906, // 108
         911, // 109
         918, // 110
         925, // 111
         931, // 112
         938, // 113
         943, // 114
         947, // 115
         959, // 116
         964, // 117
         971, // 118
         976, // 119
         980, // 120
         984, // 121
         992, // 122
         996, // 123
        1002, // 124
        1008, // 125
        1013, // 126
        1022, // 127
        1031, // 128
        1038, // 129
        1046, // 130
        1051, // 131
        1059, // 132
        1065, // 133
        1072, // 134
        1078, // 135
        1085, // 136
        1092, // 137
        1097, // 138
        1102, // 139
        1106, // 140
        1110, // 141
        1118, // 142
        1129, // 143
        1134, // 144
        1141, // 145
        1147, // 146
        1153, // 147
        1164, // 148
        1177, // 149
        1182, // 150
        1187, // 151
        1191, // 152
        1197, // 153
        1201, // 154
        1207, // 155
        1213, // 156
        1222, // 157
        1228, // 158
        1233, // 159
        1240, // 160
        1246, // 161
        1252, // 162
        1258, // 163
        1265, // 164
        1271, // 165
        1275, // 166
        //CG>
    };

    constexpr auto qstrNext = qstrFrom + sizeof qstrPos / sizeof *qstrPos - 1;

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
        return new Callable (*mod, bc);
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

        bc.constObjs.insert(0, bc.n_pos + bc.n_kwonly + bc.nData + bc.nCode);

        int ct = 0;
        for (int i = 0; i < bc.n_pos + bc.n_kwonly; ++i)
            bc.constObjs[ct++] = loadQstr();

        for (int i = 0; i < bc.nData; ++i) {
            auto type = *dp++;
            (void) type;
            assert(type != 'e'); // TODO ellipsis
            auto sz = varInt();
            auto ptr = skip(sz);
            if (type == 'b') {
                auto p = new (sz) Bytes (ptr, sz);
                debugf("  obj %d = type %c %db @ %p\n", i, type, (int) sz, p);
                bc.constObjs[ct++] = p;
            } else if (type == 's') {
                auto p = new Str ((char const*) ptr, sz);
                debugf("  obj %d = type %c %db = %s\n",
                        i, type, (int) sz, (char const*) *p);
                bc.constObjs[ct++] = p;
            } else
                assert(false); // TODO
        }
        for (int i = 0; i < bc.nCode; ++i) {
            debugf("  raw %d:\n", i+bc.nData);
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
                while (n-- > 0)
                    *bcNext++ = *dp++;
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

auto Monty::loadModule (char const* name, uint8_t const* addr) -> Callable* {
    Loader loader;
    auto* init = loader.load(addr);
    if (init == nullptr)
        return nullptr;

    init->mo.at("__name__") = name;
    return init;
}
