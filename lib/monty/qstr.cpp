// qstr.cpp - set of "quick strings" from MicroPython 1.13 plus own entries

namespace Monty {

    extern char const qstrBase [] =
        //CG< qstr-emit v
        "\x94\x01\x5C\x02\x5D\x02\x65\x02\x67\x02\x69\x02\x6B\x02\x6D\x02"
        "\x76\x02\x78\x02\x81\x02\x8B\x02\x97\x02\xA1\x02\xAA\x02\xB6\x02"
        "\xC2\x02\xCB\x02\xD4\x02\xDC\x02\xE5\x02\xED\x02\xF6\x02\x01\x03"
        "\x0A\x03\x12\x03\x1B\x03\x28\x03\x31\x03\x3D\x03\x45\x03\x55\x03"
        "\x64\x03\x73\x03\x81\x03\x8A\x03\x93\x03\x9D\x03\xAB\x03\xB7\x03"
        "\xC8\x03\xD3\x03\xDC\x03\xEE\x03\xFA\x03\x06\x04\x10\x04\x19\x04"
        "\x2D\x04\x35\x04\x43\x04\x50\x04\x5E\x04\x6A\x04\x75\x04\x7F\x04"
        "\x8A\x04\x9C\x04\xA0\x04\xA4\x04\xA8\x04\xAF\x04\xB4\x04\xB9\x04"
        "\xC2\x04\xCC\x04\xD5\x04\xDB\x04\xE4\x04\xE8\x04\xF4\x04\xFA\x04"
        "\x00\x05\x06\x05\x0B\x05\x11\x05\x16\x05\x1A\x05\x21\x05\x25\x05"
        "\x2E\x05\x33\x05\x38\x05\x3F\x05\x44\x05\x4B\x05\x56\x05\x5A\x05"
        "\x62\x05\x6A\x05\x72\x05\x77\x05\x7A\x05\x80\x05\x87\x05\x8B\x05"
        "\x93\x05\x9B\x05\xA6\x05\xAE\x05\xB6\x05\xC1\x05\xC9\x05\xCF\x05"
        "\xD4\x05\xD9\x05\xDD\x05\xE2\x05\xE6\x05\xEB\x05\xF2\x05\xF9\x05"
        "\xFF\x05\x06\x06\x0B\x06\x0F\x06\x1B\x06\x20\x06\x27\x06\x2C\x06"
        "\x30\x06\x34\x06\x3C\x06\x40\x06\x46\x06\x4C\x06\x51\x06\x5A\x06"
        "\x63\x06\x6A\x06\x72\x06\x77\x06\x7F\x06\x85\x06\x8C\x06\x92\x06"
        "\x99\x06\xA0\x06\xA5\x06\xAA\x06\xAE\x06\xB2\x06\xBA\x06\xC5\x06"
        "\xCA\x06\xD1\x06\xD7\x06\xDD\x06\xE8\x06\xF5\x06\xFA\x06\xFF\x06"
        "\x03\x07\x09\x07\x0D\x07\x13\x07\x19\x07\x22\x07\x28\x07\x2D\x07"
        "\x34\x07\x3A\x07\x40\x07\x46\x07\x4D\x07\x53\x07\x57\x07\x61\x07"
        "\x73\x07\x79\x07\x81\x07\x89\x07\x92\x07\x99\x07\xA2\x07\xB1\x07"
        "\xB7\x07\xBF\x07\xCC\x07\xD8\x07\xE1\x07\xEC\x07\xF7\x07\xFE\x07"
        "\x08\x08\x12\x08\x1D\x08\x29\x08\x34\x08\x3F\x08\x48\x08\x51\x08"
        "\x58\x08\x5E\x08\x64\x08\x6A\x08\x73\x08\x7E\x08\x82\x08\x8A\x08"
        "\x92\x08\x99\x08"
        // index [0..403], hashes [404..603], 200 strings [604..2200]
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
        "\xB7\x4E\x7D\x98\xE6\x03\xDA\x5B\xEC\x6F\x8D\x08\xFF\x17\x64\xBF"
        "\x22\xC1\xA7\x40\x8F\xE1\x56\x98\xB0\xB0\x25\x05\x45\xF8\x4D\x7C"
        "\xAB\xB5\xB2\x2E\xBC\x60\x5B\xC6"
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
        "__bases__"            "\0" // 166
        "v0.94-73-gaa93dd3"    "\0" // 167
        "tasks"                "\0" // 168
        "modules"              "\0" // 169
        "suspend"              "\0" // 170
        "gc_avail"             "\0" // 171
        "gc_now"               "\0" // 172
        "gc_stats"             "\0" // 173
        "implementation"       "\0" // 174
        "monty"                "\0" // 175
        "version"              "\0" // 176
        "UnicodeError"         "\0" // 177
        "<boundmeth>"          "\0" // 178
        "<buffer>"             "\0" // 179
        "<bytecode>"           "\0" // 180
        "<callable>"           "\0" // 181
        "<cell>"               "\0" // 182
        "<closure>"            "\0" // 183
        "<context>"            "\0" // 184
        "<dictview>"           "\0" // 185
        "<exception>"          "\0" // 186
        "<function>"           "\0" // 187
        "<iterator>"           "\0" // 188
        "<lookup>"             "\0" // 189
        "<method>"             "\0" // 190
        "<none>"               "\0" // 191
        "array"                "\0" // 192
        "class"                "\0" // 193
        "slice"                "\0" // 194
        "<object>"             "\0" // 195
        "<instance>"           "\0" // 196
        "sys"                  "\0" // 197
        "machine"              "\0" // 198
        "network"              "\0" // 199
        "sdcard"               "\0" // 200
        //CG>
    ;

    extern int const qstrBaseLen = sizeof qstrBase;

} // namespace Monty
