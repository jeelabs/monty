// qstr.cpp - set of "quick strings" from MicroPython 1.12 plus own entries

namespace Monty {

    extern char const qstrBase [] =
        //CG< qstr-emit v
        "\x8C\x01\x50\x02\x51\x02\x59\x02\x5B\x02\x5D\x02\x5F\x02\x61\x02"
        "\x6A\x02\x6C\x02\x75\x02\x7F\x02\x8B\x02\x95\x02\x9E\x02\xAA\x02"
        "\xB6\x02\xBF\x02\xC8\x02\xD0\x02\xD9\x02\xE1\x02\xEA\x02\xF5\x02"
        "\xFE\x02\x06\x03\x0F\x03\x1C\x03\x25\x03\x31\x03\x39\x03\x49\x03"
        "\x58\x03\x67\x03\x75\x03\x7E\x03\x87\x03\x91\x03\x9F\x03\xAB\x03"
        "\xBC\x03\xC7\x03\xD0\x03\xE2\x03\xEE\x03\xFA\x03\x04\x04\x0D\x04"
        "\x21\x04\x29\x04\x37\x04\x44\x04\x52\x04\x5E\x04\x69\x04\x73\x04"
        "\x7E\x04\x90\x04\x94\x04\x98\x04\x9C\x04\xA3\x04\xA8\x04\xAD\x04"
        "\xB6\x04\xC0\x04\xC9\x04\xCF\x04\xD8\x04\xDC\x04\xE8\x04\xEE\x04"
        "\xF4\x04\xFA\x04\xFF\x04\x05\x05\x0A\x05\x0E\x05\x15\x05\x19\x05"
        "\x22\x05\x27\x05\x2C\x05\x33\x05\x38\x05\x3F\x05\x4A\x05\x4E\x05"
        "\x56\x05\x5E\x05\x66\x05\x6B\x05\x6E\x05\x74\x05\x7B\x05\x7F\x05"
        "\x87\x05\x8F\x05\x9A\x05\xA2\x05\xAA\x05\xB5\x05\xBD\x05\xC3\x05"
        "\xC8\x05\xCD\x05\xD1\x05\xD6\x05\xDA\x05\xDF\x05\xE6\x05\xED\x05"
        "\xF3\x05\xFA\x05\xFF\x05\x03\x06\x0F\x06\x14\x06\x1B\x06\x20\x06"
        "\x24\x06\x28\x06\x30\x06\x34\x06\x3A\x06\x40\x06\x45\x06\x4E\x06"
        "\x57\x06\x5E\x06\x66\x06\x6B\x06\x73\x06\x79\x06\x80\x06\x86\x06"
        "\x8D\x06\x94\x06\x99\x06\x9E\x06\xA2\x06\xA6\x06\xAE\x06\xB9\x06"
        "\xBE\x06\xC5\x06\xCB\x06\xD1\x06\xDC\x06\xE9\x06\xEE\x06\xF3\x06"
        "\xF7\x06\xFD\x06\x01\x07\x07\x07\x0D\x07\x16\x07\x1C\x07\x21\x07"
        "\x28\x07\x2E\x07\x34\x07\x3A\x07\x41\x07\x47\x07\x4B\x07\x55\x07"
        "\x68\x07\x6F\x07\x77\x07\x7D\x07\x85\x07\x94\x07\x9A\x07\xA2\x07"
        "\xAF\x07\xBB\x07\xC4\x07\xCF\x07\xDA\x07\xE1\x07\xEB\x07\xF5\x07"
        "\x01\x08\x0C\x08\x15\x08\x1E\x08\x25\x08\x2B\x08\x31\x08\x37\x08"
        "\x40\x08\x4B\x08\x4F\x08\x57\x08\x5F\x08\x66\x08"
        // index [0..395], hashes [396..591], 196 strings [592..2149]
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
        "\xB7\x4E\x7D\x98\xE6\x03\x3C\xC7\x6F\x5B\xEC\x17\x64\xBF\x22\xC1"
        "\xA7\x40\x8F\xE1\x56\x98\xB0\x25\x45\xF8\x4D\x7C\xAB\xB5\xB2\x2E"
        "\xBC\x60\x5B\xC6"
        // found 141 distinct hashes
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
        "__bases__"            "\0" // 166
        "v0.93-146-ge5593bb"   "\0" // 167
        "snooze"               "\0" // 168
        "suspend"              "\0" // 169
        "tasks"                "\0" // 170
        "modules"              "\0" // 171
        "implementation"       "\0" // 172
        "monty"                "\0" // 173
        "version"              "\0" // 174
        "UnicodeError"         "\0" // 175
        "<boundmeth>"          "\0" // 176
        "<buffer>"             "\0" // 177
        "<bytecode>"           "\0" // 178
        "<callable>"           "\0" // 179
        "<cell>"               "\0" // 180
        "<closure>"            "\0" // 181
        "<context>"            "\0" // 182
        "<exception>"          "\0" // 183
        "<function>"           "\0" // 184
        "<lookup>"             "\0" // 185
        "<method>"             "\0" // 186
        "<none>"               "\0" // 187
        "array"                "\0" // 188
        "class"                "\0" // 189
        "slice"                "\0" // 190
        "<object>"             "\0" // 191
        "<instance>"           "\0" // 192
        "sys"                  "\0" // 193
        "machine"              "\0" // 194
        "network"              "\0" // 195
        "sdcard"               "\0" // 196
        //CG>
    ;

    extern int const qstrBaseLen = sizeof qstrBase;

} // namespace Monty
