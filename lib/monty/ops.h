// the opcodes and unary/binary operators, as emitted by MicroPython 1.13

namespace monty {
    enum UnOp : uint8_t {
        Pos, Neg, Inv, Not,
        Boln, Hash, Abs, Int,
    };

    enum BinOp : uint8_t {
        //CG< binops ../../git/micropython/py/runtime0.h 35
        Less,
        More,
        Equal,
        LessEqual,
        MoreEqual,
        NotEqual,
        In,
        Is,
        ExceptionMatch,
        InplaceOr,
        InplaceXor,
        InplaceAnd,
        InplaceLshift,
        InplaceRshift,
        InplaceAdd,
        InplaceSubtract,
        InplaceMultiply,
        InplaceMatMultiply,
        InplaceFloorDivide,
        InplaceTrueDivide,
        InplaceModulo,
        InplacePower,
        Or,
        Xor,
        And,
        Lshift,
        Rshift,
        Add,
        Subtract,
        Multiply,
        MatMultiply,
        FloorDivide,
        TrueDivide,
        Modulo,
        Power,
        //CG>
        Contains,
    };

} // namespace monty
