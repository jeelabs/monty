// qstr.cpp - set of "quick strings" from MicroPython 1.13 plus own entries

namespace monty {

    extern char const qstrBase [] =
        //CG< qstr-emit v
        "\x50\x01\xF6\x01\xF7\x01\xFF\x01\x01\x02\x03\x02\x05\x02\x07\x02"
        "\x10\x02\x12\x02\x1B\x02\x25\x02\x31\x02\x3B\x02\x44\x02\x50\x02"
        "\x5C\x02\x65\x02\x6E\x02\x76\x02\x7F\x02\x87\x02\x90\x02\x9B\x02"
        "\xA4\x02\xAC\x02\xB5\x02\xC2\x02\xCB\x02\xD7\x02\xDF\x02\xEF\x02"
        "\xFE\x02\x0D\x03\x1B\x03\x24\x03\x2D\x03\x37\x03\x45\x03\x51\x03"
        "\x62\x03\x6D\x03\x76\x03\x88\x03\x94\x03\xA0\x03\xAA\x03\xB3\x03"
        "\xC7\x03\xCF\x03\xDD\x03\xEA\x03\xF8\x03\x04\x04\x0F\x04\x19\x04"
        "\x24\x04\x36\x04\x3A\x04\x3E\x04\x42\x04\x49\x04\x4E\x04\x53\x04"
        "\x5C\x04\x66\x04\x6F\x04\x75\x04\x7E\x04\x82\x04\x8E\x04\x94\x04"
        "\x9A\x04\xA0\x04\xA5\x04\xAB\x04\xB0\x04\xB4\x04\xBB\x04\xBF\x04"
        "\xC8\x04\xCD\x04\xD2\x04\xD9\x04\xDE\x04\xE5\x04\xF0\x04\xF4\x04"
        "\xFC\x04\x04\x05\x0C\x05\x11\x05\x14\x05\x1A\x05\x21\x05\x25\x05"
        "\x2D\x05\x35\x05\x40\x05\x48\x05\x50\x05\x5B\x05\x63\x05\x69\x05"
        "\x6E\x05\x73\x05\x77\x05\x7C\x05\x80\x05\x85\x05\x8C\x05\x93\x05"
        "\x99\x05\xA0\x05\xA5\x05\xA9\x05\xB5\x05\xBA\x05\xC1\x05\xC6\x05"
        "\xCA\x05\xCE\x05\xD6\x05\xDA\x05\xE0\x05\xE6\x05\xEB\x05\xF4\x05"
        "\xFD\x05\x04\x06\x0C\x06\x11\x06\x19\x06\x1F\x06\x26\x06\x2C\x06"
        "\x33\x06\x3A\x06\x3F\x06\x44\x06\x48\x06\x4C\x06\x54\x06\x5F\x06"
        "\x64\x06\x6B\x06\x71\x06\x77\x06\x82\x06\x8F\x06\x94\x06\x99\x06"
        "\x9D\x06\xA3\x06\xA7\x06\xAD\x06\xB3\x06\xBC\x06\xC2\x06\xC7\x06"
        "\xCE\x06\xD4\x06\xDA\x06\xE0\x06\xE7\x06\xED\x06\xF1\x06\xFE\x06"
        // index [0..335], hashes [336..501], 166 strings [502..1789]
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
        "\xB7\x4E\x7D\x98\xE6\x22"
        // found 126 distinct hashes
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
        //CG>
    ;

    extern int const qstrBaseLen = sizeof qstrBase;

} // namespace monty
