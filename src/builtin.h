// Some implementation details for the built-in data types and functions.

TypeObj  Context::info ("<context>");
TypeObj& Context::type () const { return info; }

//CG< builtin-types lib/monty/monty.h
TypeObj BoundMethObj::info ("<bound-meth>");
TypeObj  BytecodeObj::info ("<bytecode>");
TypeObj     FrameObj::info ("<frame>");
TypeObj       FunObj::info ("<function>");
TypeObj      IterObj::info ("<iterator>");
TypeObj    LookupObj::info ("<lookup>");
TypeObj      MethObj::info ("<method>");
TypeObj    ModuleObj::info ("<module>");
TypeObj    MutSeqObj::info ("<mut-seq>");
TypeObj       SeqObj::info ("<sequence>");
TypeObj      TypeObj::info ("<type>");

TypeObj ClassObj::info ("class", ClassObj::create, &ClassObj::names);
TypeObj  DictObj::info ("dict", DictObj::create, &DictObj::names);
TypeObj   IntObj::info ("int", IntObj::create, &IntObj::names);
TypeObj  ListObj::info ("list", ListObj::create, &ListObj::names);
TypeObj   StrObj::info ("str", StrObj::create, &StrObj::names);
TypeObj TupleObj::info ("tuple", TupleObj::create, &TupleObj::names);

TypeObj& BoundMethObj::type () const { return info; }
TypeObj&  BytecodeObj::type () const { return info; }
TypeObj&     FrameObj::type () const { return info; }
TypeObj&       FunObj::type () const { return info; }
TypeObj&      IterObj::type () const { return info; }
TypeObj&    LookupObj::type () const { return info; }
TypeObj&      MethObj::type () const { return info; }
TypeObj&    ModuleObj::type () const { return info; }
TypeObj&    MutSeqObj::type () const { return info; }
TypeObj&       SeqObj::type () const { return info; }
TypeObj&      TypeObj::type () const { return info; }
TypeObj&     ClassObj::type () const { return info; }
TypeObj&      DictObj::type () const { return info; }
TypeObj&       IntObj::type () const { return info; }
TypeObj&      ListObj::type () const { return info; }
TypeObj&       StrObj::type () const { return info; }
TypeObj&     TupleObj::type () const { return info; }
//CG>

//CG1 builtin print
static Value bi_print (int argc, Value argv[]) {
    for (int i = 0; i < argc; ++i) {
        if (i > 0)
            printf(" ");
        Value v = argv[i];
        switch (v.tag()) {
            case Value::Nil: printf("Nil"); break;
            case Value::Int: printf("%d", (int) v); break;
            case Value::Str: printf("%s", (const char*) v); break;
            case Value::Obj: Context::print(v); break;
        }
    }
    printf("\n");
    return Value::nil;
}

//CG1 builtin next
static Value bi_next (int argc, Value argv[]) {
    assert(argc == 1);
    return argv[0].objPtr()->next();
}

//CG1 builtin suspend
static Value bi_suspend (int argc, Value argv[]) {
    Context::suspend();
    return Value::nil;
}

//CG3 builtin-emit 0
static const FunObj f_print (bi_print);
static const FunObj f_next (bi_next);
static const FunObj f_suspend (bi_suspend);

#include "timer.h"

const FunObj f_setTimer (xSetTimer);
const FunObj f_getTime (xGetTime);

static const LookupObj::Item builtins [] = {
    //CG< builtin-emit 1
    { "class", &ClassObj::info },
    { "dict", &DictObj::info },
    { "int", &IntObj::info },
    { "list", &ListObj::info },
    { "str", &StrObj::info },
    { "tuple", &TupleObj::info },
    { "print", &f_print },
    { "next", &f_next },
    { "suspend", &f_suspend },
    //CG>
    { "setTimer", &f_setTimer },
    { "getTime", &f_getTime },
};

const LookupObj builtinDict (builtins, sizeof builtins / sizeof *builtins);