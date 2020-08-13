// type.cpp - basic object types and type system

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "xmonty.h"

namespace Monty { // TODO move into defs.h
#include "defs.h"
}

using namespace Monty;

None const None::nullObj;
Bool const Bool::falseObj;
Bool const Bool::trueObj;
Tuple const Tuple::emptyObj;

Value const Monty::Null  {None::nullObj};
Value const Monty::False {Bool::falseObj};
Value const Monty::True  {Bool::trueObj};
Value const Monty::Empty {Tuple::emptyObj};

Value::Value (int arg) : v ((arg << 1) | 1) {
    if ((int) *this != arg)
        *this = new Fixed (arg);
}

Value::Value (char const* arg) : v (((uintptr_t) arg << 2) | 2) {
    assert((char const*) *this == arg);
}

auto Value::asObj () -> Object& {
    switch (tag()) {
        case Value::Nil: return (Object&) None::nullObj; // drop const
        case Value::Int: *this = new Fixed (*this); break;
        case Value::Str: *this = new struct Str (*this); break;
        case Value::Obj: break;
    }
    return obj();
}

bool Value::truthy () const {
    switch (tag()) {
        case Value::Nil: return false;
        case Value::Int: return (int) *this != 0;
        case Value::Str: return *(const char*) *this != 0;
        case Value::Obj: return obj().unop(UnOp::Boln).isTrue();
    }
    assert(false);
}

auto Value::operator== (Value rhs) const -> bool {
    if (v == rhs.v)
        return true;
#if 0 // TODO redundant?
    if (tag() == rhs.tag())
        switch (tag()) {
            case Nil: assert(false); // handled above
            case Int: return false;  // handled above
            case Str: return strcmp(*this, rhs) == 0;
            case Obj: return obj().binop(BinOp::Equal, rhs);
#endif
    // TODO return binOp(BinOp::Equal, rhs);
    return false;
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
            const char* s = *this;
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
    // TODO return objPtr()->unop(op);
    assert(false);
    return {}; // TODO
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
                            // TODO Context::raise("blah"); // TODO
                            return 0;
                        }
                        return l / r;
                    default: break;
                }
                break;
            }
            case Str: {
                auto l = (const char*) *this, r = (const char*) rhs;
                switch (op) {
                    case BinOp::Add: {
                        auto buf = (char*) malloc(strlen(l) + strlen(r) + 1);
                        strcpy(buf, l);
                        strcat(buf, r);
                        return buf;
                    }
                    default:
                        break;
                }
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
    // TODO return objPtr()->binop(op, rhs);
    assert(false);
    return {}; // TODO
}

auto Value::check (Type const& t) const -> bool {
    return isObj() && &obj().type() == &t;
}

void Value::verify (Type const& t) const {
    assert(check(t));
}

auto Object::call (Context&, int, int) const -> Value {
    assert(false);
    return {};
}

auto Object::unop (UnOp) const -> Value {
    assert(false);
    return {};
}

auto Object::binop (BinOp, Value) const -> Value {
    assert(false);
    return {};
}

auto Object::attr (const char* name, Value&) const -> Value {
    auto atab = type().chain;
    return atab != 0 ? atab->getAt(name) : getAt(name);
}

auto Object::getAt (Value) const -> Value {
    assert(false);
    return {};
}

auto Object::setAt (Value k, Value v) -> Value {
    assert(false);
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

auto Fixed::unop (UnOp) const -> Value {
    assert(false);
    return {}; // TODO
}

auto Fixed::binop (BinOp, Value) const -> Value {
    assert(false);
    return {}; // TODO
}

auto Str::getAt (Value k) const -> Value {
    assert(k.isInt());
    int idx = k;
    if (idx < 0)
        idx += strlen(ptr);
    auto buf = (char*) malloc(2); // TODO no malloc, please
    buf[0] = ptr[idx];
    buf[1] = 0;
    return buf;
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
        mark(items[i].v);
}

Type const Object::info ("<object>");
auto Object::type () const -> Type const& { return info; }

Type const Inst::info ("<instance>");
auto Inst::type () const -> Type const& { return info; }

//CG< builtin-types lib/monty/xmonty.h
Type const    BoundMeth::info ("<boundmeth>");
Type const       Buffer::info ("<buffer>");
Type const     Callable::info ("<callable>");
Type const      Context::info ("<context>");
Type const     Function::info ("<function>");
Type const         Iter::info ("<iterator>");
Type const       Lookup::info ("<lookup>");
Type const       Module::info ("<module>");
Type const         None::info ("<none>");

Type const     Bool::info ("bool", Bool::create, &Bool::attrs);
Type const    Bytes::info ("bytes", Bytes::create, &Bytes::attrs);
Type const    Class::info ("class", Class::create, &Class::attrs);
Type const     Dict::info ("dict", Dict::create, &Dict::attrs);
Type const    Fixed::info ("int", Fixed::create, &Fixed::attrs);
Type const     List::info ("list", List::create, &List::attrs);
Type const    Range::info ("range", Range::create, &Range::attrs);
Type const      Set::info ("set", Set::create, &Set::attrs);
Type const    Slice::info ("slice", Slice::create, &Slice::attrs);
Type const      Str::info ("str", Str::create, &Str::attrs);
Type const    Tuple::info ("tuple", Tuple::create, &Tuple::attrs);
Type const     Type::info ("type", Type::create, &Type::attrs);

auto    BoundMeth::type () const -> Type const& { return info; }
auto       Buffer::type () const -> Type const& { return info; }
auto     Callable::type () const -> Type const& { return info; }
auto      Context::type () const -> Type const& { return info; }
auto     Function::type () const -> Type const& { return info; }
auto         Iter::type () const -> Type const& { return info; }
auto       Lookup::type () const -> Type const& { return info; }
auto       Module::type () const -> Type const& { return info; }
auto         None::type () const -> Type const& { return info; }
auto         Bool::type () const -> Type const& { return info; }
auto        Bytes::type () const -> Type const& { return info; }
auto        Class::type () const -> Type const& { return info; }
auto         Dict::type () const -> Type const& { return info; }
auto        Fixed::type () const -> Type const& { return info; }
auto         List::type () const -> Type const& { return info; }
auto        Range::type () const -> Type const& { return info; }
auto          Set::type () const -> Type const& { return info; }
auto        Slice::type () const -> Type const& { return info; }
auto          Str::type () const -> Type const& { return info; }
auto        Tuple::type () const -> Type const& { return info; }
auto         Type::type () const -> Type const& { return info; }
//CG>

static auto bi_print (Context& ctx, int argc, int args) -> Value {
    {
        Buffer buf;
        for (int i = 0; i < argc; ++i)
            buf << ctx[args+i];
    }
    printf("\n");
    return {};
}

static Function const f_print (bi_print);

static const Lookup::Item builtinsMap [] = {
    //CG< builtin-emit 1
    { "bool", &Bool::info },
    { "bytes", &Bytes::info },
    { "class", &Class::info },
    { "dict", &Dict::info },
    { "int", &Fixed::info },
    { "list", &List::info },
    { "range", &Range::info },
    { "set", &Set::info },
    { "slice", &Slice::info },
    { "str", &Str::info },
    { "tuple", &Tuple::info },
    { "type", &Type::info },
    //CG>
    { "print", &f_print },
#if 0
    { "monty", &m_monty },
    { "machine", &m_machine },
#if INCLUDE_NETWORK
    { "network", &m_network },
#endif
#if INCLUDE_SDCARD
    { "sdcard", &m_sdcard },
#endif
#endif
};

Lookup const Monty::builtins (builtinsMap, sizeof builtinsMap);

static auto str_count (Context& ctx, int argc, int args) -> Value {
    return 9; // TODO, hardcoded for features.py
}

static Function const f_str_count (str_count);

static auto str_format (Context& ctx, int argc, int args) -> Value {
    return 4; // TODO, hardcoded for features.py
}

static Function const f_str_format (str_format);

static const Lookup::Item strMap [] = {
    { "count", &f_str_count },
    { "format", &f_str_format },
};

const Lookup Str::attrs (strMap, sizeof strMap);

static auto list_append (Context& ctx, int argc, int args) -> Value {
    assert(argc == 2);
    auto& l = ctx[args].asType<List>();
    auto n = l.size();
    l.insert(n);
    l[n] = ctx[args+1];
    return {};
}

static Function const f_list_append (list_append);

static const Lookup::Item listMap [] = {
    { "append", &f_list_append },
};

const Lookup List::attrs (listMap, sizeof listMap);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// TODO added to satisfy linker

Lookup const     Bool::attrs {nullptr, 0};
Lookup const    Fixed::attrs {nullptr, 0};
Lookup const    Bytes::attrs {nullptr, 0};
Lookup const    Range::attrs {nullptr, 0};
Lookup const    Slice::attrs {nullptr, 0};
Lookup const    Tuple::attrs {nullptr, 0};
Lookup const      Set::attrs {nullptr, 0};
Lookup const     Dict::attrs {nullptr, 0};
Lookup const     Type::attrs {nullptr, 0};
Lookup const    Class::attrs {nullptr, 0};
Lookup const     Inst::attrs {nullptr, 0};

auto Bool::create (Chunk const& args, Type const*) -> Value {
    auto n = args.size();
    if (n == 1)
        return args[0].unOp(UnOp::Boln);
    assert(n == 0);
    return False;
}

auto Fixed::create (Chunk const& args, Type const*) -> Value {
    assert(args.size() == 1);
    switch (args[0].tag()) {
        case Value::Nil: assert(false); break;
        case Value::Int: return args[0];
        case Value::Str: return atoi(args[0]);
        case Value::Obj: return args[0].unOp(UnOp::Int);
    }
    return {};
}

auto Bytes::create (Chunk const& args, Type const*) -> Value {
    assert(false);
    return {}; // TODO
}

auto Str::create (Chunk const& args, Type const*) -> Value {
    assert(args.size() == 1 && args[0].isStr());
    return new Str (args[0]);
}

auto Range::create (Chunk const& args, Type const*) -> Value {
    assert(false);
    return {}; // TODO
}

auto Slice::create (Chunk const& args, Type const*) -> Value {
    auto n = args.size();
    assert(1 <= n && n <= 3);
    Value a = n > 1 ? args[0] : Value (0);
    Value b = n == 1 ? args[0] : args[1];
    Value c = n > 2 ? args[2] : b;
    return new Slice (a, b, c);
}

auto Tuple::create (Chunk const& args, Type const*) -> Value {
    auto n = args.size();
    if (n == 0)
        return Empty; // there's one unique empty tuple
    return new (n * sizeof (Value)) Tuple (n, args.begin());
}

auto List::create (Chunk const& args, Type const*) -> Value {
    // TODO
    return new List;
}

auto Set::create (Chunk const& args, Type const*) -> Value {
    // TODO
    return new Set;
}

auto Dict::create (Chunk const& args, Type const*) -> Value {
    // TODO
    return new Dict;
}

auto Type::create (Chunk const& args, Type const*) -> Value {
    assert(false);
    return {}; // TODO
}

auto Class::create (Chunk const& args, Type const*) -> Value {
    // TODO
    return new Class;
}

auto Inst::create (Chunk const& args, Type const*) -> Value {
    // TODO
    return new Inst;
}
