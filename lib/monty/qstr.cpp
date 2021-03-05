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
    { Q (202,"machine"), ext_machine },
#endif
#if STM32
    { Q (212,"machine"), ext_machine },
#endif
//CG>
};

static Lookup const mod_attrs (mod_map);
Dict Module::loaded (&mod_attrs);

extern char const monty::qstrBase [] =
//CG< qstr-emit
#if NATIVE
    "\x98\x01\x62\x02\x63\x02\x6B\x02\x6D\x02\x6F\x02\x71\x02\x73\x02"
    "\x7C\x02\x7E\x02\x87\x02\x91\x02\x9D\x02\xA7\x02\xB0\x02\xBC\x02"
    "\xC8\x02\xD1\x02\xDA\x02\xE2\x02\xEB\x02\xF3\x02\xFC\x02\x07\x03"
    "\x10\x03\x18\x03\x21\x03\x2E\x03\x37\x03\x43\x03\x4B\x03\x5B\x03"
    "\x6A\x03\x79\x03\x87\x03\x90\x03\x99\x03\xA3\x03\xB1\x03\xBD\x03"
    "\xCE\x03\xD9\x03\xE2\x03\xF4\x03\x00\x04\x0C\x04\x16\x04\x1F\x04"
    "\x33\x04\x3B\x04\x49\x04\x56\x04\x64\x04\x70\x04\x7B\x04\x85\x04"
    "\x90\x04\xA2\x04\xA6\x04\xAA\x04\xAE\x04\xB5\x04\xBA\x04\xBF\x04"
    "\xC8\x04\xD2\x04\xDB\x04\xE1\x04\xEA\x04\xEE\x04\xFA\x04\x00\x05"
    "\x06\x05\x0C\x05\x11\x05\x17\x05\x1C\x05\x20\x05\x27\x05\x2B\x05"
    "\x34\x05\x39\x05\x3E\x05\x45\x05\x4A\x05\x51\x05\x5C\x05\x60\x05"
    "\x68\x05\x70\x05\x78\x05\x7D\x05\x80\x05\x86\x05\x8D\x05\x91\x05"
    "\x99\x05\xA1\x05\xAC\x05\xB4\x05\xBC\x05\xC7\x05\xCF\x05\xD5\x05"
    "\xDA\x05\xDF\x05\xE3\x05\xE8\x05\xEC\x05\xF1\x05\xF8\x05\xFF\x05"
    "\x05\x06\x0C\x06\x11\x06\x15\x06\x21\x06\x26\x06\x2D\x06\x32\x06"
    "\x36\x06\x3A\x06\x42\x06\x46\x06\x4C\x06\x52\x06\x57\x06\x60\x06"
    "\x69\x06\x70\x06\x78\x06\x7D\x06\x85\x06\x8B\x06\x92\x06\x98\x06"
    "\x9F\x06\xA6\x06\xAB\x06\xB0\x06\xB4\x06\xB8\x06\xC0\x06\xCB\x06"
    "\xD0\x06\xD7\x06\xDD\x06\xE3\x06\xEE\x06\xFB\x06\x00\x07\x05\x07"
    "\x09\x07\x0F\x07\x13\x07\x19\x07\x1F\x07\x28\x07\x2E\x07\x33\x07"
    "\x3A\x07\x40\x07\x46\x07\x4C\x07\x53\x07\x59\x07\x5D\x07\x66\x07"
    "\x73\x07\x7E\x07\x89\x07\x93\x07\x96\x07\x9C\x07\xA4\x07\xA8\x07"
    "\xAE\x07\xB6\x07\xC5\x07\xCB\x07\xD3\x07\xDE\x07\xE9\x07\xF0\x07"
    "\xFC\x07\x06\x08\x0D\x08\x12\x08\x18\x08\x1E\x08\x24\x08\x2A\x08"
    "\x30\x08\x39\x08\x45\x08\x50\x08\x5B\x08\x64\x08\x6D\x08\x74\x08"
    "\x7F\x08\x86\x08\x8C\x08\x94\x08"
    // offsets [0..407], hashes [408..609], 202 strings [610..2195]
#endif
#if STM32
    "\xB4\x01\x8C\x02\x8D\x02\x95\x02\x97\x02\x99\x02\x9B\x02\x9D\x02"
    "\xA6\x02\xA8\x02\xB1\x02\xBB\x02\xC7\x02\xD1\x02\xDA\x02\xE6\x02"
    "\xF2\x02\xFB\x02\x04\x03\x0C\x03\x15\x03\x1D\x03\x26\x03\x31\x03"
    "\x3A\x03\x42\x03\x4B\x03\x58\x03\x61\x03\x6D\x03\x75\x03\x85\x03"
    "\x94\x03\xA3\x03\xB1\x03\xBA\x03\xC3\x03\xCD\x03\xDB\x03\xE7\x03"
    "\xF8\x03\x03\x04\x0C\x04\x1E\x04\x2A\x04\x36\x04\x40\x04\x49\x04"
    "\x5D\x04\x65\x04\x73\x04\x80\x04\x8E\x04\x9A\x04\xA5\x04\xAF\x04"
    "\xBA\x04\xCC\x04\xD0\x04\xD4\x04\xD8\x04\xDF\x04\xE4\x04\xE9\x04"
    "\xF2\x04\xFC\x04\x05\x05\x0B\x05\x14\x05\x18\x05\x24\x05\x2A\x05"
    "\x30\x05\x36\x05\x3B\x05\x41\x05\x46\x05\x4A\x05\x51\x05\x55\x05"
    "\x5E\x05\x63\x05\x68\x05\x6F\x05\x74\x05\x7B\x05\x86\x05\x8A\x05"
    "\x92\x05\x9A\x05\xA2\x05\xA7\x05\xAA\x05\xB0\x05\xB7\x05\xBB\x05"
    "\xC3\x05\xCB\x05\xD6\x05\xDE\x05\xE6\x05\xF1\x05\xF9\x05\xFF\x05"
    "\x04\x06\x09\x06\x0D\x06\x12\x06\x16\x06\x1B\x06\x22\x06\x29\x06"
    "\x2F\x06\x36\x06\x3B\x06\x3F\x06\x4B\x06\x50\x06\x57\x06\x5C\x06"
    "\x60\x06\x64\x06\x6C\x06\x70\x06\x76\x06\x7C\x06\x81\x06\x8A\x06"
    "\x93\x06\x9A\x06\xA2\x06\xA7\x06\xAF\x06\xB5\x06\xBC\x06\xC2\x06"
    "\xC9\x06\xD0\x06\xD5\x06\xDA\x06\xDE\x06\xE2\x06\xEA\x06\xF5\x06"
    "\xFA\x06\x01\x07\x07\x07\x0D\x07\x18\x07\x25\x07\x2A\x07\x2F\x07"
    "\x33\x07\x39\x07\x3D\x07\x43\x07\x49\x07\x52\x07\x58\x07\x5D\x07"
    "\x64\x07\x6A\x07\x70\x07\x76\x07\x7D\x07\x83\x07\x87\x07\x90\x07"
    "\x9D\x07\xA8\x07\xB3\x07\xBD\x07\xC0\x07\xC6\x07\xCE\x07\xD2\x07"
    "\xD8\x07\xE0\x07\xEF\x07\xF5\x07\xFD\x07\x08\x08\x13\x08\x1A\x08"
    "\x26\x08\x30\x08\x37\x08\x3C\x08\x42\x08\x48\x08\x4E\x08\x54\x08"
    "\x5A\x08\x63\x08\x6F\x08\x7A\x08\x85\x08\x8E\x08\x97\x08\x9E\x08"
    "\xA9\x08\xB1\x08\xB8\x08\xBD\x08\xC2\x08\xC8\x08\xCD\x08\xD1\x08"
    "\xD6\x08\xDB\x08\xDF\x08\xE6\x08\xEC\x08\xF4\x08\xFB\x08\x01\x09"
    "\x08\x09\x0D\x09"
    // offsets [0..435], hashes [436..651], 216 strings [652..2316]
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
    "\x17\x64\xBF\x40\x8F\xE1\xC1\x56\x15\x8E\xA4\x7C\xAB\xC9\xB5\xA7"
    "\xB0\x25\x05\x45\xF8\x4D\xF4"
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
    "wait"                 "\0" // 186
    "trace"                "\0" // 187
    "array"                "\0" // 188
    "class"                "\0" // 189
    "event"                "\0" // 190
    "slice"                "\0" // 191
    "<buffer>"             "\0" // 192
    "<exception>"          "\0" // 193
    "<function>"           "\0" // 194
    "<iterator>"           "\0" // 195
    "<lookup>"             "\0" // 196
    "<method>"             "\0" // 197
    "<none>"               "\0" // 198
    "<stacklet>"           "\0" // 199
#if NATIVE
    "ticker"               "\0" // 200
    "ticks"                "\0" // 201
    "machine"              "\0" // 202
#endif
#if STM32
    "disable"              "\0" // 200
    "enable"               "\0" // 201
    "xfer"                 "\0" // 202
    "recv"                 "\0" // 203
    "sleep"                "\0" // 204
    "xmit"                 "\0" // 205
    "dog"                  "\0" // 206
    "kick"                 "\0" // 207
    "rf69"                 "\0" // 208
    "spi"                  "\0" // 209
    "ticker"               "\0" // 210
    "ticks"                "\0" // 211
    "machine"              "\0" // 212
    "<pins>"               "\0" // 213
    "<spi>"                "\0" // 214
    "<rf69>"               "\0" // 215
    "pins"                 "\0" // 216
#endif
//CG>
;

int const monty::qstrBaseLen = sizeof qstrBase;
