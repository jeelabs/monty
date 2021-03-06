// qstr.cpp - set of "quick strings" from MicroPython 1.14 plus own entries

#include "monty.h"

using namespace monty;

//CG2 mod-list d
extern Module ext_machine;
extern Module ext_sys;

static Lookup::Item const mod_map [] = {
//CG< mod-list a
    { Q (174,"sys"), ext_sys },
#if NATIVE
    { Q (206,"machine"), ext_machine },
#endif
#if STM32
    { Q (216,"machine"), ext_machine },
#endif
//CG>
};

static Lookup const mod_attrs (mod_map);
Dict Module::loaded (&mod_attrs);

extern char const monty::qstrBase [] =
//CG< qstr-emit
#if NATIVE
    "\xA0\x01\x6E\x02\x6F\x02\x77\x02\x79\x02\x7B\x02\x7D\x02\x7F\x02"
    "\x88\x02\x8A\x02\x93\x02\x9D\x02\xA9\x02\xB3\x02\xBC\x02\xC8\x02"
    "\xD4\x02\xDD\x02\xE6\x02\xEE\x02\xF7\x02\xFF\x02\x08\x03\x13\x03"
    "\x1C\x03\x24\x03\x2D\x03\x3A\x03\x43\x03\x4F\x03\x57\x03\x67\x03"
    "\x76\x03\x85\x03\x93\x03\x9C\x03\xA5\x03\xAF\x03\xBD\x03\xC9\x03"
    "\xDA\x03\xE5\x03\xEE\x03\x00\x04\x0C\x04\x18\x04\x22\x04\x2B\x04"
    "\x3F\x04\x47\x04\x55\x04\x62\x04\x70\x04\x7C\x04\x87\x04\x91\x04"
    "\x9C\x04\xAE\x04\xB2\x04\xB6\x04\xBA\x04\xC1\x04\xC6\x04\xCB\x04"
    "\xD4\x04\xDE\x04\xE7\x04\xED\x04\xF6\x04\xFA\x04\x06\x05\x0C\x05"
    "\x12\x05\x18\x05\x1D\x05\x23\x05\x28\x05\x2C\x05\x33\x05\x37\x05"
    "\x40\x05\x45\x05\x4A\x05\x51\x05\x56\x05\x5D\x05\x68\x05\x6C\x05"
    "\x74\x05\x7C\x05\x84\x05\x89\x05\x8C\x05\x92\x05\x99\x05\x9D\x05"
    "\xA5\x05\xAD\x05\xB8\x05\xC0\x05\xC8\x05\xD3\x05\xDB\x05\xE1\x05"
    "\xE6\x05\xEB\x05\xEF\x05\xF4\x05\xF8\x05\xFD\x05\x04\x06\x0B\x06"
    "\x11\x06\x18\x06\x1D\x06\x21\x06\x2D\x06\x32\x06\x39\x06\x3E\x06"
    "\x42\x06\x46\x06\x4E\x06\x52\x06\x58\x06\x5E\x06\x63\x06\x6C\x06"
    "\x75\x06\x7C\x06\x84\x06\x89\x06\x91\x06\x97\x06\x9E\x06\xA4\x06"
    "\xAB\x06\xB2\x06\xB7\x06\xBC\x06\xC0\x06\xC4\x06\xCC\x06\xD7\x06"
    "\xDC\x06\xE3\x06\xE9\x06\xEF\x06\xFA\x06\x07\x07\x0C\x07\x11\x07"
    "\x15\x07\x1B\x07\x1F\x07\x25\x07\x2B\x07\x34\x07\x3A\x07\x3F\x07"
    "\x46\x07\x4C\x07\x52\x07\x58\x07\x5F\x07\x65\x07\x69\x07\x72\x07"
    "\x7F\x07\x8A\x07\x95\x07\x9F\x07\xA2\x07\xA8\x07\xB0\x07\xB4\x07"
    "\xBA\x07\xC2\x07\xD1\x07\xD7\x07\xDF\x07\xEA\x07\xF5\x07\xFC\x07"
    "\x08\x08\x12\x08\x19\x08\x1D\x08\x21\x08\x25\x08\x2A\x08\x30\x08"
    "\x36\x08\x3C\x08\x42\x08\x48\x08\x51\x08\x5D\x08\x68\x08\x73\x08"
    "\x7C\x08\x85\x08\x8C\x08\x97\x08\x9F\x08\xA6\x08\xAC\x08\xB4\x08"
    // offsets [0..415], hashes [416..621], 206 strings [622..2227]
#endif
#if STM32
    "\xBC\x01\x98\x02\x99\x02\xA1\x02\xA3\x02\xA5\x02\xA7\x02\xA9\x02"
    "\xB2\x02\xB4\x02\xBD\x02\xC7\x02\xD3\x02\xDD\x02\xE6\x02\xF2\x02"
    "\xFE\x02\x07\x03\x10\x03\x18\x03\x21\x03\x29\x03\x32\x03\x3D\x03"
    "\x46\x03\x4E\x03\x57\x03\x64\x03\x6D\x03\x79\x03\x81\x03\x91\x03"
    "\xA0\x03\xAF\x03\xBD\x03\xC6\x03\xCF\x03\xD9\x03\xE7\x03\xF3\x03"
    "\x04\x04\x0F\x04\x18\x04\x2A\x04\x36\x04\x42\x04\x4C\x04\x55\x04"
    "\x69\x04\x71\x04\x7F\x04\x8C\x04\x9A\x04\xA6\x04\xB1\x04\xBB\x04"
    "\xC6\x04\xD8\x04\xDC\x04\xE0\x04\xE4\x04\xEB\x04\xF0\x04\xF5\x04"
    "\xFE\x04\x08\x05\x11\x05\x17\x05\x20\x05\x24\x05\x30\x05\x36\x05"
    "\x3C\x05\x42\x05\x47\x05\x4D\x05\x52\x05\x56\x05\x5D\x05\x61\x05"
    "\x6A\x05\x6F\x05\x74\x05\x7B\x05\x80\x05\x87\x05\x92\x05\x96\x05"
    "\x9E\x05\xA6\x05\xAE\x05\xB3\x05\xB6\x05\xBC\x05\xC3\x05\xC7\x05"
    "\xCF\x05\xD7\x05\xE2\x05\xEA\x05\xF2\x05\xFD\x05\x05\x06\x0B\x06"
    "\x10\x06\x15\x06\x19\x06\x1E\x06\x22\x06\x27\x06\x2E\x06\x35\x06"
    "\x3B\x06\x42\x06\x47\x06\x4B\x06\x57\x06\x5C\x06\x63\x06\x68\x06"
    "\x6C\x06\x70\x06\x78\x06\x7C\x06\x82\x06\x88\x06\x8D\x06\x96\x06"
    "\x9F\x06\xA6\x06\xAE\x06\xB3\x06\xBB\x06\xC1\x06\xC8\x06\xCE\x06"
    "\xD5\x06\xDC\x06\xE1\x06\xE6\x06\xEA\x06\xEE\x06\xF6\x06\x01\x07"
    "\x06\x07\x0D\x07\x13\x07\x19\x07\x24\x07\x31\x07\x36\x07\x3B\x07"
    "\x3F\x07\x45\x07\x49\x07\x4F\x07\x55\x07\x5E\x07\x64\x07\x69\x07"
    "\x70\x07\x76\x07\x7C\x07\x82\x07\x89\x07\x8F\x07\x93\x07\x9C\x07"
    "\xA9\x07\xB4\x07\xBF\x07\xC9\x07\xCC\x07\xD2\x07\xDA\x07\xDE\x07"
    "\xE4\x07\xEC\x07\xFB\x07\x01\x08\x09\x08\x14\x08\x1F\x08\x26\x08"
    "\x32\x08\x3C\x08\x43\x08\x47\x08\x4B\x08\x4F\x08\x54\x08\x5A\x08"
    "\x60\x08\x66\x08\x6C\x08\x72\x08\x7B\x08\x87\x08\x92\x08\x9D\x08"
    "\xA6\x08\xAF\x08\xB6\x08\xC1\x08\xC9\x08\xD1\x08\xD8\x08\xDD\x08"
    "\xE2\x08\xE8\x08\xED\x08\xF1\x08\xF6\x08\xFB\x08\xFF\x08\x06\x09"
    "\x0C\x09\x14\x09\x1B\x09\x21\x09\x28\x09\x2D\x09"
    // offsets [0..443], hashes [444..663], 220 strings [664..2348]
#endif
    "\x05\x7A\xB0\x85\x8F\x8A\xBD\xFA\xA7\x2B\xFD\x6D\x45\x40\x26\xF7"
    "\x5F\x16\xCF\xE2\x8E\xFF\xE2\x79\x02\x6B\x10\x32\xD0\x2D\x97\x21"
    "\x07\x91\xF0\xF2\x16\x20\x5C\x83\xEA\xAF\xFF\xDC\xBA\x17\xC6\xA1"
    "\x81\x61\xEA\x94\x20\x25\x96\xB6\x95\x44\x13\x6B\xC2\xEB\xF7\x76"
    "\x22\x5C\x0D\xDC\xB4\x7C\x33\xC0\xE0\xA6\x3F\xFA\xB8\x0A\x1B\x9B"
    "\x1E\x63\x00\x26\x35\x33\xC0\x9D\x8C\xB7\x28\x7B\x12\x16\xEB\xA8"
    "\xB6\xFC\x5B\xB5\xDD\xE3\x8F\xA7\x32\x01\x62\x27\x89\x3B\xC6\xE5"
    "\xCE\xB9\x0B\x42\x90\xD1\x1C\x2A\xBF\x2D\x54\x1A\xB7\x4B\xF9\x63"
    "\x49\xD0\x25\xD2\xE9\xE7\xA5\x3B\x79\xB9\x23\x27\xD4\x6C\xBF\x5E"
    "\xB7\x85\x74\x62\x57\x9D\x50\x29\x2E\xC4\xB3\xD8\xFD\x9D\xB4\x27"
    "\xB7\x4E\x7D\x98\xE6\xB2\x22\x2E\xB0\x03\x61\xD5\xE0\xBC\xEE\xEC"
    "\x17\x64\xBF\x40\x8F\xE1\xC1\x56\x15\xA3\xF4\xFC\x8E\xA4\x7C\xAB"
    "\xC9\xB5\xA7\xB0\x25\x05\x45\xF8\x4D\xF4\xA7"
#if NATIVE
    "\x87\x43\x60"
#endif
#if STM32
    "\x91\x04\xAC\xE7\xEA\xED\x29\x4F\x9E\xCF\x87\x43\x60\x43\x0D\x7C"
    "\x41"
#endif
    // end of 1-byte hashes, start of string data:
    ""                     "\0" // 1
    "__dir__"              "\0" // 2
    "\x0a"                 "\0" // 3
    " "                    "\0" // 4
    "*"                    "\0" // 5
    "/"                    "\0" // 6
    "<module>"             "\0" // 7
    "_"                    "\0" // 8
    "__call__"             "\0" // 9
    "__class__"            "\0" // 10
    "__delitem__"          "\0" // 11
    "__enter__"            "\0" // 12
    "__exit__"             "\0" // 13
    "__getattr__"          "\0" // 14
    "__getitem__"          "\0" // 15
    "__hash__"             "\0" // 16
    "__init__"             "\0" // 17
    "__int__"              "\0" // 18
    "__iter__"             "\0" // 19
    "__len__"              "\0" // 20
    "__main__"             "\0" // 21
    "__module__"           "\0" // 22
    "__name__"             "\0" // 23
    "__new__"              "\0" // 24
    "__next__"             "\0" // 25
    "__qualname__"         "\0" // 26
    "__repr__"             "\0" // 27
    "__setitem__"          "\0" // 28
    "__str__"              "\0" // 29
    "ArithmeticError"      "\0" // 30
    "AssertionError"       "\0" // 31
    "AttributeError"       "\0" // 32
    "BaseException"        "\0" // 33
    "EOFError"             "\0" // 34
    "Ellipsis"             "\0" // 35
    "Exception"            "\0" // 36
    "GeneratorExit"        "\0" // 37
    "ImportError"          "\0" // 38
    "IndentationError"     "\0" // 39
    "IndexError"           "\0" // 40
    "KeyError"             "\0" // 41
    "KeyboardInterrupt"    "\0" // 42
    "LookupError"          "\0" // 43
    "MemoryError"          "\0" // 44
    "NameError"            "\0" // 45
    "NoneType"             "\0" // 46
    "NotImplementedError"  "\0" // 47
    "OSError"              "\0" // 48
    "OverflowError"        "\0" // 49
    "RuntimeError"         "\0" // 50
    "StopIteration"        "\0" // 51
    "SyntaxError"          "\0" // 52
    "SystemExit"           "\0" // 53
    "TypeError"            "\0" // 54
    "ValueError"           "\0" // 55
    "ZeroDivisionError"    "\0" // 56
    "abs"                  "\0" // 57
    "all"                  "\0" // 58
    "any"                  "\0" // 59
    "append"               "\0" // 60
    "args"                 "\0" // 61
    "bool"                 "\0" // 62
    "builtins"             "\0" // 63
    "bytearray"            "\0" // 64
    "bytecode"             "\0" // 65
    "bytes"                "\0" // 66
    "callable"             "\0" // 67
    "chr"                  "\0" // 68
    "classmethod"          "\0" // 69
    "clear"                "\0" // 70
    "close"                "\0" // 71
    "const"                "\0" // 72
    "copy"                 "\0" // 73
    "count"                "\0" // 74
    "dict"                 "\0" // 75
    "dir"                  "\0" // 76
    "divmod"               "\0" // 77
    "end"                  "\0" // 78
    "endswith"             "\0" // 79
    "eval"                 "\0" // 80
    "exec"                 "\0" // 81
    "extend"               "\0" // 82
    "find"                 "\0" // 83
    "format"               "\0" // 84
    "from_bytes"           "\0" // 85
    "get"                  "\0" // 86
    "getattr"              "\0" // 87
    "globals"              "\0" // 88
    "hasattr"              "\0" // 89
    "hash"                 "\0" // 90
    "id"                   "\0" // 91
    "index"                "\0" // 92
    "insert"               "\0" // 93
    "int"                  "\0" // 94
    "isalpha"              "\0" // 95
    "isdigit"              "\0" // 96
    "isinstance"           "\0" // 97
    "islower"              "\0" // 98
    "isspace"              "\0" // 99
    "issubclass"           "\0" // 100
    "isupper"              "\0" // 101
    "items"                "\0" // 102
    "iter"                 "\0" // 103
    "join"                 "\0" // 104
    "key"                  "\0" // 105
    "keys"                 "\0" // 106
    "len"                  "\0" // 107
    "list"                 "\0" // 108
    "little"               "\0" // 109
    "locals"               "\0" // 110
    "lower"                "\0" // 111
    "lstrip"               "\0" // 112
    "main"                 "\0" // 113
    "map"                  "\0" // 114
    "micropython"          "\0" // 115
    "next"                 "\0" // 116
    "object"               "\0" // 117
    "open"                 "\0" // 118
    "ord"                  "\0" // 119
    "pop"                  "\0" // 120
    "popitem"              "\0" // 121
    "pow"                  "\0" // 122
    "print"                "\0" // 123
    "range"                "\0" // 124
    "read"                 "\0" // 125
    "readinto"             "\0" // 126
    "readline"             "\0" // 127
    "remove"               "\0" // 128
    "replace"              "\0" // 129
    "repr"                 "\0" // 130
    "reverse"              "\0" // 131
    "rfind"                "\0" // 132
    "rindex"               "\0" // 133
    "round"                "\0" // 134
    "rsplit"               "\0" // 135
    "rstrip"               "\0" // 136
    "self"                 "\0" // 137
    "send"                 "\0" // 138
    "sep"                  "\0" // 139
    "set"                  "\0" // 140
    "setattr"              "\0" // 141
    "setdefault"           "\0" // 142
    "sort"                 "\0" // 143
    "sorted"               "\0" // 144
    "split"                "\0" // 145
    "start"                "\0" // 146
    "startswith"           "\0" // 147
    "staticmethod"         "\0" // 148
    "step"                 "\0" // 149
    "stop"                 "\0" // 150
    "str"                  "\0" // 151
    "strip"                "\0" // 152
    "sum"                  "\0" // 153
    "super"                "\0" // 154
    "throw"                "\0" // 155
    "to_bytes"             "\0" // 156
    "tuple"                "\0" // 157
    "type"                 "\0" // 158
    "update"               "\0" // 159
    "upper"                "\0" // 160
    "utf-8"                "\0" // 161
    "value"                "\0" // 162
    "values"               "\0" // 163
    "write"                "\0" // 164
    "zip"                  "\0" // 165
    "<object>"             "\0" // 166
    "UnicodeError"         "\0" // 167
    "<instance>"           "\0" // 168
    "<dictview>"           "\0" // 169
    "__bases__"            "\0" // 170
    "gc"                   "\0" // 171
    "gcmax"                "\0" // 172
    "gcstats"              "\0" // 173
    "sys"                  "\0" // 174
    "ready"                "\0" // 175
    "modules"              "\0" // 176
    "implementation"       "\0" // 177
    "monty"                "\0" // 178
    "version"              "\0" // 179
    "<bytecode>"           "\0" // 180
    "<callable>"           "\0" // 181
    "<cell>"               "\0" // 182
    "<boundmeth>"          "\0" // 183
    "<closure>"            "\0" // 184
    "<pyvm>"               "\0" // 185
    "foo"                  "\0" // 186
    "bar"                  "\0" // 187
    "baz"                  "\0" // 188
    "wait"                 "\0" // 189
    "trace"                "\0" // 190
    "array"                "\0" // 191
    "class"                "\0" // 192
    "event"                "\0" // 193
    "slice"                "\0" // 194
    "<buffer>"             "\0" // 195
    "<exception>"          "\0" // 196
    "<function>"           "\0" // 197
    "<iterator>"           "\0" // 198
    "<lookup>"             "\0" // 199
    "<method>"             "\0" // 200
    "<none>"               "\0" // 201
    "<stacklet>"           "\0" // 202
    "argtest"              "\0" // 203
#if NATIVE
    "ticker"               "\0" // 204
    "ticks"                "\0" // 205
    "machine"              "\0" // 206
#endif
#if STM32
    "disable"              "\0" // 204
    "enable"               "\0" // 205
    "xfer"                 "\0" // 206
    "recv"                 "\0" // 207
    "sleep"                "\0" // 208
    "xmit"                 "\0" // 209
    "dog"                  "\0" // 210
    "kick"                 "\0" // 211
    "rf69"                 "\0" // 212
    "spi"                  "\0" // 213
    "ticker"               "\0" // 214
    "ticks"                "\0" // 215
    "machine"              "\0" // 216
    "<pins>"               "\0" // 217
    "<spi>"                "\0" // 218
    "<rf69>"               "\0" // 219
    "pins"                 "\0" // 220
#endif
//CG>
;

int const monty::qstrBaseLen = sizeof qstrBase;
