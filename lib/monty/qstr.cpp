// qstr.cpp - set of "quick strings" from MicroPython 1.13 plus own entries

namespace monty {

    extern char const qstrBase [] =
        //CG< qstr-emit v
        "\x68\x01\x1A\x02\x1B\x02\x23\x02\x25\x02\x27\x02\x29\x02\x2B\x02"
        "\x34\x02\x36\x02\x3F\x02\x49\x02\x55\x02\x5F\x02\x68\x02\x74\x02"
        "\x80\x02\x89\x02\x92\x02\x9A\x02\xA3\x02\xAB\x02\xB4\x02\xBF\x02"
        "\xC8\x02\xD0\x02\xD9\x02\xE6\x02\xEF\x02\xFB\x02\x03\x03\x13\x03"
        "\x22\x03\x31\x03\x3F\x03\x48\x03\x51\x03\x5B\x03\x69\x03\x75\x03"
        "\x86\x03\x91\x03\x9A\x03\xAC\x03\xB8\x03\xC4\x03\xCE\x03\xD7\x03"
        "\xEB\x03\xF3\x03\x01\x04\x0E\x04\x1C\x04\x28\x04\x33\x04\x3D\x04"
        "\x48\x04\x5A\x04\x5E\x04\x62\x04\x66\x04\x6D\x04\x72\x04\x77\x04"
        "\x80\x04\x8A\x04\x93\x04\x99\x04\xA2\x04\xA6\x04\xB2\x04\xB8\x04"
        "\xBE\x04\xC4\x04\xC9\x04\xCF\x04\xD4\x04\xD8\x04\xDF\x04\xE3\x04"
        "\xEC\x04\xF1\x04\xF6\x04\xFD\x04\x02\x05\x09\x05\x14\x05\x18\x05"
        "\x20\x05\x28\x05\x30\x05\x35\x05\x38\x05\x3E\x05\x45\x05\x49\x05"
        "\x51\x05\x59\x05\x64\x05\x6C\x05\x74\x05\x7F\x05\x87\x05\x8D\x05"
        "\x92\x05\x97\x05\x9B\x05\xA0\x05\xA4\x05\xA9\x05\xB0\x05\xB7\x05"
        "\xBD\x05\xC4\x05\xC9\x05\xCD\x05\xD9\x05\xDE\x05\xE5\x05\xEA\x05"
        "\xEE\x05\xF2\x05\xFA\x05\xFE\x05\x04\x06\x0A\x06\x0F\x06\x18\x06"
        "\x21\x06\x28\x06\x30\x06\x35\x06\x3D\x06\x43\x06\x4A\x06\x50\x06"
        "\x57\x06\x5E\x06\x63\x06\x68\x06\x6C\x06\x70\x06\x78\x06\x83\x06"
        "\x88\x06\x8F\x06\x95\x06\x9B\x06\xA6\x06\xB3\x06\xB8\x06\xBD\x06"
        "\xC1\x06\xC7\x06\xCB\x06\xD1\x06\xD7\x06\xE0\x06\xE6\x06\xEB\x06"
        "\xF2\x06\xF8\x06\xFE\x06\x04\x07\x0B\x07\x11\x07\x15\x07\x22\x07"
        "\x2D\x07\x37\x07\x42\x07\x4E\x07\x59\x07\x64\x07\x6D\x07\x76\x07"
        "\x7D\x07\x83\x07\x8C\x07\x97\x07"
        // index [0..359], hashes [360..537], 178 strings [538..1942]
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
        "\xB7\x4E\x7D\x98\xE6\x22\x8F\x98\xB0\xB0\x25\x05\x45\xF8\x4D\xB5"
        "\xB2\x2E"
        // found 129 distinct hashes
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
        "<callable>"           "\0" // 167
        "<context>"            "\0" // 168
        "<dictview>"           "\0" // 169
        "<exception>"          "\0" // 170
        "<function>"           "\0" // 171
        "<iterator>"           "\0" // 172
        "<lookup>"             "\0" // 173
        "<method>"             "\0" // 174
        "<none>"               "\0" // 175
        "slice"                "\0" // 176
        "<object>"             "\0" // 177
        "<instance>"           "\0" // 178
        //CG>
    ;

    extern int const qstrBaseLen = sizeof qstrBase;

} // namespace monty
