// qstr.h - fixed set of "quick strings" in MicroPython 1.13

namespace Monty {

    char const qstrData [] =
        //CG< qstr 1
        ""                    "\0" // 1
        "__dir__"             "\0" // 2
        "\x0a"                "\0" // 3
        " "                   "\0" // 4
        "*"                   "\0" // 5
        "/"                   "\0" // 6
        "<module>"            "\0" // 7
        "_"                   "\0" // 8
        "__call__"            "\0" // 9
        "__class__"           "\0" // 10
        "__delitem__"         "\0" // 11
        "__enter__"           "\0" // 12
        "__exit__"            "\0" // 13
        "__getattr__"         "\0" // 14
        "__getitem__"         "\0" // 15
        "__hash__"            "\0" // 16
        "__init__"            "\0" // 17
        "__int__"             "\0" // 18
        "__iter__"            "\0" // 19
        "__len__"             "\0" // 20
        "__main__"            "\0" // 21
        "__module__"          "\0" // 22
        "__name__"            "\0" // 23
        "__new__"             "\0" // 24
        "__next__"            "\0" // 25
        "__qualname__"        "\0" // 26
        "__repr__"            "\0" // 27
        "__setitem__"         "\0" // 28
        "__str__"             "\0" // 29
        "ArithmeticError"     "\0" // 30
        "AssertionError"      "\0" // 31
        "AttributeError"      "\0" // 32
        "BaseException"       "\0" // 33
        "EOFError"            "\0" // 34
        "Ellipsis"            "\0" // 35
        "Exception"           "\0" // 36
        "GeneratorExit"       "\0" // 37
        "ImportError"         "\0" // 38
        "IndentationError"    "\0" // 39
        "IndexError"          "\0" // 40
        "KeyError"            "\0" // 41
        "KeyboardInterrupt"   "\0" // 42
        "LookupError"         "\0" // 43
        "MemoryError"         "\0" // 44
        "NameError"           "\0" // 45
        "NoneType"            "\0" // 46
        "NotImplementedError" "\0" // 47
        "OSError"             "\0" // 48
        "OverflowError"       "\0" // 49
        "RuntimeError"        "\0" // 50
        "StopIteration"       "\0" // 51
        "SyntaxError"         "\0" // 52
        "SystemExit"          "\0" // 53
        "TypeError"           "\0" // 54
        "ValueError"          "\0" // 55
        "ZeroDivisionError"   "\0" // 56
        "abs"                 "\0" // 57
        "all"                 "\0" // 58
        "any"                 "\0" // 59
        "append"              "\0" // 60
        "args"                "\0" // 61
        "bool"                "\0" // 62
        "builtins"            "\0" // 63
        "bytearray"           "\0" // 64
        "bytecode"            "\0" // 65
        "bytes"               "\0" // 66
        "callable"            "\0" // 67
        "chr"                 "\0" // 68
        "classmethod"         "\0" // 69
        "clear"               "\0" // 70
        "close"               "\0" // 71
        "const"               "\0" // 72
        "copy"                "\0" // 73
        "count"               "\0" // 74
        "dict"                "\0" // 75
        "dir"                 "\0" // 76
        "divmod"              "\0" // 77
        "end"                 "\0" // 78
        "endswith"            "\0" // 79
        "eval"                "\0" // 80
        "exec"                "\0" // 81
        "extend"              "\0" // 82
        "find"                "\0" // 83
        "format"              "\0" // 84
        "from_bytes"          "\0" // 85
        "get"                 "\0" // 86
        "getattr"             "\0" // 87
        "globals"             "\0" // 88
        "hasattr"             "\0" // 89
        "hash"                "\0" // 90
        "id"                  "\0" // 91
        "index"               "\0" // 92
        "insert"              "\0" // 93
        "int"                 "\0" // 94
        "isalpha"             "\0" // 95
        "isdigit"             "\0" // 96
        "isinstance"          "\0" // 97
        "islower"             "\0" // 98
        "isspace"             "\0" // 99
        "issubclass"          "\0" // 100
        "isupper"             "\0" // 101
        "items"               "\0" // 102
        "iter"                "\0" // 103
        "join"                "\0" // 104
        "key"                 "\0" // 105
        "keys"                "\0" // 106
        "len"                 "\0" // 107
        "list"                "\0" // 108
        "little"              "\0" // 109
        "locals"              "\0" // 110
        "lower"               "\0" // 111
        "lstrip"              "\0" // 112
        "main"                "\0" // 113
        "map"                 "\0" // 114
        "micropython"         "\0" // 115
        "next"                "\0" // 116
        "object"              "\0" // 117
        "open"                "\0" // 118
        "ord"                 "\0" // 119
        "pop"                 "\0" // 120
        "popitem"             "\0" // 121
        "pow"                 "\0" // 122
        "print"               "\0" // 123
        "range"               "\0" // 124
        "read"                "\0" // 125
        "readinto"            "\0" // 126
        "readline"            "\0" // 127
        "remove"              "\0" // 128
        "replace"             "\0" // 129
        "repr"                "\0" // 130
        "reverse"             "\0" // 131
        "rfind"               "\0" // 132
        "rindex"              "\0" // 133
        "round"               "\0" // 134
        "rsplit"              "\0" // 135
        "rstrip"              "\0" // 136
        "self"                "\0" // 137
        "send"                "\0" // 138
        "sep"                 "\0" // 139
        "set"                 "\0" // 140
        "setattr"             "\0" // 141
        "setdefault"          "\0" // 142
        "sort"                "\0" // 143
        "sorted"              "\0" // 144
        "split"               "\0" // 145
        "start"               "\0" // 146
        "startswith"          "\0" // 147
        "staticmethod"        "\0" // 148
        "step"                "\0" // 149
        "stop"                "\0" // 150
        "str"                 "\0" // 151
        "strip"               "\0" // 152
        "sum"                 "\0" // 153
        "super"               "\0" // 154
        "throw"               "\0" // 155
        "to_bytes"            "\0" // 156
        "tuple"               "\0" // 157
        "type"                "\0" // 158
        "update"              "\0" // 159
        "upper"               "\0" // 160
        "utf-8"               "\0" // 161
        "value"               "\0" // 162
        "values"              "\0" // 163
        "write"               "\0" // 164
        "zip"                 "\0" // 165
        //CG>
    ;

} // namespace Monty
