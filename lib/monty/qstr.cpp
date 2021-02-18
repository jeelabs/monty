// qstr.cpp - set of "quick strings" from MicroPython 1.13 plus own entries

#include "monty.h"

namespace monty {

    extern char const qstrBase [] =
        //CG< qstr-emit v
        "\x92\x01\x59\x02\x5A\x02\x62\x02\x64\x02\x66\x02\x68\x02\x6A\x02"
        "\x73\x02\x75\x02\x7E\x02\x88\x02\x94\x02\x9E\x02\xA7\x02\xB3\x02"
        "\xBF\x02\xC8\x02\xD1\x02\xD9\x02\xE2\x02\xEA\x02\xF3\x02\xFE\x02"
        "\x07\x03\x0F\x03\x18\x03\x25\x03\x2E\x03\x3A\x03\x42\x03\x52\x03"
        "\x61\x03\x70\x03\x7E\x03\x87\x03\x90\x03\x9A\x03\xA8\x03\xB4\x03"
        "\xC5\x03\xD0\x03\xD9\x03\xEB\x03\xF7\x03\x03\x04\x0D\x04\x16\x04"
        "\x2A\x04\x32\x04\x40\x04\x4D\x04\x5B\x04\x67\x04\x72\x04\x7C\x04"
        "\x87\x04\x99\x04\x9D\x04\xA1\x04\xA5\x04\xAC\x04\xB1\x04\xB6\x04"
        "\xBF\x04\xC9\x04\xD2\x04\xD8\x04\xE1\x04\xE5\x04\xF1\x04\xF7\x04"
        "\xFD\x04\x03\x05\x08\x05\x0E\x05\x13\x05\x17\x05\x1E\x05\x22\x05"
        "\x2B\x05\x30\x05\x35\x05\x3C\x05\x41\x05\x48\x05\x53\x05\x57\x05"
        "\x5F\x05\x67\x05\x6F\x05\x74\x05\x77\x05\x7D\x05\x84\x05\x88\x05"
        "\x90\x05\x98\x05\xA3\x05\xAB\x05\xB3\x05\xBE\x05\xC6\x05\xCC\x05"
        "\xD1\x05\xD6\x05\xDA\x05\xDF\x05\xE3\x05\xE8\x05\xEF\x05\xF6\x05"
        "\xFC\x05\x03\x06\x08\x06\x0C\x06\x18\x06\x1D\x06\x24\x06\x29\x06"
        "\x2D\x06\x31\x06\x39\x06\x3D\x06\x43\x06\x49\x06\x4E\x06\x57\x06"
        "\x60\x06\x67\x06\x6F\x06\x74\x06\x7C\x06\x82\x06\x89\x06\x8F\x06"
        "\x96\x06\x9D\x06\xA2\x06\xA7\x06\xAB\x06\xAF\x06\xB7\x06\xC2\x06"
        "\xC7\x06\xCE\x06\xD4\x06\xDA\x06\xE5\x06\xF2\x06\xF7\x06\xFC\x06"
        "\x00\x07\x06\x07\x0A\x07\x10\x07\x16\x07\x1F\x07\x25\x07\x2A\x07"
        "\x31\x07\x37\x07\x3D\x07\x43\x07\x4A\x07\x50\x07\x54\x07\x5A\x07"
        "\x62\x07\x68\x07\x6B\x07\x71\x07\x79\x07\x88\x07\x8E\x07\x96\x07"
        "\xA3\x07\xAE\x07\xB9\x07\xC0\x07\xCA\x07\xD0\x07\xD6\x07\xDC\x07"
        "\xE8\x07\xF1\x07\xF8\x07\x02\x08\x0D\x08\x19\x08\x24\x08\x2F\x08"
        "\x38\x08\x41\x08\x48\x08\x53\x08\x5C\x08\x67\x08\x6C\x08\x70\x08"
        "\x78\x08"
        // index [0..401], hashes [402..600], 199 strings [601..2167]
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
        "\xB7\x4E\x7D\x98\xE6\x5B\xEC\xC9\x61\xD5\xE0\x17\x64\xBF\x22\x40"
        "\x8F\x15\x03\x7C\xAB\xB5\xC1\xA7\xE1\x56\xB0\xB0\x25\x05\x45\xF8"
        "\x4D\xF4\xB2\x2E\x8E\xBC\x60"
        // found 142 distinct hashes
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
        "tasks"                "\0" // 166
        "modules"              "\0" // 167
        "event"                "\0" // 168
        "gc"                   "\0" // 169
        "gcmax"                "\0" // 170
        "gcstats"              "\0" // 171
        "implementation"       "\0" // 172
        "monty"                "\0" // 173
        "version"              "\0" // 174
        "UnicodeError"         "\0" // 175
        "<bytecode>"           "\0" // 176
        "<callable>"           "\0" // 177
        "<pyvm>"               "\0" // 178
        "__bases__"            "\0" // 179
        "array"                "\0" // 180
        "class"                "\0" // 181
        "slice"                "\0" // 182
        "<boundmeth>"          "\0" // 183
        "<buffer>"             "\0" // 184
        "<cell>"               "\0" // 185
        "<closure>"            "\0" // 186
        "<dictview>"           "\0" // 187
        "<exception>"          "\0" // 188
        "<function>"           "\0" // 189
        "<iterator>"           "\0" // 190
        "<lookup>"             "\0" // 191
        "<method>"             "\0" // 192
        "<none>"               "\0" // 193
        "<stacklet>"           "\0" // 194
        "<object>"             "\0" // 195
        "<instance>"           "\0" // 196
        "wait"                 "\0" // 197
        "sys"                  "\0" // 198
        "machine"              "\0" // 199
        //CG>
    ;

    extern int const qstrBaseLen = sizeof qstrBase;

} // namespace monty
