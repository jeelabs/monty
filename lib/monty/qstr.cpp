// qstr.cpp - set of "quick strings" from MicroPython 1.13 plus own entries

#include "monty.h"

namespace monty {

    extern char const qstrBase [] =
        //CG< qstr-emit v
        "\xB0\x01\x86\x02\x87\x02\x8F\x02\x91\x02\x93\x02\x95\x02\x97\x02"
        "\xA0\x02\xA2\x02\xAB\x02\xB5\x02\xC1\x02\xCB\x02\xD4\x02\xE0\x02"
        "\xEC\x02\xF5\x02\xFE\x02\x06\x03\x0F\x03\x17\x03\x20\x03\x2B\x03"
        "\x34\x03\x3C\x03\x45\x03\x52\x03\x5B\x03\x67\x03\x6F\x03\x7F\x03"
        "\x8E\x03\x9D\x03\xAB\x03\xB4\x03\xBD\x03\xC7\x03\xD5\x03\xE1\x03"
        "\xF2\x03\xFD\x03\x06\x04\x18\x04\x24\x04\x30\x04\x3A\x04\x43\x04"
        "\x57\x04\x5F\x04\x6D\x04\x7A\x04\x88\x04\x94\x04\x9F\x04\xA9\x04"
        "\xB4\x04\xC6\x04\xCA\x04\xCE\x04\xD2\x04\xD9\x04\xDE\x04\xE3\x04"
        "\xEC\x04\xF6\x04\xFF\x04\x05\x05\x0E\x05\x12\x05\x1E\x05\x24\x05"
        "\x2A\x05\x30\x05\x35\x05\x3B\x05\x40\x05\x44\x05\x4B\x05\x4F\x05"
        "\x58\x05\x5D\x05\x62\x05\x69\x05\x6E\x05\x75\x05\x80\x05\x84\x05"
        "\x8C\x05\x94\x05\x9C\x05\xA1\x05\xA4\x05\xAA\x05\xB1\x05\xB5\x05"
        "\xBD\x05\xC5\x05\xD0\x05\xD8\x05\xE0\x05\xEB\x05\xF3\x05\xF9\x05"
        "\xFE\x05\x03\x06\x07\x06\x0C\x06\x10\x06\x15\x06\x1C\x06\x23\x06"
        "\x29\x06\x30\x06\x35\x06\x39\x06\x45\x06\x4A\x06\x51\x06\x56\x06"
        "\x5A\x06\x5E\x06\x66\x06\x6A\x06\x70\x06\x76\x06\x7B\x06\x84\x06"
        "\x8D\x06\x94\x06\x9C\x06\xA1\x06\xA9\x06\xAF\x06\xB6\x06\xBC\x06"
        "\xC3\x06\xCA\x06\xCF\x06\xD4\x06\xD8\x06\xDC\x06\xE4\x06\xEF\x06"
        "\xF4\x06\xFB\x06\x01\x07\x07\x07\x12\x07\x1F\x07\x24\x07\x29\x07"
        "\x2D\x07\x33\x07\x37\x07\x3D\x07\x43\x07\x4C\x07\x52\x07\x57\x07"
        "\x5E\x07\x64\x07\x6A\x07\x70\x07\x77\x07\x7D\x07\x81\x07\x8A\x07"
        "\x90\x07\x93\x07\x99\x07\xA1\x07\xA7\x07\xAF\x07\xBE\x07\xC4\x07"
        "\xCC\x07\xD9\x07\xE4\x07\xEF\x07\xF6\x07\x01\x08\x0B\x08\x10\x08"
        "\x16\x08\x1C\x08\x22\x08\x2E\x08\x37\x08\x3E\x08\x48\x08\x53\x08"
        "\x5F\x08\x6A\x08\x75\x08\x7E\x08\x87\x08\x8E\x08\x99\x08\x9D\x08"
        "\xA5\x08\xAC\x08\xB2\x08\xC1\x08\xC8\x08\xD0\x08\xD9\x08\xE7\x08"
        "\xEF\x08\xF5\x08\x04\x09\x09\x09\x0D\x09\x12\x09\x16\x09\x1B\x09"
        // index [0..431], hashes [432..645], 214 strings [646..2330]
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
        "\xB7\x4E\x7D\x98\xE6\xB2\xC9\x61\xD5\xE0\x5B\xEC\x17\x64\xBF\x22"
        "\x40\x8F\x15\x2E\x03\x8E\x7C\xAB\xB5\xC1\xA7\xE1\x56\xB0\xB0\x25"
        "\x05\x45\xF8\x4D\xF4\xBC\x60\x87\x43\x68\x04\x91\xEE\x46\x4E\xEA"
        "\xD7\x41\xCF\x9E\x29\x4F"
        // found 152 distinct hashes
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
        "UnicodeError"         "\0" // 176
        "<bytecode>"           "\0" // 177
        "<callable>"           "\0" // 178
        "<pyvm>"               "\0" // 179
        "<instance>"           "\0" // 180
        "__bases__"            "\0" // 181
        "wait"                 "\0" // 182
        "array"                "\0" // 183
        "class"                "\0" // 184
        "slice"                "\0" // 185
        "<boundmeth>"          "\0" // 186
        "<buffer>"             "\0" // 187
        "<cell>"               "\0" // 188
        "<closure>"            "\0" // 189
        "<dictview>"           "\0" // 190
        "<exception>"          "\0" // 191
        "<function>"           "\0" // 192
        "<iterator>"           "\0" // 193
        "<lookup>"             "\0" // 194
        "<method>"             "\0" // 195
        "<none>"               "\0" // 196
        "<stacklet>"           "\0" // 197
        "sys"                  "\0" // 198
        "machine"              "\0" // 199
        "ticker"               "\0" // 200
        "ticks"                "\0" // 201
        "<machine.pins>"       "\0" // 202
        "enable"               "\0" // 203
        "disable"              "\0" // 204
        "transfer"             "\0" // 205
        "<machine.spi>"        "\0" // 206
        "receive"              "\0" // 207
        "sleep"                "\0" // 208
        "<machine.rf69>"       "\0" // 209
        "pins"                 "\0" // 210
        "spi"                  "\0" // 211
        "rf69"                 "\0" // 212
        "dog"                  "\0" // 213
        "kick"                 "\0" // 214
        //CG>
    ;

    extern int const qstrBaseLen = sizeof qstrBase;

} // namespace monty
