// Some implementation details for the built-in data types and functions.

TypeObj  Object::info ("<object>");
TypeObj& Object::type () const { return info; }

//CG< builtin-types lib/monty/monty.h
TypeObj      BoolObj::info ("<bool>");
TypeObj BoundMethObj::info ("<bound-meth>");
TypeObj  BytecodeObj::info ("<bytecode>");
TypeObj      Context::info ("<context>");
TypeObj     FrameObj::info ("<frame>");
TypeObj       FunObj::info ("<function>");
TypeObj      IterObj::info ("<iterator>");
TypeObj    LookupObj::info ("<lookup>");
TypeObj      MethObj::info ("<method>");
TypeObj    ModuleObj::info ("<module>");
TypeObj    MutSeqObj::info ("<mut-seq>");
TypeObj      NoneObj::info ("<none>");
TypeObj       SeqObj::info ("<sequence>");
TypeObj      TypeObj::info ("<type>");

TypeObj BytesObj::info ("bytes", BytesObj::create, &BytesObj::attrs);
TypeObj ClassObj::info ("class", ClassObj::create, &ClassObj::attrs);
TypeObj  DictObj::info ("dict", DictObj::create, &DictObj::attrs);
TypeObj   IntObj::info ("int", IntObj::create, &IntObj::attrs);
TypeObj  ListObj::info ("list", ListObj::create, &ListObj::attrs);
TypeObj   StrObj::info ("str", StrObj::create, &StrObj::attrs);
TypeObj TupleObj::info ("tuple", TupleObj::create, &TupleObj::attrs);

TypeObj&      BoolObj::type () const { return info; }
TypeObj& BoundMethObj::type () const { return info; }
TypeObj&  BytecodeObj::type () const { return info; }
TypeObj&      Context::type () const { return info; }
TypeObj&     FrameObj::type () const { return info; }
TypeObj&       FunObj::type () const { return info; }
TypeObj&      IterObj::type () const { return info; }
TypeObj&    LookupObj::type () const { return info; }
TypeObj&      MethObj::type () const { return info; }
TypeObj&    ModuleObj::type () const { return info; }
TypeObj&    MutSeqObj::type () const { return info; }
TypeObj&      NoneObj::type () const { return info; }
TypeObj&       SeqObj::type () const { return info; }
TypeObj&      TypeObj::type () const { return info; }
TypeObj&     BytesObj::type () const { return info; }
TypeObj&     ClassObj::type () const { return info; }
TypeObj&      DictObj::type () const { return info; }
TypeObj&       IntObj::type () const { return info; }
TypeObj&      ListObj::type () const { return info; }
TypeObj&       StrObj::type () const { return info; }
TypeObj&     TupleObj::type () const { return info; }
//CG>

void Context::print (Value v) {
    switch (v.tag()) {
        case Value::Nil: printf("<nil>"); break;
        case Value::Int: printf("<Int %d>", (int) v); break;
        case Value::Str: printf("<Str '%s' at %p>",
                                 (const char*) v, (const char*) v); break;
        case Value::Obj: {
            auto& o = v.obj();
            auto& t = o.type();
            if (&t == &StrObj::info)
                printf("%s", (const char*) (const StrObj&) o);
            else
                printf("<Obj %s at %p>", t.name, &o);
            break;
        }
    }
}

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

//CG2 builtin-emit 0
static const FunObj f_print (bi_print);
static const FunObj f_next (bi_next);

static const StrObj s_version = VERSION;

static Value f_suspend (int argc, Value argv[]) {
    Context::suspend(argc > 1 ? argv[1].asType<ListObj>() : Context::tasks);
    return Value::nil;
}

static const FunObj fo_suspend (f_suspend);

static const LookupObj::Item lo_monty [] = {
    { "version", &s_version },
    { "tasks", &Context::tasks },
    { "suspend", &fo_suspend },
};

static const LookupObj ma_monty (lo_monty, sizeof lo_monty / sizeof *lo_monty);
static const ModuleObj m_monty (&ma_monty);

static const LookupObj::Item builtins [] = {
    //CG< builtin-emit 1
    { "bytes", &BytesObj::info },
    { "class", &ClassObj::info },
    { "dict", &DictObj::info },
    { "int", &IntObj::info },
    { "list", &ListObj::info },
    { "str", &StrObj::info },
    { "tuple", &TupleObj::info },
    { "print", &f_print },
    { "next", &f_next },
    //CG>
    { "monty", &m_monty },
    { "machine", &m_machine },
#if INCLUDE_NETWORK
    { "network", &m_network },
#endif
#if INCLUDE_SDCARD
    { "sdcard", &m_sdcard },
#endif
};

const LookupObj builtinDict (builtins, sizeof builtins / sizeof *builtins);
