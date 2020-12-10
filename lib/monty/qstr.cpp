// qstr.cpp - set of "quick strings" from MicroPython 1.13 plus own entries

namespace monty {

    extern char const qstrBase [] =
        //CG< qstr-emit v
        "\x72\x01\x29\x02\x2A\x02\x32\x02\x34\x02\x36\x02\x38\x02\x3A\x02"
        "\x43\x02\x45\x02\x4E\x02\x58\x02\x64\x02\x6E\x02\x77\x02\x83\x02"
        "\x8F\x02\x98\x02\xA1\x02\xA9\x02\xB2\x02\xBA\x02\xC3\x02\xCE\x02"
        "\xD7\x02\xDF\x02\xE8\x02\xF5\x02\xFE\x02\x0A\x03\x12\x03\x22\x03"
        "\x31\x03\x40\x03\x4E\x03\x57\x03\x60\x03\x6A\x03\x78\x03\x84\x03"
        "\x95\x03\xA0\x03\xA9\x03\xBB\x03\xC7\x03\xD3\x03\xDD\x03\xE6\x03"
        "\xFA\x03\x02\x04\x10\x04\x1D\x04\x2B\x04\x37\x04\x42\x04\x4C\x04"
        "\x57\x04\x69\x04\x6D\x04\x71\x04\x75\x04\x7C\x04\x81\x04\x86\x04"
        "\x8F\x04\x99\x04\xA2\x04\xA8\x04\xB1\x04\xB5\x04\xC1\x04\xC7\x04"
        "\xCD\x04\xD3\x04\xD8\x04\xDE\x04\xE3\x04\xE7\x04\xEE\x04\xF2\x04"
        "\xFB\x04\x00\x05\x05\x05\x0C\x05\x11\x05\x18\x05\x23\x05\x27\x05"
        "\x2F\x05\x37\x05\x3F\x05\x44\x05\x47\x05\x4D\x05\x54\x05\x58\x05"
        "\x60\x05\x68\x05\x73\x05\x7B\x05\x83\x05\x8E\x05\x96\x05\x9C\x05"
        "\xA1\x05\xA6\x05\xAA\x05\xAF\x05\xB3\x05\xB8\x05\xBF\x05\xC6\x05"
        "\xCC\x05\xD3\x05\xD8\x05\xDC\x05\xE8\x05\xED\x05\xF4\x05\xF9\x05"
        "\xFD\x05\x01\x06\x09\x06\x0D\x06\x13\x06\x19\x06\x1E\x06\x27\x06"
        "\x30\x06\x37\x06\x3F\x06\x44\x06\x4C\x06\x52\x06\x59\x06\x5F\x06"
        "\x66\x06\x6D\x06\x72\x06\x77\x06\x7B\x06\x7F\x06\x87\x06\x92\x06"
        "\x97\x06\x9E\x06\xA4\x06\xAA\x06\xB5\x06\xC2\x06\xC7\x06\xCC\x06"
        "\xD0\x06\xD6\x06\xDA\x06\xE0\x06\xE6\x06\xEF\x06\xF5\x06\xFA\x06"
        "\x01\x07\x07\x07\x0D\x07\x13\x07\x1A\x07\x20\x07\x24\x07\x31\x07"
        "\x3D\x07\x48\x07\x4F\x07\x59\x07\x63\x07\x6E\x07\x7A\x07\x85\x07"
        "\x90\x07\x99\x07\xA2\x07\xA9\x07\xAF\x07\xB5\x07\xBE\x07\xC9\x07"
        "\xD3\x07"
        // index [0..369], hashes [370..552], 183 strings [553..2002]
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
        "\xB7\x4E\x7D\x98\xE6\x22\xC1\x8F\xE1\x56\x98\xB0\xB0\x25\x05\x45"
        "\xF8\x4D\xAB\xB5\xB2\x2E\x03"
        // found 134 distinct hashes
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
        "<boundmeth>"          "\0" // 167
        "<callable>"           "\0" // 168
        "<cell>"               "\0" // 169
        "<closure>"            "\0" // 170
        "<context>"            "\0" // 171
        "<dictview>"           "\0" // 172
        "<exception>"          "\0" // 173
        "<function>"           "\0" // 174
        "<iterator>"           "\0" // 175
        "<lookup>"             "\0" // 176
        "<method>"             "\0" // 177
        "<none>"               "\0" // 178
        "class"                "\0" // 179
        "slice"                "\0" // 180
        "<object>"             "\0" // 181
        "<instance>"           "\0" // 182
        "__bases__"            "\0" // 183
        //CG>
    ;

    extern int const qstrBaseLen = sizeof qstrBase;

} // namespace monty
