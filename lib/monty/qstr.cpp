// qstr.cpp - set of "quick strings" from MicroPython 1.13 plus own entries

namespace monty {

    extern char const qstrBase [] =
        //CG< qstr-emit v
        "\x7A\x01\x35\x02\x36\x02\x3E\x02\x40\x02\x42\x02\x44\x02\x46\x02"
        "\x4F\x02\x51\x02\x5A\x02\x64\x02\x70\x02\x7A\x02\x83\x02\x8F\x02"
        "\x9B\x02\xA4\x02\xAD\x02\xB5\x02\xBE\x02\xC6\x02\xCF\x02\xDA\x02"
        "\xE3\x02\xEB\x02\xF4\x02\x01\x03\x0A\x03\x16\x03\x1E\x03\x2E\x03"
        "\x3D\x03\x4C\x03\x5A\x03\x63\x03\x6C\x03\x76\x03\x84\x03\x90\x03"
        "\xA1\x03\xAC\x03\xB5\x03\xC7\x03\xD3\x03\xDF\x03\xE9\x03\xF2\x03"
        "\x06\x04\x0E\x04\x1C\x04\x29\x04\x37\x04\x43\x04\x4E\x04\x58\x04"
        "\x63\x04\x75\x04\x79\x04\x7D\x04\x81\x04\x88\x04\x8D\x04\x92\x04"
        "\x9B\x04\xA5\x04\xAE\x04\xB4\x04\xBD\x04\xC1\x04\xCD\x04\xD3\x04"
        "\xD9\x04\xDF\x04\xE4\x04\xEA\x04\xEF\x04\xF3\x04\xFA\x04\xFE\x04"
        "\x07\x05\x0C\x05\x11\x05\x18\x05\x1D\x05\x24\x05\x2F\x05\x33\x05"
        "\x3B\x05\x43\x05\x4B\x05\x50\x05\x53\x05\x59\x05\x60\x05\x64\x05"
        "\x6C\x05\x74\x05\x7F\x05\x87\x05\x8F\x05\x9A\x05\xA2\x05\xA8\x05"
        "\xAD\x05\xB2\x05\xB6\x05\xBB\x05\xBF\x05\xC4\x05\xCB\x05\xD2\x05"
        "\xD8\x05\xDF\x05\xE4\x05\xE8\x05\xF4\x05\xF9\x05\x00\x06\x05\x06"
        "\x09\x06\x0D\x06\x15\x06\x19\x06\x1F\x06\x25\x06\x2A\x06\x33\x06"
        "\x3C\x06\x43\x06\x4B\x06\x50\x06\x58\x06\x5E\x06\x65\x06\x6B\x06"
        "\x72\x06\x79\x06\x7E\x06\x83\x06\x87\x06\x8B\x06\x93\x06\x9E\x06"
        "\xA3\x06\xAA\x06\xB0\x06\xB6\x06\xC1\x06\xCE\x06\xD3\x06\xD8\x06"
        "\xDC\x06\xE2\x06\xE6\x06\xEC\x06\xF2\x06\xFB\x06\x01\x07\x06\x07"
        "\x0D\x07\x13\x07\x19\x07\x1F\x07\x26\x07\x2C\x07\x30\x07\x3D\x07"
        "\x49\x07\x52\x07\x59\x07\x63\x07\x6E\x07\x76\x07\x82\x07\x8D\x07"
        "\x98\x07\xA1\x07\xAA\x07\xB1\x07\xBC\x07\xC2\x07\xC8\x07\xD1\x07"
        "\xDC\x07\xE7\x07\xF2\x07\xF9\x07\x03\x08"
        // index [0..377], hashes [378..564], 187 strings [565..2050]
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
        "\xB7\x4E\x7D\x98\xE6\x22\xC1\xA7\xE1\x56\xB0\x4B\xB0\x25\x05\x45"
        "\xF8\x4D\xF4\xAB\xB5\xB2\x2E\x40\x8F\x15\x03"
        // found 136 distinct hashes
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
        "<buffer>"             "\0" // 168
        "<cell>"               "\0" // 169
        "<closure>"            "\0" // 170
        "<dictview>"           "\0" // 171
        "<event>"              "\0" // 172
        "<exception>"          "\0" // 173
        "<function>"           "\0" // 174
        "<iterator>"           "\0" // 175
        "<lookup>"             "\0" // 176
        "<method>"             "\0" // 177
        "<none>"               "\0" // 178
        "<stacklet>"           "\0" // 179
        "class"                "\0" // 180
        "slice"                "\0" // 181
        "<object>"             "\0" // 182
        "<instance>"           "\0" // 183
        "<bytecode>"           "\0" // 184
        "<callable>"           "\0" // 185
        "<pyvm>"               "\0" // 186
        "__bases__"            "\0" // 187
        //CG>
    ;

    extern int const qstrBaseLen = sizeof qstrBase;

} // namespace monty
