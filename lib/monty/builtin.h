// Some implementation details for the built-in data types and functions.

const TypeObj  Object::info ("<object>");
const TypeObj& Object::type () const { return info; }

//CG< builtin-types lib/monty/monty.h
const TypeObj      BoolObj::info ("<bool>");
const TypeObj BoundMethObj::info ("<bound-meth>");
const TypeObj    BufferObj::info ("<buffer>");
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
const TypeObj SliceObj::info ("slice", SliceObj::create, &SliceObj::attrs);
const TypeObj   StrObj::info ("str", StrObj::create, &StrObj::attrs);
const TypeObj TupleObj::info ("tuple", TupleObj::create, &TupleObj::attrs);

const TypeObj&      BoolObj::type () const { return info; }
const TypeObj& BoundMethObj::type () const { return info; }
const TypeObj&    BufferObj::type () const { return info; }
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
const TypeObj&     SliceObj::type () const { return info; }
const TypeObj&       StrObj::type () const { return info; }
const TypeObj&     TupleObj::type () const { return info; }
//CG>

BufferObj::BufferObj (Value wfun) : writer (wfun) {
    need(120); // TODO may need to suspend
}

void BufferObj::mark (void (*gc)(const Object&)) const {
    assert(writer.isObj());
    if (writer.isObj()) // TODO nil right now
        gc(writer.obj());
}

bool BufferObj::flush () {
    //::printf("flush %d %d\n", fill, capacity);
    for (size_t i = 0; i < fill; ++i)
        ::printf("%c", get(i)); // TODO for now, pfff
    fill = 0;
    return true;
}

bool BufferObj::need (size_t bytes) {
    if (bytes == 0 || fill + bytes > capacity) {
        if (fill > 0 && !flush())
            return false;
        auto grow = fill + bytes - capacity;
        if (grow > 0) {
            ins(fill, grow);
            del(fill - grow, grow);
        }
    }
    base = (uint8_t*) getPtr(0);
    assert(room() >= bytes);
    return true;
}

void BufferObj::putc (uint8_t v) {
    assert(fill < capacity);    // fits
    assert(base == getPtr(0));  // hasn't moved
    base[fill++] = v;
}

#if 0
void BufferObj::put (const void* p, size_t n) {
    assert(n <= room());
    memcpy(base + fill, p, n);
    fill += n;
}
#endif

// formatted output, adapted from JeeH

#include <stdarg.h>

int BufferObj::splitInt (uint32_t val, int base, uint8_t* buf) {
    int i = 0;
    do {
        buf[i++] = val % base;
        val /= base;
    } while (val != 0);
    return i;
}

void BufferObj::putFiller (int n, char fill) {
    while (--n >= 0)
        putc(fill);
}

void BufferObj::putInt (int val, int base, int width, char fill) {
    uint8_t buf [33];
    int n;
    if (val < 0 && base == 10) {
        n = splitInt(-val, base, buf);
        if (fill != ' ')
            putc('-');
        putFiller(width - n - 1, fill);
        if (fill == ' ')
            putc('-');
    } else {
        n = splitInt(val, base, buf);
        putFiller(width - n, fill);
    }
    while (n > 0) {
        uint8_t b = buf[--n];
        putc("0123456789ABCDEF"[b]);
    }
}

void BufferObj::printf(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    char const* s;
    while (*fmt) {
        char c = *fmt++;
        if (c == '%') {
            char fill = *fmt == '0' ? '0' : ' ';
            int width = 0, base = 0;
            while (base == 0) {
                c = *fmt++;
                switch (c) {
                    case 'b':
                        base =  2;
                        break;
                    case 'o':
                        base =  8;
                        break;
                    case 'd':
                        base = 10;
                        break;
                    case 'p':
                        fill = '0';
                        width = 8;
                        // fall through
                    case 'x':
                        base = 16;
                        break;
                    case 'c':
                        putFiller(width - 1, fill);
                        c = va_arg(ap, int);
                        // fall through
                    case '%':
                        putc(c);
                        base = 1;
                        break;
                    case 's':
                        s = va_arg(ap, char const*);
                        width -= strlen(s);
                        while (*s)
                            putc(*s++);
                        putFiller(width, fill);
                        // fall through
                    default:
                        if ('0' <= c && c <= '9')
                            width = 10 * width + c - '0';
                        else
                            base = 1; // stop scanning
                }
            }
            if (base > 1) {
                int val = va_arg(ap, int);
                putInt(val, base, width, fill);
            }
        } else
            putc(c);
    }
    va_end(ap);
}

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
    Printer (Value writer, int argc, Value argv[], const char* style ="\0  \n")
            : ResumableObj (argc, argv), w (writer.asType<BufferObj>()) {
        memcpy(fmt, style, 4);
    }

    void mark (void (*gc)(const Object&)) const override {
        ResumableObj::mark(gc);
        gc(w);
    }

    bool step (Value v) override {
        if (!w.need(100))
            return true; // TODO
        if (pos == 0 && fmt[0] != 0)
            w.putc(fmt[0]);
        if (pos >= nargs) {
            w.putc(fmt[3]);
            w.need(0); // TODO in the wrong place, but where to put it?
            return false;
        }
        if (pos > 0)
            w.putc(pos & 1 ? fmt[1] : fmt[2]);
        retVal = Context::print(w, args[pos++]);
        return true;
    }

private:
    int pos = 0;
    char fmt [4]; // prefix sepOdd, sepEven, postfix
    BufferObj& w;
};

Value Object::repr (BufferObj& w) const {
    w.printf("<Obj %s at %p>", type().name, this);
    return Value::nil;
}

static void printEscaped (BufferObj& w, const char* fmt, uint8_t ch) {
    w.putc('\\');
    switch (ch) {
        case '\t': w.putc('t'); break;
        case '\n': w.putc('n'); break;
        case '\r': w.putc('r'); break;
        default:   w.printf(fmt, ch); break;
    }
}

Value NoneObj::repr (BufferObj& w) const {
    w.printf("null"); // JSON
    return Value::nil;
}

Value BoolObj::repr (BufferObj& w) const {
    w.printf("%s", this == &falseObj ? "false" : "true"); // JSON
    return Value::nil;
}

Value BytesObj::repr (BufferObj& w) const {
    w.putc('\'');
    int n = len();
    for (auto p = (const uint8_t*) *this; --n >= 0; ++p) {
        if (*p == '\\' || *p == '\'')
            w.putc('\\');
        if (' ' <= *p && *p <= '~')
            w.putc(*p);
        else
            printEscaped(w, "x%02x", *p);
    }
    w.putc('\'');
    return Value::nil;
}

Value StrObj::repr (BufferObj& w) const {
    w.putc('"');
    for (auto p = (const uint8_t*)(const char*) *this; *p != 0; ++p) {
        if (*p == '\\' || *p == '"')
            w.putc('\\');
        if (*p >= ' ')
            w.putc(*p);
        else
            printEscaped(w, "u%04x", *p);
    }
    w.putc('"');
    return Value::nil;
}

Value TupleObj::repr (BufferObj& w) const {
    return new Printer (w, length, (Value*) vec, "(,,)"); // FIXME const!
}

Value ArrayObj::repr (BufferObj& w) const {
    w.printf("%d%c", (int) length(), atype);
    auto p = (const uint8_t*) getPtr(0);
    auto n = widthOf(length());
    for (int i = 0; i < n; ++i)
        w.printf("%02x", p[i]);
    return Value::nil;
}

Value ListObj::repr (BufferObj& w) const {
    return new Printer (w, length(), (Value*) getPtr(0), "[,,]");
}

Value SetObj::repr (BufferObj& w) const {
    return new Printer (w, length(), (Value*) getPtr(0), "{,,}");
}

Value DictObj::repr (BufferObj& w) const {
    return new Printer (w, length(), (Value*) getPtr(0), "{:,}");
}

Value ClassObj::repr (BufferObj& w) const {
    w.printf("<class %s>", (const char*) at("__name__"));
    return Value::nil;
}

Value InstanceObj::repr (BufferObj& w) const {
    w.printf("<%s object at %p>", type().name, this);
    return Value::nil;
}

Value Context::print (BufferObj& w, Value v) {
    switch (v.tag()) {
        case Value::Nil: w.printf("Nil"); break;
        case Value::Int: w.printf("%d", (int) v); break;
        case Value::Str: return v.objPtr()->repr(w); // conv for escapes
        case Value::Obj: return v.obj().repr(w); // TODO writer ...
    }
    return Value::nil;
}

//CG1 builtin print
static Value bi_print (int argc, Value argv[]) {
    static BufferObj w (Value::nil); // TODO
    return new Printer (w, argc, argv);
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
    { "suspend", &fo_suspend },
    { "tasks", &Context::tasks },
    { "modules", &Context::modules },
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
    { "slice", &SliceObj::info },
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
