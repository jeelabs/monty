// Some implementation details for the built-in data types and functions.

//CG< builtin-types lib/monty/monty.h
const TypeObj BoundMethObj::info ("<bound-meth>");
const TypeObj  BytecodeObj::info ("<bytecode>");
const TypeObj     FrameObj::info ("<frame>");
const TypeObj       FunObj::info ("<function>");
const TypeObj      IterObj::info ("<iterator>");
const TypeObj    LookupObj::info ("<lookup>");
const TypeObj      MethObj::info ("<method>");
const TypeObj    ModuleObj::info ("<module>");
const TypeObj    MutSeqObj::info ("<mut-seq>");
const TypeObj       SeqObj::info ("<sequence>");
const TypeObj      TypeObj::info ("<type>");

const TypeObj ClassObj::info ("class", ClassObj::create, &ClassObj::names);
const TypeObj  DictObj::info ("dict", DictObj::create, &DictObj::names);
const TypeObj   IntObj::info ("int", IntObj::create, &IntObj::names);
const TypeObj  ListObj::info ("list", ListObj::create, &ListObj::names);
const TypeObj   StrObj::info ("str", StrObj::create, &StrObj::names);
const TypeObj TupleObj::info ("tuple", TupleObj::create, &TupleObj::names);

const TypeObj& BoundMethObj::type () const { return info; }
const TypeObj&  BytecodeObj::type () const { return info; }
const TypeObj&     FrameObj::type () const { return info; }
const TypeObj&       FunObj::type () const { return info; }
const TypeObj&      IterObj::type () const { return info; }
const TypeObj&    LookupObj::type () const { return info; }
const TypeObj&      MethObj::type () const { return info; }
const TypeObj&    ModuleObj::type () const { return info; }
const TypeObj&    MutSeqObj::type () const { return info; }
const TypeObj&       SeqObj::type () const { return info; }
const TypeObj&      TypeObj::type () const { return info; }
const TypeObj&     ClassObj::type () const { return info; }
const TypeObj&      DictObj::type () const { return info; }
const TypeObj&       IntObj::type () const { return info; }
const TypeObj&      ListObj::type () const { return info; }
const TypeObj&       StrObj::type () const { return info; }
const TypeObj&     TupleObj::type () const { return info; }
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
