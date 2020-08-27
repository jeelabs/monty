// type.cpp - basic object types and type system

#include "monty.h"
#include "ops.h"
#include <cassert>

using namespace Monty;
using Monty::Q;

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

constexpr int QID_RAM_BASE = 10000;
constexpr int QID_RAM_LAST = 20000;

static VaryVec qstrBaseMap (qstrBase, qstrBaseLen);
static VaryVec qstrRamMap;

auto Q::hash (void const* p, size_t n) -> uint32_t {
    // see http://www.cse.yorku.ca/~oz/hash.html
    uint32_t h = 5381;
    for (size_t i = 0; i < n; ++i)
        h = ((h<<5) + h) ^ ((uint8_t const*) p)[i];
    return h;
}

auto Q::str (uint16_t i) -> char const* {
    assert(i != 0);
    auto s = i < QID_RAM_BASE ? qstrBaseMap.atGet(i)
                               : qstrRamMap.atGet(i - QID_RAM_BASE);
    return (char const*) s;
}

static auto qstrFind (VaryVec const& v, char const* s, uint8_t h) -> uint16_t {
    if (v.size() > 0) {
        auto p = v.atGet(0);
        auto n = v.atLen(0);
        for (size_t i = 0; i < n; ++i)
            if (h == p[i] && strcmp(s, (char const*) v.atGet(i+1)) == 0)
                return i+1;
    }
    return 0;
}

auto Q::find (char const* s) -> uint16_t {
    uint8_t h = hash(s, strlen(s));
    auto i = qstrFind(qstrBaseMap, s, h);
    if (i > 0)
        return i;
    auto j = qstrFind(qstrRamMap, s, h);
    if (j > 0)
        return j + QID_RAM_BASE;
    return 0;
}

auto Q::last () -> uint16_t {
    return qstrRamMap.size() - 1 + QID_RAM_BASE;
}

auto Q::make (char const* s) -> uint16_t {
    auto i = find(s);
    if (i > 0)
        return i;
    auto n = strlen(s);
    auto& v = qstrRamMap; // shorthand
    if (v.size() == 0)
        v.insert(0); // empty hash map
    i = v.atLen(0);
    v.atAdj(0, i + 1);
    v.atGet(0)[i++] = hash(s, n);
    v.insert(i);
    v.atSet(i, s, n+1);
    return i + QID_RAM_BASE;
}

Value::Value (char const* arg) : v ((uintptr_t) arg * 4 + 2) {
    if (Vec::inPool(arg)) // don't store pointers into movable vector space
#if 0
        *this = new struct Str (arg); // TODO should try Q::find first
#else
        v = Q::make(arg) * 4 + 2;
#endif
    else
        assert((char const*) *this == arg); // watch out for address truncation
}

Value::operator char const* () const {
    if (!isStr())
        return asType<struct Str>();
    auto p = v >> 2;
    if (p < QID_RAM_LAST)
        return Q::str(p);
    return (char const*) p;
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
                case UnOp::Hash: return Q::hash(s, strlen(s));
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
        case BinOp::In:        return rhs.binOp(BinOp::Contains, *this);
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

void VaryVec::atAdj (size_t idx, size_t len) {
    assert(idx < fill);
    auto olen = atLen(idx);
    if (len == olen)
        return;
    auto ofill = fill;
    fill = pos(fill);
    if (len > olen)
        ByteVec::insert(pos(idx+1), len - olen);
    else
        ByteVec::remove(pos(idx) + len, olen - len);
    fill = ofill;

    for (size_t i = idx + 1; i <= fill; ++i)
        pos(i) += len - olen;
}

void VaryVec::atSet (size_t idx, void const* ptr, size_t len) {
    atAdj(idx, len);
    memcpy(begin() + pos(idx), ptr, len);
}

void VaryVec::insert (size_t idx, size_t num) {
    assert(idx <= fill);
    if (cap() == 0) {
        ByteVec::insert(0, 2);
        pos(0) = 2;
        fill = 0;
    }

    auto ofill = fill;
    fill = pos(fill);
    ByteVec::insert(2 * idx, 2 * num);
    fill = ofill + num;

    for (size_t i = 0; i <= fill; ++i)
        pos(i) += 2 * num;
    for (size_t i = 0; i < num; ++i)
        pos(idx+i) = pos(idx+num);
}

void VaryVec::remove (size_t idx, size_t num) {
    assert(idx + num <= fill);
    auto diff = pos(idx+num) - pos(idx);

    auto ofill = fill;
    fill = pos(fill);
    ByteVec::remove(pos(idx), diff);
    ByteVec::remove(2 * idx, 2 * num);
    fill = ofill - num;

    for (size_t i = 0; i <= fill; ++i)
        pos(i) -= 2 * num;
    for (size_t i = idx; i <= fill; ++i)
        pos(i) -= diff;
}

auto Object::call (ArgVec const&) const -> Value {
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

auto Bytes::unop (UnOp op) const -> Value {
    switch (op) {
        case UnOp::Boln: return Value::asBool(size());
        case UnOp::Hash: return Q::hash(begin(), size());
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

Range::Range (Value a, Value b, Value c) {
    assert(a.isInt() && b.isInt());
    start = a;
    limit = b;
    step = c.isInt() ? (int) c : 1;
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

Tuple::Tuple (ArgVec const& args) : fill (args.num) {
    memcpy((Value*) data(), args.begin(), args.num * sizeof (Value));
}

auto Tuple::getAt (Value k) const -> Value {
    assert(k.isInt());
    return data()[k];
}

void Tuple::marker () const {
    for (size_t i = 0; i < fill; ++i)
        data()[i].marker();
}

Exception::Exception (int exc, ArgVec const& args) : Tuple (args) {
    extra().code = exc;
}

auto Exception::binop (BinOp op, Value rhs) const -> Value {
    if (op == BinOp::ExceptionMatch)
        return True; // TODO
    return Tuple::binop(op, rhs);
}

void Exception::marker () const {
    Tuple::marker();
    // TODO Extra fields
}

//CG: exception BaseException
//CG: exception Exception BaseException
//CG: exception StopIteration Exception
//CG: exception ArithmeticError Exception
//CG: exception AssertionError Exception
//CG: exception AttributeError Exception
//CG: exception EOFError Exception
//CG: exception ImportError Exception
//CG: exception LookupError Exception
//CG: exception IndexError LookupError
//CG: exception KeyError LookupError
//CG: exception MemoryError Exception
//CG: exception NameError Exception
//CG: exception OSError Exception
//CG: exception RuntimeError Exception
//CG: exception NotImplementedError RuntimeError
//CG: exception TypeError Exception
//CG: exception ValueError Exception
//CG: exception UnicodeError ValueError

static Lookup::Item const exceptionMap [] = {
    //CG< exception-emit h
    { Q( 33,"BaseException")       , -1 }, //  0 -> 
    { Q( 36,"Exception")           ,  0 }, //  1 -> BaseException
    { Q( 51,"StopIteration")       ,  1 }, //  2 -> Exception
    { Q( 30,"ArithmeticError")     ,  1 }, //  3 -> Exception
    { Q( 31,"AssertionError")      ,  1 }, //  4 -> Exception
    { Q( 32,"AttributeError")      ,  1 }, //  5 -> Exception
    { Q( 34,"EOFError")            ,  1 }, //  6 -> Exception
    { Q( 38,"ImportError")         ,  1 }, //  7 -> Exception
    { Q( 43,"LookupError")         ,  1 }, //  8 -> Exception
    { Q( 40,"IndexError")          ,  8 }, //  9 -> LookupError
    { Q( 41,"KeyError")            ,  8 }, // 10 -> LookupError
    { Q( 44,"MemoryError")         ,  1 }, // 11 -> Exception
    { Q( 45,"NameError")           ,  1 }, // 12 -> Exception
    { Q( 48,"OSError")             ,  1 }, // 13 -> Exception
    { Q( 50,"RuntimeError")        ,  1 }, // 14 -> Exception
    { Q( 47,"NotImplementedError") , 14 }, // 15 -> RuntimeError
    { Q( 54,"TypeError")           ,  1 }, // 16 -> Exception
    { Q( 55,"ValueError")          ,  1 }, // 17 -> Exception
    { Q(175,"UnicodeError")        , 17 }, // 18 -> ValueError
    //CG>
};

Lookup const Exception::bases (exceptionMap, sizeof exceptionMap);

//CG< exception-emit f
static auto e_BaseException (ArgVec const& args) -> Value {
    return Exception::create(0, args);
}
static Function const f_BaseException (e_BaseException);

static auto e_Exception (ArgVec const& args) -> Value {
    return Exception::create(1, args);
}
static Function const f_Exception (e_Exception);

static auto e_StopIteration (ArgVec const& args) -> Value {
    return Exception::create(2, args);
}
static Function const f_StopIteration (e_StopIteration);

static auto e_ArithmeticError (ArgVec const& args) -> Value {
    return Exception::create(3, args);
}
static Function const f_ArithmeticError (e_ArithmeticError);

static auto e_AssertionError (ArgVec const& args) -> Value {
    return Exception::create(4, args);
}
static Function const f_AssertionError (e_AssertionError);

static auto e_AttributeError (ArgVec const& args) -> Value {
    return Exception::create(5, args);
}
static Function const f_AttributeError (e_AttributeError);

static auto e_EOFError (ArgVec const& args) -> Value {
    return Exception::create(6, args);
}
static Function const f_EOFError (e_EOFError);

static auto e_ImportError (ArgVec const& args) -> Value {
    return Exception::create(7, args);
}
static Function const f_ImportError (e_ImportError);

static auto e_LookupError (ArgVec const& args) -> Value {
    return Exception::create(8, args);
}
static Function const f_LookupError (e_LookupError);

static auto e_IndexError (ArgVec const& args) -> Value {
    return Exception::create(9, args);
}
static Function const f_IndexError (e_IndexError);

static auto e_KeyError (ArgVec const& args) -> Value {
    return Exception::create(10, args);
}
static Function const f_KeyError (e_KeyError);

static auto e_MemoryError (ArgVec const& args) -> Value {
    return Exception::create(11, args);
}
static Function const f_MemoryError (e_MemoryError);

static auto e_NameError (ArgVec const& args) -> Value {
    return Exception::create(12, args);
}
static Function const f_NameError (e_NameError);

static auto e_OSError (ArgVec const& args) -> Value {
    return Exception::create(13, args);
}
static Function const f_OSError (e_OSError);

static auto e_RuntimeError (ArgVec const& args) -> Value {
    return Exception::create(14, args);
}
static Function const f_RuntimeError (e_RuntimeError);

static auto e_NotImplementedError (ArgVec const& args) -> Value {
    return Exception::create(15, args);
}
static Function const f_NotImplementedError (e_NotImplementedError);

static auto e_TypeError (ArgVec const& args) -> Value {
    return Exception::create(16, args);
}
static Function const f_TypeError (e_TypeError);

static auto e_ValueError (ArgVec const& args) -> Value {
    return Exception::create(17, args);
}
static Function const f_ValueError (e_ValueError);

static auto e_UnicodeError (ArgVec const& args) -> Value {
    return Exception::create(18, args);
}
static Function const f_UnicodeError (e_UnicodeError);
//CG>

Type const Object::info (Q(191,"<object>"));
auto Object::type () const -> Type const& { return info; }

Type const Inst::info (Q(192,"<instance>"));

//CG< builtin-types lib/monty/monty.h
Type const    BoundMeth::info (Q(176,"<boundmeth>"));
Type const       Buffer::info (Q(177,"<buffer>"));
Type const     Bytecode::info (Q(178,"<bytecode>"));
Type const     Callable::info (Q(179,"<callable>"));
Type const         Cell::info (Q(180,"<cell>"));
Type const      Closure::info (Q(181,"<closure>"));
Type const      Context::info (Q(182,"<context>"));
Type const    Exception::info (Q(183,"<exception>"));
Type const     Function::info (Q(184,"<function>"));
Type const       Lookup::info (Q(185,"<lookup>"));
Type const       Method::info (Q(186,"<method>"));
Type const       Module::info (Q(  7,"<module>"));
Type const         None::info (Q(187,"<none>"));

Type const    Array::info (Q(188,"array") ,  Array::create, &Array::attrs);
Type const     Bool::info (Q( 62,"bool")  ,   Bool::create, &Bool::attrs);
Type const    Bytes::info (Q( 66,"bytes") ,  Bytes::create, &Bytes::attrs);
Type const    Class::info (Q(189,"class") ,  Class::create, &Class::attrs);
Type const     Dict::info (Q( 75,"dict")  ,   Dict::create, &Dict::attrs);
Type const      Int::info (Q( 94,"int")   ,    Int::create, &Int::attrs);
Type const     List::info (Q(108,"list")  ,   List::create, &List::attrs);
Type const    Range::info (Q(124,"range") ,  Range::create, &Range::attrs);
Type const      Set::info (Q(140,"set")   ,    Set::create, &Set::attrs);
Type const    Slice::info (Q(190,"slice") ,  Slice::create, &Slice::attrs);
Type const      Str::info (Q(151,"str")   ,    Str::create, &Str::attrs);
Type const    Tuple::info (Q(157,"tuple") ,  Tuple::create, &Tuple::attrs);
Type const     Type::info (Q(158,"type")  ,   Type::create, &Type::attrs);

auto    BoundMeth::type () const -> Type const& { return info; }
auto       Buffer::type () const -> Type const& { return info; }
auto     Bytecode::type () const -> Type const& { return info; }
auto     Callable::type () const -> Type const& { return info; }
auto         Cell::type () const -> Type const& { return info; }
auto      Closure::type () const -> Type const& { return info; }
auto      Context::type () const -> Type const& { return info; }
auto    Exception::type () const -> Type const& { return info; }
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

static auto bi_print (ArgVec const& args) -> Value {
    Buffer buf; // TODO
    for (int i = 0; i < args.num; ++i) {
        // TODO ugly logic to avoid quotes and escapes for string args
        //  this only applies to the top level, not inside lists, etc.
        char const* s = nullptr;
        Value v = args[i];
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

static auto bi_next (ArgVec const& args) -> Value {
    assert(args.num == 1 && args[0].isObj());
    return args[0].obj().next();
}

static Function const f_next (bi_next);

static auto bi_len (ArgVec const& args) -> Value {
    assert(args.num == 1);
    return args[0].asObj().len();
}

static Function const f_len (bi_len);

static auto bi_abs (ArgVec const& args) -> Value {
    assert(args.num == 1);
    return args[0].unOp(UnOp::Abs);
}

static Function const f_abs (bi_abs);

static auto bi_hash (ArgVec const& args) -> Value {
    assert(args.num == 1);
    return args[0].unOp(UnOp::Hash);
}

static Function const f_hash (bi_hash);

static Lookup::Item const builtinsMap [] = {
    //CG< builtin-emit 1
    { Q(188,"array") , Array::info },
    { Q( 62,"bool")  , Bool::info },
    { Q( 66,"bytes") , Bytes::info },
    { Q(189,"class") , Class::info },
    { Q( 75,"dict")  , Dict::info },
    { Q( 94,"int")   , Int::info },
    { Q(108,"list")  , List::info },
    { Q(124,"range") , Range::info },
    { Q(140,"set")   , Set::info },
    { Q(190,"slice") , Slice::info },
    { Q(151,"str")   , Str::info },
    { Q(157,"tuple") , Tuple::info },
    { Q(158,"type")  , Type::info },
    //CG>
    //CG< exception-emit d
    { Q( 33,"BaseException")       , f_BaseException },
    { Q( 36,"Exception")           , f_Exception },
    { Q( 51,"StopIteration")       , f_StopIteration },
    { Q( 30,"ArithmeticError")     , f_ArithmeticError },
    { Q( 31,"AssertionError")      , f_AssertionError },
    { Q( 32,"AttributeError")      , f_AttributeError },
    { Q( 34,"EOFError")            , f_EOFError },
    { Q( 38,"ImportError")         , f_ImportError },
    { Q( 43,"LookupError")         , f_LookupError },
    { Q( 40,"IndexError")          , f_IndexError },
    { Q( 41,"KeyError")            , f_KeyError },
    { Q( 44,"MemoryError")         , f_MemoryError },
    { Q( 45,"NameError")           , f_NameError },
    { Q( 48,"OSError")             , f_OSError },
    { Q( 50,"RuntimeError")        , f_RuntimeError },
    { Q( 47,"NotImplementedError") , f_NotImplementedError },
    { Q( 54,"TypeError")           , f_TypeError },
    { Q( 55,"ValueError")          , f_ValueError },
    { Q(175,"UnicodeError")        , f_UnicodeError },
    //CG>
    { Q(123,"print"), f_print },
    { Q(116,"next"),  f_next },
    { Q(107,"len"),   f_len },
    { Q( 57,"abs"),   f_abs },
    { Q( 90,"hash"),  f_hash },
    { Q(193,"sys"),   m_sys },
#ifndef UNIT_TEST
    { Q(194,"machine"), m_machine },
#endif
#if 0
#if INCLUDE_NETWORK
    { Q(195,"network"), m_network },
#endif
#if INCLUDE_SDCARD
    { Q(196,"sdcard"), m_sdcard },
#endif
#endif
};

Lookup const Monty::builtins (builtinsMap, sizeof builtinsMap);

static auto str_count (ArgVec const&) -> Value {
    return 9; // TODO, hardcoded for features.py
}

static Function const f_str_count (str_count);

static auto str_format (ArgVec const&) -> Value {
    return 4; // TODO, hardcoded for features.py
}

static Function const f_str_format (str_format);

static Lookup::Item const strMap [] = {
    { Q( 74,"count"), f_str_count },
    { Q( 84,"format"), f_str_format },
};

Lookup const Str::attrs (strMap, sizeof strMap);

#if 0
static auto list_append (ArgVec const& args) -> Value {
    assert(args.num == 2);
    auto& l = args[0].asType<List>();
    l.append(args[1]);
    return {};
}

static Function const f_list_append (list_append);

static Lookup::Item const listMap [] = {
    { Q( 60,"append"), f_list_append },
};
#else
// TODO this method wrapper adds 168 bytes on STM32, but is it a one-time cost?
static auto d_list_append = Method::wrap(&List::append);
static Method const m_list_append (d_list_append);

static Lookup::Item const listMap [] = {
    { Q( 60,"append"), m_list_append },
};
#endif

Lookup const List::attrs (listMap, sizeof listMap);

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

auto Bool::create (ArgVec const& args, Type const*) -> Value {
    if (args.num == 1)
        return args[0].unOp(UnOp::Boln);
    assert(args.num == 0);
    return False;
}

auto Int::create (ArgVec const& args, Type const*) -> Value {
    assert(args.num == 1);
    auto v = args[0];
    switch (v.tag()) {
        case Value::Nil: assert(false); break;
        case Value::Int: return v;
        case Value::Str: return atoi(v);
        case Value::Obj: return v.unOp(UnOp::Int);
    }
    return {};
}

auto Bytes::create (ArgVec const& args, Type const*) -> Value {
    assert(args.num == 1);
    Value v = args[0];
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

auto Str::create (ArgVec const& args, Type const*) -> Value {
    assert(args.num == 1 && args[0].isStr());
    return new Str (args[0]);
}

auto Range::create (ArgVec const& args, Type const*) -> Value {
    assert(1 <= args.num && args.num <= 3);
    Value a = args.num > 1 ? args[0] : Value (0);
    Value b = args.num == 1 ? args[0] : args[1];
    Value c = args.num > 2 ? args[2] : b;
    return new Range (a, b, c);
}

auto Slice::create (ArgVec const& args, Type const*) -> Value {
    assert(1 <= args.num && args.num <= 3);
    Value a = args.num > 1 ? args[0] : Value (0);
    Value b = args.num == 1 ? args[0] : args[1];
    Value c = args.num > 2 ? args[2] : b;
    return new Slice (a, b, c);
}

auto Tuple::create (ArgVec const& args, Type const*) -> Value {
    if (args.num == 0)
        return Empty; // there's one unique empty tuple
    return new (args.num * sizeof (Value)) Tuple (args);
}

auto Exception::create (int exc, ArgVec const& args) -> Value {
    // single alloc: first a tuple with args.num values, then exception info
    auto sz = args.num * sizeof (Value) + sizeof (Extra);
    return new (sz) Exception (exc, args);
}

auto Array::create (ArgVec const& args, Type const*) -> Value {
    assert(args.num >= 1 && args[0].isStr());
    char type = *((char const*) args[0]);
    size_t len = 0;
    if (args.num == 2) {
        assert(args[1].isInt());
        len = args[1];
    }
    return new Array (type, len);
}

auto List::create (ArgVec const& args, Type const*) -> Value {
    return new List (args);
}

auto Set::create (ArgVec const& args, Type const*) -> Value {
    return new Set (args);
}

auto Dict::create (ArgVec const&, Type const*) -> Value {
    // TODO pre-alloc space to support fast add, needs vals midway cap iso len
    return new Dict;
}

auto Type::create (ArgVec const& args, Type const*) -> Value {
    assert(args.num == 1);
    Value v = args[0];
    switch (v.tag()) {
        case Value::Nil: assert(false); break;
        case Value::Int: return "int";
        case Value::Str: return "str";
        case Value::Obj: return v.obj().type().name;
    }
    return {};
}

auto Class::create (ArgVec const& args, Type const*) -> Value {
    assert(args.num >= 2 && args[0].isObj() && args[1].isStr());
    return new Class (args);
}

auto Inst::create (ArgVec const& args, Type const* t) -> Value {
    Value v = t;
    return new Inst (args, v.asType<Class>());
}
