// qstr.cpp - set of "quick strings" from MicroPython 1.13 plus own entries

namespace Monty {

    extern char const qstrBase [] =
        //CG< qstr-emit v
        "\x96\x01\x5F\x02\x60\x02\x68\x02\x6A\x02\x6C\x02\x6E\x02\x70\x02"
        "\x79\x02\x7B\x02\x84\x02\x8E\x02\x9A\x02\xA4\x02\xAD\x02\xB9\x02"
        "\xC5\x02\xCE\x02\xD7\x02\xDF\x02\xE8\x02\xF0\x02\xF9\x02\x04\x03"
        "\x0D\x03\x15\x03\x1E\x03\x2B\x03\x34\x03\x40\x03\x48\x03\x58\x03"
        "\x67\x03\x76\x03\x84\x03\x8D\x03\x96\x03\xA0\x03\xAE\x03\xBA\x03"
        "\xCB\x03\xD6\x03\xDF\x03\xF1\x03\xFD\x03\x09\x04\x13\x04\x1C\x04"
        "\x30\x04\x38\x04\x46\x04\x53\x04\x61\x04\x6D\x04\x78\x04\x82\x04"
        "\x8D\x04\x9F\x04\xA3\x04\xA7\x04\xAB\x04\xB2\x04\xB7\x04\xBC\x04"
        "\xC5\x04\xCF\x04\xD8\x04\xDE\x04\xE7\x04\xEB\x04\xF7\x04\xFD\x04"
        "\x03\x05\x09\x05\x0E\x05\x14\x05\x19\x05\x1D\x05\x24\x05\x28\x05"
        "\x31\x05\x36\x05\x3B\x05\x42\x05\x47\x05\x4E\x05\x59\x05\x5D\x05"
        "\x65\x05\x6D\x05\x75\x05\x7A\x05\x7D\x05\x83\x05\x8A\x05\x8E\x05"
        "\x96\x05\x9E\x05\xA9\x05\xB1\x05\xB9\x05\xC4\x05\xCC\x05\xD2\x05"
        "\xD7\x05\xDC\x05\xE0\x05\xE5\x05\xE9\x05\xEE\x05\xF5\x05\xFC\x05"
        "\x02\x06\x09\x06\x0E\x06\x12\x06\x1E\x06\x23\x06\x2A\x06\x2F\x06"
        "\x33\x06\x37\x06\x3F\x06\x43\x06\x49\x06\x4F\x06\x54\x06\x5D\x06"
        "\x66\x06\x6D\x06\x75\x06\x7A\x06\x82\x06\x88\x06\x8F\x06\x95\x06"
        "\x9C\x06\xA3\x06\xA8\x06\xAD\x06\xB1\x06\xB5\x06\xBD\x06\xC8\x06"
        "\xCD\x06\xD4\x06\xDA\x06\xE0\x06\xEB\x06\xF8\x06\xFD\x06\x02\x07"
        "\x06\x07\x0C\x07\x10\x07\x16\x07\x1C\x07\x25\x07\x2B\x07\x30\x07"
        "\x37\x07\x3D\x07\x43\x07\x49\x07\x50\x07\x56\x07\x5A\x07\x64\x07"
        "\x71\x07\x7D\x07\x86\x07\x91\x07\x9C\x07\xA3\x07\xAD\x07\xB7\x07"
        "\xC2\x07\xCE\x07\xD9\x07\xE4\x07\xED\x07\xF6\x07\xFD\x07\x09\x08"
        "\x0F\x08\x15\x08\x1B\x08\x24\x08\x2F\x08\x33\x08\x3B\x08\x43\x08"
        "\x4A\x08\x5D\x08\x63\x08\x6B\x08\x73\x08\x7C\x08\x83\x08\x8C\x08"
        "\x9B\x08\xA1\x08\xA9\x08"
        // index [0..405], hashes [406..606], 201 strings [607..2216]
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
        "\xB7\x4E\x7D\x98\xE6\x03\x22\xC1\xA7\x40\x8F\xE1\x56\x98\xB0\xB0"
        "\x25\x05\x45\xF8\x4D\xB1\x7C\xAB\xB5\xB2\x2E\xBC\x60\x5B\xC6\x68"
        "\x5B\xEC\x6F\x8D\x08\xFF\x17\x64\xBF"
        // found 143 distinct hashes
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
        "UnicodeError"         "\0" // 167
        "<boundmeth>"          "\0" // 168
        "<buffer>"             "\0" // 169
        "<bytecode>"           "\0" // 170
        "<callable>"           "\0" // 171
        "<cell>"               "\0" // 172
        "<closure>"            "\0" // 173
        "<context>"            "\0" // 174
        "<dictview>"           "\0" // 175
        "<exception>"          "\0" // 176
        "<function>"           "\0" // 177
        "<iterator>"           "\0" // 178
        "<lookup>"             "\0" // 179
        "<method>"             "\0" // 180
        "<none>"               "\0" // 181
        "<resumable>"          "\0" // 182
        "array"                "\0" // 183
        "class"                "\0" // 184
        "slice"                "\0" // 185
        "<object>"             "\0" // 186
        "<instance>"           "\0" // 187
        "sys"                  "\0" // 188
        "machine"              "\0" // 189
        "network"              "\0" // 190
        "sdcard"               "\0" // 191
        "v0.94-108-ga73a6ae"   "\0" // 192
        "tasks"                "\0" // 193
        "modules"              "\0" // 194
        "suspend"              "\0" // 195
        "gc_avail"             "\0" // 196
        "gc_now"               "\0" // 197
        "gc_stats"             "\0" // 198
        "implementation"       "\0" // 199
        "monty"                "\0" // 200
        "version"              "\0" // 201
        //CG>
    ;

    extern int const qstrBaseLen = sizeof qstrBase;

} // namespace Monty
