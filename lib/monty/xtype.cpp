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

bool Value::truthy () const {
    switch (tag()) {
        case Value::Nil: return false;
        case Value::Int: return (int) *this != 0;
        case Value::Str: return *(const char*) *this != 0;
        case Value::Obj: return obj().unop(UnOp::Bool).isTrue();
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
                case UnOp::Bool: return asBool(n);
            }
            break;
        }
        case Str: {
            const char* s = *this;
            switch (op) {
                case UnOp::Bool: return asBool(*s);
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

auto Object::repr (Printer&) const -> Value {
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

auto None::unop (UnOp) const -> Value {
    assert(false);
    return {}; // TODO
}

auto Bool::unop (UnOp) const -> Value {
    assert(false);
    return {}; // TODO
}

auto Fixed::unop (UnOp) const -> Value {
    assert(false);
    return {}; // TODO
}

auto Fixed::binop (BinOp, Value) const -> Value {
    assert(false);
    return {}; // TODO
}

Slice::Slice (Value a, Value b, Value c) {
    assert(a.isInt() && b.isInt());
    off = a;
    num = b;
    step = c.isInt() ? (int) c : 1;
}

auto Lookup::operator[] (char const* key) -> Value {
    for (size_t i = 0; i < count; ++i)
        if (strcmp(key, items[i].k) == 0)
            return items[i].v;
    return {};
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
Type const     Callable::info ("<callable>");
Type const      Context::info ("<context>");
Type const     Function::info ("<function>");
Type const       Lookup::info ("<lookup>");
Type const       Module::info ("<module>");
Type const         None::info ("<none>");
Type const      Printer::info ("<printer>");

Type const    Array::info ("array", Array::create, &Array::attrs);
Type const     Bool::info ("bool", Bool::create, &Bool::attrs);
Type const    Bytes::info ("bytes", Bytes::create, &Bytes::attrs);
Type const    Class::info ("class", Class::create, &Class::attrs);
Type const     Dict::info ("dict", Dict::create, &Dict::attrs);
Type const    Fixed::info ("int", Fixed::create, &Fixed::attrs);
Type const     List::info ("list", List::create, &List::attrs);
Type const      Set::info ("set", Set::create, &Set::attrs);
Type const    Slice::info ("slice", Slice::create, &Slice::attrs);
Type const      Str::info ("str", Str::create, &Str::attrs);
Type const    Tuple::info ("tuple", Tuple::create, &Tuple::attrs);
Type const     Type::info ("type", Type::create, &Type::attrs);

auto    BoundMeth::type () const -> Type const& { return info; }
auto     Callable::type () const -> Type const& { return info; }
auto      Context::type () const -> Type const& { return info; }
auto     Function::type () const -> Type const& { return info; }
auto       Lookup::type () const -> Type const& { return info; }
auto       Module::type () const -> Type const& { return info; }
auto         None::type () const -> Type const& { return info; }
auto      Printer::type () const -> Type const& { return info; }
auto        Array::type () const -> Type const& { return info; }
auto         Bool::type () const -> Type const& { return info; }
auto        Bytes::type () const -> Type const& { return info; }
auto        Class::type () const -> Type const& { return info; }
auto         Dict::type () const -> Type const& { return info; }
auto        Fixed::type () const -> Type const& { return info; }
auto         List::type () const -> Type const& { return info; }
auto          Set::type () const -> Type const& { return info; }
auto        Slice::type () const -> Type const& { return info; }
auto          Str::type () const -> Type const& { return info; }
auto        Tuple::type () const -> Type const& { return info; }
auto         Type::type () const -> Type const& { return info; }
//CG>

static const Lookup::Item builtins [] = {
    //CG< builtin-emit 1
    { "array", &Array::info },
    { "bool", &Bool::info },
    { "bytes", &Bytes::info },
    { "class", &Class::info },
    { "dict", &Dict::info },
    { "int", &Fixed::info },
    { "list", &List::info },
    { "set", &Set::info },
    { "slice", &Slice::info },
    { "str", &Str::info },
    { "tuple", &Tuple::info },
    { "type", &Type::info },
    //CG>
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

Lookup const builtinDict (builtins, sizeof builtins / sizeof *builtins);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// TODO added to satisfy linker

Lookup const     Bool::attrs {nullptr, 0};
Lookup const    Fixed::attrs {nullptr, 0};
Lookup const    Bytes::attrs {nullptr, 0};
Lookup const      Str::attrs {nullptr, 0};
Lookup const     Type::attrs {nullptr, 0};
Lookup const    Array::attrs {nullptr, 0};
Lookup const    Tuple::attrs {nullptr, 0};
Lookup const     List::attrs {nullptr, 0};
Lookup const      Set::attrs {nullptr, 0};
Lookup const    Slice::attrs {nullptr, 0};
Lookup const     Dict::attrs {nullptr, 0};
Lookup const    Class::attrs {nullptr, 0};
Lookup const     Inst::attrs {nullptr, 0};

auto Bool::create (CofV const& args, Type const*) -> Value {
    auto n = args.length();
    if (n == 1)
        return args[0].unOp(UnOp::Bool);
    assert(n == 0);
    return False;
}

auto Fixed::create (CofV const& args, Type const*) -> Value {
    assert(args.length() == 1);
    switch (args[0].tag()) {
        case Value::Nil: assert(false); break;
        case Value::Int: return args[0];
        case Value::Str: return atoi(args[0]);
        case Value::Obj: return args[0].unOp(UnOp::Int);
    }
    return {};
}

auto Bytes::create (CofV const& args, Type const*) -> Value {
    assert(false);
    return {}; // TODO
}

auto Str::create (CofV const& args, Type const*) -> Value {
    assert(args.length() == 1 && args[0].isStr());
    return new Str (args[0]);
}

auto Slice::create (CofV const& args, Type const*) -> Value {
    auto n = args.length();
    assert(1 <= n && n <= 3);
    Value a = n > 1 ? args[0] : Value (0);
    Value b = n == 1 ? args[0] : args[1];
    Value c = n > 2 ? args[2] : b;
    return new Slice (a, b, c);
}

auto Type::create (CofV const& args, Type const*) -> Value {
    assert(false);
    return {}; // TODO
}

auto Array::create (CofV const& args, Type const*) -> Value {
    // TODO
    return new Array ('B');
}

auto Tuple::create (CofV const& args, Type const*) -> Value {
    auto n = args.length();
    if (n == 0)
        return Empty; // there's one unique empty tuple
    return new (n * sizeof (Value)) Tuple (n, args.begin());
}

auto List::create (CofV const& args, Type const*) -> Value {
    // TODO
    return new List;
}

auto Set::create (CofV const& args, Type const*) -> Value {
    // TODO
    return new Set;
}

auto Dict::create (CofV const& args, Type const*) -> Value {
    // TODO
    return new Dict;
}

auto Class::create (CofV const& args, Type const*) -> Value {
    // TODO
    return new Class;
}

auto Inst::create (CofV const& args, Type const*) -> Value {
    // TODO
    return new Inst;
}
