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
    { Q (198,"machine"), ext_machine },
#endif
#if STM32
    { Q (208,"machine"), ext_machine },
#endif
//CG>
};

static Lookup const mod_attrs (mod_map, sizeof mod_map);
Dict Module::loaded (&mod_attrs);

extern char const monty::qstrBase [] =
//CG< qstr-emit
#if NATIVE
    "\x90\x01\x56\x02\x57\x02\x5F\x02\x61\x02\x63\x02\x65\x02\x67\x02"
    "\x70\x02\x72\x02\x7B\x02\x85\x02\x91\x02\x9B\x02\xA4\x02\xB0\x02"
    "\xBC\x02\xC5\x02\xCE\x02\xD6\x02\xDF\x02\xE7\x02\xF0\x02\xFB\x02"
    "\x04\x03\x0C\x03\x15\x03\x22\x03\x2B\x03\x37\x03\x3F\x03\x4F\x03"
    "\x5E\x03\x6D\x03\x7B\x03\x84\x03\x8D\x03\x97\x03\xA5\x03\xB1\x03"
    "\xC2\x03\xCD\x03\xD6\x03\xE8\x03\xF4\x03\x00\x04\x0A\x04\x13\x04"
    "\x27\x04\x2F\x04\x3D\x04\x4A\x04\x58\x04\x64\x04\x6F\x04\x79\x04"
    "\x84\x04\x96\x04\x9A\x04\x9E\x04\xA2\x04\xA9\x04\xAE\x04\xB3\x04"
    "\xBC\x04\xC6\x04\xCF\x04\xD5\x04\xDE\x04\xE2\x04\xEE\x04\xF4\x04"
    "\xFA\x04\x00\x05\x05\x05\x0B\x05\x10\x05\x14\x05\x1B\x05\x1F\x05"
    "\x28\x05\x2D\x05\x32\x05\x39\x05\x3E\x05\x45\x05\x50\x05\x54\x05"
    "\x5C\x05\x64\x05\x6C\x05\x71\x05\x74\x05\x7A\x05\x81\x05\x85\x05"
    "\x8D\x05\x95\x05\xA0\x05\xA8\x05\xB0\x05\xBB\x05\xC3\x05\xC9\x05"
    "\xCE\x05\xD3\x05\xD7\x05\xDC\x05\xE0\x05\xE5\x05\xEC\x05\xF3\x05"
    "\xF9\x05\x00\x06\x05\x06\x09\x06\x15\x06\x1A\x06\x21\x06\x26\x06"
    "\x2A\x06\x2E\x06\x36\x06\x3A\x06\x40\x06\x46\x06\x4B\x06\x54\x06"
    "\x5D\x06\x64\x06\x6C\x06\x71\x06\x79\x06\x7F\x06\x86\x06\x8C\x06"
    "\x93\x06\x9A\x06\x9F\x06\xA4\x06\xA8\x06\xAC\x06\xB4\x06\xBF\x06"
    "\xC4\x06\xCB\x06\xD1\x06\xD7\x06\xE2\x06\xEF\x06\xF4\x06\xF9\x06"
    "\xFD\x06\x03\x07\x07\x07\x0D\x07\x13\x07\x1C\x07\x22\x07\x27\x07"
    "\x2E\x07\x34\x07\x3A\x07\x40\x07\x47\x07\x4D\x07\x51\x07\x5E\x07"
    "\x69\x07\x74\x07\x7B\x07\x81\x07\x84\x07\x8A\x07\x92\x07\x96\x07"
    "\x9C\x07\xA4\x07\xB3\x07\xB9\x07\xC1\x07\xC6\x07\xCC\x07\xD2\x07"
    "\xD8\x07\xE4\x07\xED\x07\xF4\x07\xFE\x07\x09\x08\x15\x08\x20\x08"
    "\x2B\x08\x34\x08\x3D\x08\x44\x08\x4F\x08\x56\x08\x5C\x08\x64\x08"
    // offsets [0..399], hashes [400..597], 198 strings [598..2147]
#endif
#if STM32
    "\xAC\x01\x80\x02\x81\x02\x89\x02\x8B\x02\x8D\x02\x8F\x02\x91\x02"
    "\x9A\x02\x9C\x02\xA5\x02\xAF\x02\xBB\x02\xC5\x02\xCE\x02\xDA\x02"
    "\xE6\x02\xEF\x02\xF8\x02\x00\x03\x09\x03\x11\x03\x1A\x03\x25\x03"
    "\x2E\x03\x36\x03\x3F\x03\x4C\x03\x55\x03\x61\x03\x69\x03\x79\x03"
    "\x88\x03\x97\x03\xA5\x03\xAE\x03\xB7\x03\xC1\x03\xCF\x03\xDB\x03"
    "\xEC\x03\xF7\x03\x00\x04\x12\x04\x1E\x04\x2A\x04\x34\x04\x3D\x04"
    "\x51\x04\x59\x04\x67\x04\x74\x04\x82\x04\x8E\x04\x99\x04\xA3\x04"
    "\xAE\x04\xC0\x04\xC4\x04\xC8\x04\xCC\x04\xD3\x04\xD8\x04\xDD\x04"
    "\xE6\x04\xF0\x04\xF9\x04\xFF\x04\x08\x05\x0C\x05\x18\x05\x1E\x05"
    "\x24\x05\x2A\x05\x2F\x05\x35\x05\x3A\x05\x3E\x05\x45\x05\x49\x05"
    "\x52\x05\x57\x05\x5C\x05\x63\x05\x68\x05\x6F\x05\x7A\x05\x7E\x05"
    "\x86\x05\x8E\x05\x96\x05\x9B\x05\x9E\x05\xA4\x05\xAB\x05\xAF\x05"
    "\xB7\x05\xBF\x05\xCA\x05\xD2\x05\xDA\x05\xE5\x05\xED\x05\xF3\x05"
    "\xF8\x05\xFD\x05\x01\x06\x06\x06\x0A\x06\x0F\x06\x16\x06\x1D\x06"
    "\x23\x06\x2A\x06\x2F\x06\x33\x06\x3F\x06\x44\x06\x4B\x06\x50\x06"
    "\x54\x06\x58\x06\x60\x06\x64\x06\x6A\x06\x70\x06\x75\x06\x7E\x06"
    "\x87\x06\x8E\x06\x96\x06\x9B\x06\xA3\x06\xA9\x06\xB0\x06\xB6\x06"
    "\xBD\x06\xC4\x06\xC9\x06\xCE\x06\xD2\x06\xD6\x06\xDE\x06\xE9\x06"
    "\xEE\x06\xF5\x06\xFB\x06\x01\x07\x0C\x07\x19\x07\x1E\x07\x23\x07"
    "\x27\x07\x2D\x07\x31\x07\x37\x07\x3D\x07\x46\x07\x4C\x07\x51\x07"
    "\x58\x07\x5E\x07\x64\x07\x6A\x07\x71\x07\x77\x07\x7B\x07\x88\x07"
    "\x93\x07\x9E\x07\xA5\x07\xAB\x07\xAE\x07\xB4\x07\xBC\x07\xC0\x07"
    "\xC6\x07\xCE\x07\xDD\x07\xE3\x07\xEB\x07\xF0\x07\xF6\x07\xFC\x07"
    "\x02\x08\x0E\x08\x17\x08\x1E\x08\x28\x08\x33\x08\x3F\x08\x4A\x08"
    "\x55\x08\x5E\x08\x67\x08\x6E\x08\x79\x08\x81\x08\x88\x08\x8D\x08"
    "\x92\x08\x98\x08\x9D\x08\xA1\x08\xA6\x08\xAB\x08\xAF\x08\xB6\x08"
    "\xBC\x08\xC4\x08\xCB\x08\xD1\x08\xD8\x08\xDD\x08"
    // offsets [0..427], hashes [428..639], 212 strings [640..2268]
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
    "\xB7\x4E\x7D\x98\xE6\x22\x40\x8F\x15\xC9\x61\xD5\xE0\xBC\x5B\xEC"
    "\x17\x64\xBF\x8E\x7C\xAB\xB5\xC1\xA7\xE1\x56\xB0\xB0\x25\x05\x45"
    "\xF8\x4D\xF4"
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
    "UnicodeError"         "\0" // 166
    "<bytecode>"           "\0" // 167
    "<callable>"           "\0" // 168
    "<pyvm>"               "\0" // 169
    "event"                "\0" // 170
    "gc"                   "\0" // 171
    "gcmax"                "\0" // 172
    "gcstats"              "\0" // 173
    "sys"                  "\0" // 174
    "tasks"                "\0" // 175
    "modules"              "\0" // 176
    "implementation"       "\0" // 177
    "monty"                "\0" // 178
    "version"              "\0" // 179
    "wait"                 "\0" // 180
    "array"                "\0" // 181
    "class"                "\0" // 182
    "slice"                "\0" // 183
    "<boundmeth>"          "\0" // 184
    "<buffer>"             "\0" // 185
    "<cell>"               "\0" // 186
    "<closure>"            "\0" // 187
    "<dictview>"           "\0" // 188
    "<exception>"          "\0" // 189
    "<function>"           "\0" // 190
    "<iterator>"           "\0" // 191
    "<lookup>"             "\0" // 192
    "<method>"             "\0" // 193
    "<none>"               "\0" // 194
    "<stacklet>"           "\0" // 195
#if NATIVE
    "ticker"               "\0" // 196
    "ticks"                "\0" // 197
    "machine"              "\0" // 198
#endif
#if STM32
    "disable"              "\0" // 196
    "enable"               "\0" // 197
    "xfer"                 "\0" // 198
    "recv"                 "\0" // 199
    "sleep"                "\0" // 200
    "xmit"                 "\0" // 201
    "dog"                  "\0" // 202
    "kick"                 "\0" // 203
    "rf69"                 "\0" // 204
    "spi"                  "\0" // 205
    "ticker"               "\0" // 206
    "ticks"                "\0" // 207
    "machine"              "\0" // 208
    "<pins>"               "\0" // 209
    "<spi>"                "\0" // 210
    "<rf69>"               "\0" // 211
    "pins"                 "\0" // 212
#endif
//CG>
;

int const monty::qstrBaseLen = sizeof qstrBase;
