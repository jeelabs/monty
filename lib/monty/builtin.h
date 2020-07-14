// Some implementation details for the built-in data types and functions.

const TypeObj  Object::info ("<object>");
const TypeObj& Object::type () const { return info; }

//CG< builtin-types lib/monty/monty.h
const TypeObj     ArrayObj::info ("<array>");
const TypeObj      BoolObj::info ("<bool>");
const TypeObj BoundMethObj::info ("<bound-meth>");
const TypeObj  BytecodeObj::info ("<bytecode>");
const TypeObj      Context::info ("<context>");
const TypeObj     FrameObj::info ("<frame>");
const TypeObj       FunObj::info ("<function>");
const TypeObj      IterObj::info ("<iterator>");
const TypeObj    LookupObj::info ("<lookup>");
const TypeObj      MethObj::info ("<method>");
const TypeObj    ModuleObj::info ("<module>");
const TypeObj    MutSeqObj::info ("<mut-seq>");
const TypeObj      NoneObj::info ("<none>");
const TypeObj ResumableObj::info ("<resumable>");
const TypeObj       SeqObj::info ("<sequence>");
const TypeObj      TypeObj::info ("<type>");

const TypeObj BytesObj::info ("bytes", BytesObj::create, &BytesObj::attrs);
const TypeObj ClassObj::info ("class", ClassObj::create, &ClassObj::attrs);
const TypeObj  DictObj::info ("dict", DictObj::create, &DictObj::attrs);
const TypeObj   IntObj::info ("int", IntObj::create, &IntObj::attrs);
const TypeObj  ListObj::info ("list", ListObj::create, &ListObj::attrs);
const TypeObj   StrObj::info ("str", StrObj::create, &StrObj::attrs);
const TypeObj TupleObj::info ("tuple", TupleObj::create, &TupleObj::attrs);

const TypeObj&     ArrayObj::type () const { return info; }
const TypeObj&      BoolObj::type () const { return info; }
const TypeObj& BoundMethObj::type () const { return info; }
const TypeObj&  BytecodeObj::type () const { return info; }
const TypeObj&      Context::type () const { return info; }
const TypeObj&     FrameObj::type () const { return info; }
const TypeObj&       FunObj::type () const { return info; }
const TypeObj&      IterObj::type () const { return info; }
const TypeObj&    LookupObj::type () const { return info; }
const TypeObj&      MethObj::type () const { return info; }
const TypeObj&    ModuleObj::type () const { return info; }
const TypeObj&    MutSeqObj::type () const { return info; }
const TypeObj&      NoneObj::type () const { return info; }
const TypeObj& ResumableObj::type () const { return info; }
const TypeObj&       SeqObj::type () const { return info; }
const TypeObj&      TypeObj::type () const { return info; }
const TypeObj&     BytesObj::type () const { return info; }
const TypeObj&     ClassObj::type () const { return info; }
const TypeObj&      DictObj::type () const { return info; }
const TypeObj&       IntObj::type () const { return info; }
const TypeObj&      ListObj::type () const { return info; }
const TypeObj&       StrObj::type () const { return info; }
const TypeObj&     TupleObj::type () const { return info; }
//CG>

struct Printer : ResumableObj {
    Printer (int argc, Value argv[]) : ResumableObj (argc, argv) {}

    void style (const char* pre, const char* sep, const char* post) {
        prefix = pre;
        separator = sep;
        postfix = post;
    }

    bool step (Value v) override {
        if (pos >= nargs) {
            printf("%s", postfix);
            return false;
        }
        printf("%s", pos == 0 ? prefix : separator);
        retVal = Context::print(args[pos++]);
        return true;
    }

private:
    int pos = 0;
    const char* prefix ="";
    const char* separator =" ";
    const char* postfix ="\n";
};

Value Context::print (Value v) {
    switch (v.tag()) {
        case Value::Nil: printf("Nil"); break;
        case Value::Int: printf("%d", (int) v); break;
        case Value::Str: printf("%s", (const char*) v); break;
        case Value::Obj: {
            auto ps = v.asType<StrObj>();
            if (ps != 0) {
                printf("%s", (const char*) *ps);
                break;
            }
            auto pb = v.asType<BytesObj>();
            if (pb != 0) {
                printf("b'");
                int n = pb->len();
                auto p = (const uint8_t*) *pb;
                for (int i = 0; i < n; ++i)
                    printf("%c", p[i]); // TODO yuck, and not complete
                printf("'");
                break;
            }
            auto pl = v.asType<ListObj>();
            if (pl != 0) {
                auto& v = pl->asVec(); // TODO yuck
                auto p = new Printer (v.length(), (Value*) v.getPtr(0));
                p->style("[", ", ", "]");
                return p;
            }
#if 0
            auto pt = v.asType<TupleObj>();
            if (pt != 0) {
                auto& v = pt->asVec(); // TODO yuck
                auto p = new Printer (v.length(), &v.get(0));
                p->style("[", ", ", "]");
                return p;
            }
#endif
            auto& o = v.obj();
            printf("<Obj %s at %p>", o.type().name, &o);
            break;
        }
    }
    return Value::nil;
}

static Value bi_print (int argc, Value argv[]) {
    return new Printer (argc, argv);
}

#if 0
//CG1 builtin print
static Value bi_print (int argc, Value argv[]) {
    for (int i = 0; i < argc; ++i) {
        if (i > 0)
            printf(" ");
        Context::print(argv[i]);
    }
    printf("\n");
    return Value::nil;
}
#endif

//CG1 builtin len
static Value bi_len (int argc, Value argv[]) {
    assert(argc == 1);
    return argv[0].objPtr()->asSeq().len();
}

//CG1 builtin next
static Value bi_next (int argc, Value argv[]) {
    assert(argc == 1);
    return argv[0].objPtr()->next();
}

//CG1 builtin type
static Value bi_type (int argc, Value argv[]) {
    assert(argc == 1);
    return argv[0].objPtr()->type().name;
}

//CG< builtin-emit 0
static const FunObj f_print (bi_print);
static const FunObj f_len (bi_len);
static const FunObj f_next (bi_next);
static const FunObj f_type (bi_type);
//CG>

static const StrObj s_version = VERSION;

static Value f_suspend (int argc, Value argv[]) {
    auto qp = &Context::tasks;
    if (argc > 1) {
        qp = argv[1].asType<ListObj>();
        assert(qp != 0);
    }
    Context::suspend(*qp);
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
    { "len", &f_len },
    { "next", &f_next },
    { "type", &f_type },
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
