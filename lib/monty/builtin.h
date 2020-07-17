// Some implementation details for the built-in data types and functions.

const TypeObj  Object::info ("<object>");
const TypeObj& Object::type () const { return info; }

//CG< builtin-types lib/monty/monty.h
const TypeObj      BoolObj::info ("<bool>");
const TypeObj BoundMethObj::info ("<bound-meth>");
const TypeObj  BytecodeObj::info ("<bytecode>");
const TypeObj  CallArgsObj::info ("<callargs>");
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

const TypeObj ArrayObj::info ("array", ArrayObj::create, &ArrayObj::attrs);
const TypeObj BytesObj::info ("bytes", BytesObj::create, &BytesObj::attrs);
const TypeObj ClassObj::info ("class", ClassObj::create, &ClassObj::attrs);
const TypeObj  DictObj::info ("dict", DictObj::create, &DictObj::attrs);
const TypeObj   IntObj::info ("int", IntObj::create, &IntObj::attrs);
const TypeObj  ListObj::info ("list", ListObj::create, &ListObj::attrs);
const TypeObj   SetObj::info ("set", SetObj::create, &SetObj::attrs);
const TypeObj   StrObj::info ("str", StrObj::create, &StrObj::attrs);
const TypeObj TupleObj::info ("tuple", TupleObj::create, &TupleObj::attrs);

const TypeObj&      BoolObj::type () const { return info; }
const TypeObj& BoundMethObj::type () const { return info; }
const TypeObj&  BytecodeObj::type () const { return info; }
const TypeObj&  CallArgsObj::type () const { return info; }
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
const TypeObj&     ArrayObj::type () const { return info; }
const TypeObj&     BytesObj::type () const { return info; }
const TypeObj&     ClassObj::type () const { return info; }
const TypeObj&      DictObj::type () const { return info; }
const TypeObj&       IntObj::type () const { return info; }
const TypeObj&      ListObj::type () const { return info; }
const TypeObj&       SetObj::type () const { return info; }
const TypeObj&       StrObj::type () const { return info; }
const TypeObj&     TupleObj::type () const { return info; }
//CG>

// non-recursive version for debugging, does not affect the VM state
void Value::dump (const char* msg) const {
    if (msg != 0)
        printf("%s ", msg);
    switch (tag()) {
        case Value::Nil: printf("<N>"); break;
        case Value::Int: printf("<I %d>", (int) *this); break;
        case Value::Str: printf("<S '%s'>", (const char*) *this); break;
        case Value::Obj: printf("<O %s at %p>", obj().type().name, &obj()); break;
    }
    if (msg != 0)
        printf("\n");
}

struct Printer : ResumableObj {
    Printer (Value w, int argc, Value argv[], const char* style ="\0  \n")
            : ResumableObj (argc, argv), writer (w) {
        memcpy(fmt, style, 4);
    }

    bool step (Value v) override {
        if (pos == 0 && fmt[0] != 0)
            printf("%c", fmt[0]);
        if (pos >= nargs) {
            printf("%c", fmt[3]);
            return false;
        }
        if (pos > 0)
            printf("%c", pos & 1 ? fmt[1] : fmt[2]);
        retVal = Context::print(args[pos++]);
        return true;
    }

private:
    int pos = 0;
    char fmt [4]; // prefix sepOdd, sepEven, postfix
    Value writer;
};

Value Object::repr (Value writer) const {
    printf("<Obj %s at %p>", type().name, this);
    return Value::nil;
}

static void printEscaped (Value writer, const char* fmt, uint8_t ch) {
    printf("\\");
    switch (ch) {
        case '\t': printf("t"); break;
        case '\n': printf("n"); break;
        case '\r': printf("r"); break;
        default:   printf(fmt, ch); break;
    }
}

Value BytesObj::repr (Value writer) const {
    printf("'");
    int n = len();
    for (auto p = (const uint8_t*) *this; --n >= 0; ++p)
        if (*p == '\\' || *p == '\'')
            printf("\\%c", *p);
        else if (' ' <= *p && *p <= '~')
            printf("%c", *p);
        else
            printEscaped(writer, "x%02x", *p);
    printf("'");
    return Value::nil;
}

Value StrObj::repr (Value writer) const {
    printf("\"");
    for (auto p = (const uint8_t*)(const char*) *this; *p != 0; ++p)
        if (*p == '\\' || *p == '"')
            printf("\\%c", *p);
        else if (*p >= ' ')
            printf("%c", *p);
        else
            printEscaped(writer, "u%04x", *p);
    printf("\"");
    return Value::nil;
}

Value TupleObj::repr (Value writer) const {
    return new Printer (writer, length, (Value*) vec, "(,,)"); // FIXME const!
}

Value ArrayObj::repr (Value writer) const {
    printf("%d%c", (int) length(), atype);
    auto p = (const uint8_t*) getPtr(0);
    auto n = widthOf(length());
    for (int i = 0; i < n; ++i)
        printf("%02x", p[i]);
    return Value::nil;
}

Value ListObj::repr (Value writer) const {
    return new Printer (writer, length(), (Value*) getPtr(0), "[,,]");
}

Value SetObj::repr (Value writer) const {
    return new Printer (writer, length(), (Value*) getPtr(0), "{,,}");
}

Value DictObj::repr (Value writer) const {
    return new Printer (writer, length(), (Value*) getPtr(0), "{:,}");
}

Value ClassObj::repr (Value writer) const {
    printf("<class %s>", (const char*) at("__name__"));
    return Value::nil;
}

Value InstanceObj::repr (Value writer) const {
    printf("<%s object at %p>", type().name, this);
    return Value::nil;
}

Value Context::print (Value v) {
    switch (v.tag()) {
        case Value::Nil: printf("Nil"); break;
        case Value::Int: printf("%d", (int) v); break;
        case Value::Str: printf("\"%s\"", (const char*) v); break;
        case Value::Obj: return v.obj().repr(Value::nil); // TODO writer ...
    }
    return Value::nil;
}

//CG1 builtin print
static Value bi_print (int argc, Value argv[]) {
    auto writer = Value::nil; // TODO
    return new Printer (writer, argc, argv);
}

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
    if (argc > 1)
        qp = &argv[1].asType<ListObj>();
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
    { "array", &ArrayObj::info },
    { "bytes", &BytesObj::info },
    { "class", &ClassObj::info },
    { "dict", &DictObj::info },
    { "int", &IntObj::info },
    { "list", &ListObj::info },
    { "set", &SetObj::info },
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
