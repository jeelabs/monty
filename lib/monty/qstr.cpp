// qstr.cpp - set of "quick strings" from MicroPython 1.13 plus own entries

#include "monty.h"

using namespace monty;

//CG< mod-list d
extern Module ext_sys;
#if NATIVE
extern Module ext_machine;
#endif
#if STM32
extern Module ext_machine;
#endif
//CG>

static Lookup::Item const mod_map [] = {
    //CG< mod-list a
    { Q(176,"sys"), ext_sys },
    #if NATIVE
    { Q(201,"machine"), ext_machine },
    #endif
    #if STM32
    { Q(201,"machine"), ext_machine },
    #endif
    //CG>
};

static Lookup const mod_attrs (mod_map, sizeof mod_map);
Dict Module::loaded (&mod_attrs);

extern char const monty::qstrBase [] =
    //CG< qstr-emit v
    "\xB2\x01\x89\x02\x8A\x02\x92\x02\x94\x02\x96\x02\x98\x02\x9A\x02"
    "\xA3\x02\xA5\x02\xAE\x02\xB8\x02\xC4\x02\xCE\x02\xD7\x02\xE3\x02"
    "\xEF\x02\xF8\x02\x01\x03\x09\x03\x12\x03\x1A\x03\x23\x03\x2E\x03"
    "\x37\x03\x3F\x03\x48\x03\x55\x03\x5E\x03\x6A\x03\x72\x03\x82\x03"
    "\x91\x03\xA0\x03\xAE\x03\xB7\x03\xC0\x03\xCA\x03\xD8\x03\xE4\x03"
    "\xF5\x03\x00\x04\x09\x04\x1B\x04\x27\x04\x33\x04\x3D\x04\x46\x04"
    "\x5A\x04\x62\x04\x70\x04\x7D\x04\x8B\x04\x97\x04\xA2\x04\xAC\x04"
    "\xB7\x04\xC9\x04\xCD\x04\xD1\x04\xD5\x04\xDC\x04\xE1\x04\xE6\x04"
    "\xEF\x04\xF9\x04\x02\x05\x08\x05\x11\x05\x15\x05\x21\x05\x27\x05"
    "\x2D\x05\x33\x05\x38\x05\x3E\x05\x43\x05\x47\x05\x4E\x05\x52\x05"
    "\x5B\x05\x60\x05\x65\x05\x6C\x05\x71\x05\x78\x05\x83\x05\x87\x05"
    "\x8F\x05\x97\x05\x9F\x05\xA4\x05\xA7\x05\xAD\x05\xB4\x05\xB8\x05"
    "\xC0\x05\xC8\x05\xD3\x05\xDB\x05\xE3\x05\xEE\x05\xF6\x05\xFC\x05"
    "\x01\x06\x06\x06\x0A\x06\x0F\x06\x13\x06\x18\x06\x1F\x06\x26\x06"
    "\x2C\x06\x33\x06\x38\x06\x3C\x06\x48\x06\x4D\x06\x54\x06\x59\x06"
    "\x5D\x06\x61\x06\x69\x06\x6D\x06\x73\x06\x79\x06\x7E\x06\x87\x06"
    "\x90\x06\x97\x06\x9F\x06\xA4\x06\xAC\x06\xB2\x06\xB9\x06\xBF\x06"
    "\xC6\x06\xCD\x06\xD2\x06\xD7\x06\xDB\x06\xDF\x06\xE7\x06\xF2\x06"
    "\xF7\x06\xFE\x06\x04\x07\x0A\x07\x15\x07\x22\x07\x27\x07\x2C\x07"
    "\x30\x07\x36\x07\x3A\x07\x40\x07\x46\x07\x4F\x07\x55\x07\x5A\x07"
    "\x61\x07\x67\x07\x6D\x07\x73\x07\x7A\x07\x80\x07\x84\x07\x8D\x07"
    "\x93\x07\x96\x07\x9C\x07\xA4\x07\xAA\x07\xB2\x07\xC1\x07\xC7\x07"
    "\xCF\x07\xD3\x07\xE0\x07\xEB\x07\xF6\x07\xFD\x07\x08\x08\x12\x08"
    "\x17\x08\x1D\x08\x23\x08\x29\x08\x35\x08\x3E\x08\x45\x08\x4F\x08"
    "\x5A\x08\x66\x08\x71\x08\x7C\x08\x85\x08\x8E\x08\x95\x08\xA0\x08"
    "\xA7\x08\xAD\x08\xB5\x08\xBD\x08\xC4\x08\xC9\x08\xCE\x08\xD4\x08"
    "\xD9\x08\xDD\x08\xE2\x08\xE7\x08\xEB\x08\xF2\x08\xF8\x08\xFF\x08"
    "\x04\x09"
    // index [0..433], hashes [434..648], 215 strings [649..2307]
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
    "\xB7\x4E\x7D\x98\xE6\xB2\xC9\x61\xD5\xE0\x5B\xEC\x17\x64\xBF\xBC"
    "\x22\x40\x8F\x15\x2E\x03\x8E\x7C\xAB\xB5\xC1\xA7\xE1\x56\xB0\xB0"
    "\x25\x05\x45\xF8\x4D\xF4\x87\x43\x60\x91\x04\xAC\xE7\xEA\xED\x29"
    "\x4F\x9E\xCF\x43\x0D\x7C\x41"
    // found 150 distinct hashes
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
    "event"                "\0" // 167
    "gc"                   "\0" // 168
    "gcmax"                "\0" // 169
    "gcstats"              "\0" // 170
    "tasks"                "\0" // 171
    "modules"              "\0" // 172
    "implementation"       "\0" // 173
    "monty"                "\0" // 174
    "version"              "\0" // 175
    "sys"                  "\0" // 176
    "UnicodeError"         "\0" // 177
    "<bytecode>"           "\0" // 178
    "<callable>"           "\0" // 179
    "<pyvm>"               "\0" // 180
    "<instance>"           "\0" // 181
    "__bases__"            "\0" // 182
    "wait"                 "\0" // 183
    "array"                "\0" // 184
    "class"                "\0" // 185
    "slice"                "\0" // 186
    "<boundmeth>"          "\0" // 187
    "<buffer>"             "\0" // 188
    "<cell>"               "\0" // 189
    "<closure>"            "\0" // 190
    "<dictview>"           "\0" // 191
    "<exception>"          "\0" // 192
    "<function>"           "\0" // 193
    "<iterator>"           "\0" // 194
    "<lookup>"             "\0" // 195
    "<method>"             "\0" // 196
    "<none>"               "\0" // 197
    "<stacklet>"           "\0" // 198
    "ticker"               "\0" // 199
    "ticks"                "\0" // 200
    "machine"              "\0" // 201
    "disable"              "\0" // 202
    "enable"               "\0" // 203
    "xfer"                 "\0" // 204
    "recv"                 "\0" // 205
    "sleep"                "\0" // 206
    "xmit"                 "\0" // 207
    "dog"                  "\0" // 208
    "kick"                 "\0" // 209
    "rf69"                 "\0" // 210
    "spi"                  "\0" // 211
    "<pins>"               "\0" // 212
    "<spi>"                "\0" // 213
    "<rf69>"               "\0" // 214
    "pins"                 "\0" // 215
    //CG>
;

int const monty::qstrBaseLen = sizeof qstrBase;
