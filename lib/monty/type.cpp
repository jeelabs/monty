// type.cpp - basic object types and type system

#include "monty.h"
#include "defs.h"
#include <cassert>

using namespace Monty;

extern Module const m_sys;
extern Module const m_machine;

None const None::nullObj;
Bool const Bool::falseObj;
Bool const Bool::trueObj;
Tuple const Tuple::emptyObj;

Value const Monty::Null  {None::nullObj};
Value const Monty::False {Bool::falseObj};
Value const Monty::True  {Bool::trueObj};
Value const Monty::Empty {Tuple::emptyObj};

Value::Value (char const* arg) : v (((uintptr_t) arg << 2) | 2) {
    assert((char const*) *this == arg);
}

auto Value::asObj () const -> Object& {
    switch (tag()) {
        case Value::Nil: return (Object&) None::nullObj; // drop const
        case Value::Int: return *new struct Int (*this);
        case Value::Str: return *new struct Str (*this);
        case Value::Obj: break;
    }
    return obj();
}

bool Value::truthy () const {
    switch (tag()) {
        case Value::Nil: return false;
        case Value::Int: return (int) *this != 0;
        case Value::Str: return *(char const*) *this != 0;
        case Value::Obj: return obj().unop(UnOp::Boln).isTrue();
    }
    assert(false);
}

auto Value::operator== (Value rhs) const -> bool {
    if (v == rhs.v)
        return true;
    if (tag() == rhs.tag())
        switch (tag()) {
            case Nil: assert(false); // handled above
            case Int: return false;  // handled above
            case Str: return strcmp(*this, rhs) == 0;
            case Obj: return obj().binop(BinOp::Equal, rhs).truthy();
        }
    Value lhs = *this;
    if (!isObj()) {
        lhs = rhs;
        rhs = *this;
    }
    return lhs.binOp(BinOp::Equal, rhs).truthy();
}

auto Value::operator< (Value rhs) const -> bool {
    return binOp(BinOp::Less, rhs);
}

auto Value::unOp (UnOp op) const -> Value {
    switch (tag()) {
        case Int: {
            int n = *this;
            switch (op) {
                case UnOp::Int:  // fall through
                case UnOp::Pos:  // fall through
                case UnOp::Hash: return *this;
                case UnOp::Abs:  if (n > 0) return *this; // else fall through
                case UnOp::Neg:  return -n; // TODO overflow
                case UnOp::Inv:  return ~n;
                case UnOp::Not:  return asBool(!n);
                case UnOp::Boln: return asBool(n);
            }
            break;
        }
        case Str: {
            char const* s = *this;
            switch (op) {
                case UnOp::Boln: return asBool(*s);
              //case UnOp::Hash: return BytesObj::hash((const uint8_t*) s,
              //                                                    strlen(s));
                default:         break;
            }
            break;
        }
        default: break;
    }

    return asObj().unop(op);
}

auto Value::binOp (BinOp op, Value rhs) const -> Value {
    // TODO the inverted optimisations will fail if a ResumableObj is involved
    switch (op) {
        case BinOp::More:      return rhs.binOp(BinOp::Less, *this);
        case BinOp::LessEqual: return rhs.binOp(BinOp::Less, *this).invert();
        case BinOp::MoreEqual: return binOp(BinOp::Less, rhs).invert();
        case BinOp::NotEqual:  return binOp(BinOp::Equal, rhs).invert();
        default:               break;
    }

    if (tag() == rhs.tag())
        switch (tag()) {
            case Int: {
                auto l = (int) *this, r = (int) rhs;
                switch (op) {
                    case BinOp::Less:            return asBool(l < r);
                    case BinOp::Equal:           return asBool(l == r);
                    case BinOp::Add:
                    case BinOp::InplaceAdd:      return l + r;
                    case BinOp::Subtract:
                    case BinOp::InplaceSubtract: return l - r;
                    case BinOp::Multiply:
                    case BinOp::InplaceMultiply: return l * r;
                    case BinOp::FloorDivide:
                        if (r == 0) {
                            Interp::exception("blah"); // TODO
                            return 0;
                        }
                        return l / r;
                    default: break;
                }
                break;
            }
            case Str: {
                break;
            }
            default:
                assert(isObj());
                switch (op) {
                    case BinOp::Equal:
                        if (ifType<None>() != 0) // FIXME special-cased!
                            return asBool(id() == rhs.id());
                        // fall through
                    default:
                        break;
                }
                break;
        }

    return asObj().binop(op, rhs);
}

auto Value::check (Type const& t) const -> bool {
    return isObj() && &obj().type() == &t;
}

void Value::verify (Type const& t) const {
    auto f = check(t);
    if (!f) {
        dump("verify?");
        Value v = t;
        v.dump(t.name);
    }
    assert(f);
}

auto Object::call (Vector const&, int, int) const -> Value {
    Value v = this; v.dump("call?"); assert(false);
    return {};
}

auto Object::unop (UnOp) const -> Value {
    Value v = this; v.dump("unop?"); assert(false);
    return {};
}

auto Object::binop (BinOp, Value) const -> Value {
    Value v = this; v.dump("binop?"); assert(false);
    return {};
}

auto Object::attr (char const* name, Value&) const -> Value {
    auto atab = type().chain;
    return atab != 0 ? atab->getAt(name) : getAt(name);
}

auto Object::next  () -> Value {
    Value v = this; v.dump("next?"); assert(false);
    return {};
}

auto Object::len () const -> size_t {
    Value v = this; v.dump("len?"); assert(false);
    return {};
}

auto Object::getAt (Value) const -> Value {
    Value v = this; v.dump("getAt?"); assert(false);
    return {};
}

auto Object::setAt (Value, Value) -> Value {
    Value v = this; v.dump("setAt?"); assert(false);
    return {};
}

auto None::unop (UnOp) const -> Value {
    assert(false);
    return {}; // TODO
}

auto Bool::unop (UnOp op) const -> Value {
    switch (op) {
        case UnOp::Int:  // fall through
        case UnOp::Hash: return this == &trueObj;
        case UnOp::Boln: return *this;
        default:         break;
    }
    return Object::unop(op);
}

auto Int::unop (UnOp) const -> Value {
    assert(false);
    return {}; // TODO
}

auto Int::binop (BinOp op, Value rhs) const -> Value {
    switch (op) {
        case BinOp::Equal:
            return false; // FIXME
        default:
            break;
    }
    (void) rhs; assert(false);
    return {}; // TODO
}

Bytes::Bytes (void const* ptr, size_t len) {
    insert(0, len);
    memcpy(begin(), ptr, len);
}

auto Bytes::hash (const uint8_t*, size_t) const -> uint32_t {
    // see http://www.cse.yorku.ca/~oz/hash.html
    uint32_t h = 5381;
    for (auto b : *this)
        h = ((h<<5) + h) ^ b;
    return h;
}

auto Bytes::unop (UnOp op) const -> Value {
    switch (op) {
        case UnOp::Boln: return Value::asBool(size());
        case UnOp::Hash: return hash(begin(), size());
        default:         break;
    }
    return Object::unop(op);
}

auto Bytes::binop (BinOp op, Value rhs) const -> Value {
    auto& val = rhs.asType<Bytes>();
    assert(size() == val.size());
    switch (op) {
        case BinOp::Equal:
            return Value::asBool(size() == val.size() &&
                                    memcmp(begin(), val.begin(), size()) == 0);
        default:
            break;
    }
    return Object::binop(op, rhs);
}

auto Bytes::getAt (Value k) const -> Value {
    assert(k.isInt());
    return (*this)[k];
}

Str::Str (char const* s, int n) {
    assert(n >= 0 || s != nullptr);
    if (n < 0)
        n = strlen(s);
    insert(0, n);
    adj(n+1);
    assert((int) cap() > n);
    if (s != nullptr)
        memcpy(begin(), s, n);
    else
        memset(begin(), 0, n);
    begin()[n] = 0;
}

auto Str::unop (UnOp op) const -> Value {
    switch (op) {
        case UnOp::Int:  return atoi((char const*) begin());
        default:         break;
    }
    return Bytes::unop(op);
}

auto Str::binop (BinOp op, Value rhs) const -> Value {
    auto l = (char const*) begin();
    char const* r = nullptr;
    if (rhs.isStr())
        r = rhs;
    else {
        auto o = rhs.ifType<Str>();
        if (o != nullptr)
            r = *o;
    }
    switch (op) {
        case BinOp::Equal:
            return Value::asBool(r != 0 && strcmp(l, r) == 0);
        case BinOp::Add: {
            assert(r != nullptr);
            auto nl = strlen(l), nr = strlen(r);
            auto o = new struct Str (nullptr, nl + nr);
            memcpy((char*) o->begin(), l, nl);
            memcpy((char*) o->begin() + nl, r, nr);
            return o;
        }
        default:
            break;
    }
    return Bytes::binop(op, rhs);
}

auto Str::getAt (Value k) const -> Value {
    assert(k.isInt());
    int idx = k;
    if (idx < 0)
        idx += size();
    return new Str ((char const*) begin() + idx, 1); // TODO utf-8
}

Slice::Slice (Value a, Value b, Value c) {
    assert(a.isInt() && b.isInt());
    off = a;
    num = b;
    step = c.isInt() ? (int) c : 1;
}

auto Lookup::operator[] (char const* key) const -> Value {
    for (size_t i = 0; i < count; ++i)
        if (strcmp(key, items[i].k) == 0)
            return items[i].v;
    return {};
}

auto Lookup::getAt (Value k) const -> Value {
    assert(k.isStr());
    return (*this)[k];
}

void Lookup::marker () const {
    for (size_t i = 0; i < count; ++i)
        items[i].v.marker();
}

Tuple::Tuple (size_t n, Value const* vals) : fill (n) {
    memcpy((Value*) data(), vals, n * sizeof *vals);
}

auto Tuple::getAt (Value k) const -> Value {
    assert(k.isInt());
    return data()[k];
}

void Tuple::marker () const {
    for (size_t i = 0; i < fill; ++i)
        data()[i].marker();
}

Type const Object::info ("<object>");
auto Object::type () const -> Type const& { return info; }

Type const Inst::info ("<instance>");

//CG< builtin-types lib/monty/monty.h
Type const         Cell::info ("<Cell>");
Type const    BoundMeth::info ("<boundmeth>");
Type const       Buffer::info ("<buffer>");
Type const     Bytecode::info ("<bytecode>");
Type const     Callable::info ("<callable>");
Type const      Closure::info ("<closure>");
Type const      Context::info ("<context>");
Type const     Function::info ("<function>");
Type const       Lookup::info ("<lookup>");
Type const       Method::info ("<method>");
Type const       Module::info ("<module>");
Type const         None::info ("<none>");

Type const    Array::info ("array", Array::create, &Array::attrs);
Type const     Bool::info ("bool", Bool::create, &Bool::attrs);
Type const    Bytes::info ("bytes", Bytes::create, &Bytes::attrs);
Type const    Class::info ("class", Class::create, &Class::attrs);
Type const     Dict::info ("dict", Dict::create, &Dict::attrs);
Type const      Int::info ("int", Int::create, &Int::attrs);
Type const     List::info ("list", List::create, &List::attrs);
Type const    Range::info ("range", Range::create, &Range::attrs);
Type const      Set::info ("set", Set::create, &Set::attrs);
Type const    Slice::info ("slice", Slice::create, &Slice::attrs);
Type const      Str::info ("str", Str::create, &Str::attrs);
Type const    Tuple::info ("tuple", Tuple::create, &Tuple::attrs);
Type const     Type::info ("type", Type::create, &Type::attrs);

auto         Cell::type () const -> Type const& { return info; }
auto    BoundMeth::type () const -> Type const& { return info; }
auto       Buffer::type () const -> Type const& { return info; }
auto     Bytecode::type () const -> Type const& { return info; }
auto     Callable::type () const -> Type const& { return info; }
auto      Closure::type () const -> Type const& { return info; }
auto      Context::type () const -> Type const& { return info; }
auto     Function::type () const -> Type const& { return info; }
auto       Lookup::type () const -> Type const& { return info; }
auto       Method::type () const -> Type const& { return info; }
auto       Module::type () const -> Type const& { return info; }
auto         None::type () const -> Type const& { return info; }
auto        Array::type () const -> Type const& { return info; }
auto         Bool::type () const -> Type const& { return info; }
auto        Bytes::type () const -> Type const& { return info; }
auto        Class::type () const -> Type const& { return info; }
auto         Dict::type () const -> Type const& { return info; }
auto          Int::type () const -> Type const& { return info; }
auto         List::type () const -> Type const& { return info; }
auto        Range::type () const -> Type const& { return info; }
auto          Set::type () const -> Type const& { return info; }
auto        Slice::type () const -> Type const& { return info; }
auto          Str::type () const -> Type const& { return info; }
auto        Tuple::type () const -> Type const& { return info; }
auto         Type::type () const -> Type const& { return info; }
//CG>

static auto bi_print (Vector const& vec, int argc, int args) -> Value {
    Buffer buf; // TODO
    for (int i = 0; i < argc; ++i) {
        // TODO ugly logic to avoid quotes and escapes for string args
        //  this only applies to the top level, not inside lists, etc.
        char const* s = nullptr;
        Value v = vec[args+i];
        if (v.isStr())
            s = v;
        else {
            auto p = v.ifType<Str>();
            if (p != nullptr)
                s = *p;
        }
        // if it's a plain string, print as is, else print via repr()
        if (s != nullptr) {
            if (buf.sep)
                buf.putc(' ');
            buf.puts(s);
            buf.sep = true;
        } else
            buf << v;
    }
    buf.putc('\n');
    return {};
}

static Function const f_print (bi_print);

static auto bi_next (Vector const& vec, int argc, int args) -> Value {
    assert(argc == 1 && vec[args].isObj());
    return vec[args].obj().next();
}

static Function const f_next (bi_next);

static auto bi_len (Vector const& vec, int argc, int args) -> Value {
    assert(argc == 1);
    return vec[args].asObj().len();
}

static Function const f_len (bi_len);

static auto bi_abs (Vector const& vec, int argc, int args) -> Value {
    assert(argc == 1);
    return vec[args].unOp(UnOp::Abs);
}

static Function const f_abs (bi_abs);

static auto bi_hash (Vector const& vec, int argc, int args) -> Value {
    assert(argc == 1);
    return vec[args].unOp(UnOp::Hash);
}

static Function const f_hash (bi_hash);

static const Lookup::Item builtinsMap [] = {
    //CG< builtin-emit 1
    { "array", Array::info },
    { "bool", Bool::info },
    { "bytes", Bytes::info },
    { "class", Class::info },
    { "dict", Dict::info },
    { "int", Int::info },
    { "list", List::info },
    { "range", Range::info },
    { "set", Set::info },
    { "slice", Slice::info },
    { "str", Str::info },
    { "tuple", Tuple::info },
    { "type", Type::info },
    //CG>
    { "print", f_print },
    { "next", f_next },
    { "len", f_len },
    { "abs", f_abs },
    { "hash", f_hash },
    { "sys", m_sys },
#ifndef UNIT_TEST
    { "machine", m_machine },
#endif
#if 0
#if INCLUDE_NETWORK
    { "network", m_network },
#endif
#if INCLUDE_SDCARD
    { "sdcard", m_sdcard },
#endif
#endif
};

Lookup const Monty::builtins (builtinsMap, sizeof builtinsMap);

static auto str_count (Vector const&, int, int) -> Value {
    return 9; // TODO, hardcoded for features.py
}

static Function const f_str_count (str_count);

static auto str_format (Vector const&, int, int) -> Value {
    return 4; // TODO, hardcoded for features.py
}

static Function const f_str_format (str_format);

static const Lookup::Item strMap [] = {
    { "count", &f_str_count },
    { "format", &f_str_format },
};

const Lookup Str::attrs (strMap, sizeof strMap);

#if 0
static auto list_append (Vector const& vec, int argc, int args) -> Value {
    assert(argc == 2);
    auto& l = vec[args].asType<List>();
    l.append(vec[args+1]);
    return {};
}

static Function const f_list_append (list_append);

static const Lookup::Item listMap [] = {
    { "append", &f_list_append },
};
#else
// TODO this method wrapper adds 168 bytes on STM32, but is it a one-time cost?
static auto d_list_append = Method::wrap(&List::append);
static Method const m_list_append (d_list_append);

static const Lookup::Item listMap [] = {
    { "append", &m_list_append },
};
#endif

const Lookup List::attrs (listMap, sizeof listMap);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// TODO added to satisfy linker

Lookup const     Bool::attrs {nullptr, 0};
Lookup const      Int::attrs {nullptr, 0};
Lookup const    Bytes::attrs {nullptr, 0};
Lookup const    Range::attrs {nullptr, 0};
Lookup const    Slice::attrs {nullptr, 0};
Lookup const    Tuple::attrs {nullptr, 0};
Lookup const    Array::attrs {nullptr, 0};
Lookup const      Set::attrs {nullptr, 0};
Lookup const     Dict::attrs {nullptr, 0};
Lookup const     Type::attrs {nullptr, 0};
Lookup const    Class::attrs {nullptr, 0};
Lookup const     Inst::attrs {nullptr, 0};

auto Bool::create (Vector const& vec, int argc, int args, Type const*) -> Value {
    if (argc == 1)
        return vec[args].unOp(UnOp::Boln);
    assert(argc == 0);
    return False;
}

auto Int::create (Vector const& vec, int argc, int args, Type const*) -> Value {
    assert(argc == 1);
    auto v = vec[args];
    switch (v.tag()) {
        case Value::Nil: assert(false); break;
        case Value::Int: return v;
        case Value::Str: return atoi(v);
        case Value::Obj: return v.unOp(UnOp::Int);
    }
    return {};
}

auto Bytes::create (Vector const& vec, int argc, int args, Type const*) -> Value {
    assert(argc == 1);
    Value v = vec[args];
    if (v.isInt()) {
        auto o = new Bytes ();
        o->insert(0, v);
        return o;
    }
    const void* p = 0;
    size_t n = 0;
    if (v.isStr()) {
        p = (char const*) v;
        n = strlen((char const*) p);
    } else {
        auto ps = v.ifType<Str>();
        auto pb = v.ifType<Bytes>();
        if (ps != 0) {
            p = (char const*) *ps;
            n = strlen((char const*) p); // TODO
        } else if (pb != 0) {
            p = pb->begin();
            n = pb->size();
        } else
            assert(false); // TODO iterables
    }
    return new Bytes (p, n);
}

auto Str::create (Vector const& vec, int argc, int args, Type const*) -> Value {
    assert(argc == 1 && vec[args].isStr());
    return new Str (vec[args]);
}

auto Range::create (Vector const&, int, int, Type const*) -> Value {
    assert(false);
    return {}; // TODO
}

auto Slice::create (Vector const& vec, int argc, int args, Type const*) -> Value {
    assert(1 <= argc && argc <= 3);
    Value a = argc > 1 ? vec[args] : Value (0);
    Value b = argc == 1 ? vec[args] : vec[args+1];
    Value c = argc > 2 ? vec[args+2] : b;
    return new Slice (a, b, c);
}

auto Tuple::create (Vector const& vec, int argc, int args, Type const*) -> Value {
    if (argc == 0)
        return Empty; // there's one unique empty tuple
    return new (argc * sizeof (Value)) Tuple (argc, vec.begin() + args);
}

auto Array::create (Vector const& vec, int argc, int args, Type const*) -> Value {
    assert(argc >= 1 && vec[args].isStr());
    char type = *((char const*) vec[args]);
    size_t len = 0;
    if (argc == 2) {
        assert(vec[args+1].isInt());
        len = vec[args+1];
    }
    return new Array (type, len);
}

auto List::create (Vector const& vec, int argc, int args, Type const*) -> Value {
    return new List (vec, argc, args);
}

auto Set::create (Vector const& vec, int argc, int args, Type const*) -> Value {
    return new Set (vec, argc, args);
}

auto Dict::create (Vector const&, int, int, Type const*) -> Value {
    // TODO
    return new Dict;
}

auto Type::create (Vector const& vec, int argc, int args, Type const*) -> Value {
    assert(argc == 1);
    Value v = vec[args];
    switch (v.tag()) {
        case Value::Nil: assert(false); break;
        case Value::Int: return "int";
        case Value::Str: return "str";
        case Value::Obj: return v.obj().type().name;
    }
    return {};
}

auto Class::create (Vector const& vec, int argc, int args, Type const*) -> Value {
    assert(argc == 2 && vec[args].isObj() && vec[args+1].isStr());
    return new Class (vec, argc, args);
}

auto Inst::create (Vector const& vec, int argc, int args, Type const* t) -> Value {
    Value v = t;
    return new Inst (vec, argc, args, v.asType<Class>());
}
