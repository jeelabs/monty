// builtin.cpp - built-in types and definitions, partly autp-generated

#include "monty.h"
#include "ops.h"
#include <cassert>

using namespace Monty;

extern Module const m_sys;
extern Module const m_machine;

//CG: exception BaseException
//CG: exception Exception BaseException
//CG: exception StopIteration Exception
//CG: exception ArithmeticError Exception
//CG: exception ZeroDivisionError ArithmeticError
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
    { Q( 56,"ZeroDivisionError")   ,  3 }, //  4 -> ArithmeticError
    { Q( 31,"AssertionError")      ,  1 }, //  5 -> Exception
    { Q( 32,"AttributeError")      ,  1 }, //  6 -> Exception
    { Q( 34,"EOFError")            ,  1 }, //  7 -> Exception
    { Q( 38,"ImportError")         ,  1 }, //  8 -> Exception
    { Q( 43,"LookupError")         ,  1 }, //  9 -> Exception
    { Q( 40,"IndexError")          ,  9 }, // 10 -> LookupError
    { Q( 41,"KeyError")            ,  9 }, // 11 -> LookupError
    { Q( 44,"MemoryError")         ,  1 }, // 12 -> Exception
    { Q( 45,"NameError")           ,  1 }, // 13 -> Exception
    { Q( 48,"OSError")             ,  1 }, // 14 -> Exception
    { Q( 50,"RuntimeError")        ,  1 }, // 15 -> Exception
    { Q( 47,"NotImplementedError") , 15 }, // 16 -> RuntimeError
    { Q( 54,"TypeError")           ,  1 }, // 17 -> Exception
    { Q( 55,"ValueError")          ,  1 }, // 18 -> Exception
    { Q(167,"UnicodeError")        , 18 }, // 19 -> ValueError
    //CG>
};

Lookup const Exception::bases (exceptionMap, sizeof exceptionMap);

Exception::Exception (E exc, ArgVec const& args) : Tuple (args) {
    extra() = { .code=exc, .ipOff=0, .callee=nullptr };
}

auto Exception::binop (BinOp op, Value rhs) const -> Value {
    if (op == BinOp::ExceptionMatch) {
        auto id = findId(rhs.asType<Function>());
        auto code = (int) extra().code;
        do {
            if (code == id)
                return True;
            code = exceptionMap[code].v;
        } while (code >= 0);
        return False;
    }
    return Tuple::binop(op, rhs);
}

void Exception::marker () const {
    Tuple::marker();
    mark(extra().callee);
}

//CG< exception-emit f
static auto e_BaseException (ArgVec const& args) -> Value {
    return Exception::create(E::BaseException, args);
}
static Function const f_BaseException (e_BaseException);

static auto e_Exception (ArgVec const& args) -> Value {
    return Exception::create(E::Exception, args);
}
static Function const f_Exception (e_Exception);

static auto e_StopIteration (ArgVec const& args) -> Value {
    return Exception::create(E::StopIteration, args);
}
static Function const f_StopIteration (e_StopIteration);

static auto e_ArithmeticError (ArgVec const& args) -> Value {
    return Exception::create(E::ArithmeticError, args);
}
static Function const f_ArithmeticError (e_ArithmeticError);

static auto e_ZeroDivisionError (ArgVec const& args) -> Value {
    return Exception::create(E::ZeroDivisionError, args);
}
static Function const f_ZeroDivisionError (e_ZeroDivisionError);

static auto e_AssertionError (ArgVec const& args) -> Value {
    return Exception::create(E::AssertionError, args);
}
static Function const f_AssertionError (e_AssertionError);

static auto e_AttributeError (ArgVec const& args) -> Value {
    return Exception::create(E::AttributeError, args);
}
static Function const f_AttributeError (e_AttributeError);

static auto e_EOFError (ArgVec const& args) -> Value {
    return Exception::create(E::EOFError, args);
}
static Function const f_EOFError (e_EOFError);

static auto e_ImportError (ArgVec const& args) -> Value {
    return Exception::create(E::ImportError, args);
}
static Function const f_ImportError (e_ImportError);

static auto e_LookupError (ArgVec const& args) -> Value {
    return Exception::create(E::LookupError, args);
}
static Function const f_LookupError (e_LookupError);

static auto e_IndexError (ArgVec const& args) -> Value {
    return Exception::create(E::IndexError, args);
}
static Function const f_IndexError (e_IndexError);

static auto e_KeyError (ArgVec const& args) -> Value {
    return Exception::create(E::KeyError, args);
}
static Function const f_KeyError (e_KeyError);

static auto e_MemoryError (ArgVec const& args) -> Value {
    return Exception::create(E::MemoryError, args);
}
static Function const f_MemoryError (e_MemoryError);

static auto e_NameError (ArgVec const& args) -> Value {
    return Exception::create(E::NameError, args);
}
static Function const f_NameError (e_NameError);

static auto e_OSError (ArgVec const& args) -> Value {
    return Exception::create(E::OSError, args);
}
static Function const f_OSError (e_OSError);

static auto e_RuntimeError (ArgVec const& args) -> Value {
    return Exception::create(E::RuntimeError, args);
}
static Function const f_RuntimeError (e_RuntimeError);

static auto e_NotImplementedError (ArgVec const& args) -> Value {
    return Exception::create(E::NotImplementedError, args);
}
static Function const f_NotImplementedError (e_NotImplementedError);

static auto e_TypeError (ArgVec const& args) -> Value {
    return Exception::create(E::TypeError, args);
}
static Function const f_TypeError (e_TypeError);

static auto e_ValueError (ArgVec const& args) -> Value {
    return Exception::create(E::ValueError, args);
}
static Function const f_ValueError (e_ValueError);

static auto e_UnicodeError (ArgVec const& args) -> Value {
    return Exception::create(E::UnicodeError, args);
}
static Function const f_UnicodeError (e_UnicodeError);
//CG>

Type const Object::info (Q(185,"<object>"));
auto Object::type () const -> Type const& { return info; }

Type const Inst::info (Q(186,"<instance>"));

//CG< builtin-types lib/monty/monty.h
Type const    BoundMeth::info (Q(168,"<boundmeth>"));
Type const       Buffer::info (Q(169,"<buffer>"));
Type const     Bytecode::info (Q(170,"<bytecode>"));
Type const     Callable::info (Q(171,"<callable>"));
Type const         Cell::info (Q(172,"<cell>"));
Type const      Closure::info (Q(173,"<closure>"));
Type const      Context::info (Q(174,"<context>"));
Type const     DictView::info (Q(175,"<dictview>"));
Type const    Exception::info (Q(176,"<exception>"));
Type const     Function::info (Q(177,"<function>"));
Type const     Iterator::info (Q(178,"<iterator>"));
Type const       Lookup::info (Q(179,"<lookup>"));
Type const       Method::info (Q(180,"<method>"));
Type const       Module::info (Q(  7,"<module>"));
Type const         None::info (Q(181,"<none>"));

Type const    Array::info (Q(182,"array") ,  Array::create, &Array::attrs);
Type const     Bool::info (Q( 62,"bool")  ,   Bool::create, &Bool::attrs);
Type const    Bytes::info (Q( 66,"bytes") ,  Bytes::create, &Bytes::attrs);
Type const    Class::info (Q(183,"class") ,  Class::create, &Class::attrs);
Type const     Dict::info (Q( 75,"dict")  ,   Dict::create, &Dict::attrs);
Type const      Int::info (Q( 94,"int")   ,    Int::create, &Int::attrs);
Type const     List::info (Q(108,"list")  ,   List::create, &List::attrs);
Type const    Range::info (Q(124,"range") ,  Range::create, &Range::attrs);
Type const      Set::info (Q(140,"set")   ,    Set::create, &Set::attrs);
Type const    Slice::info (Q(184,"slice") ,  Slice::create, &Slice::attrs);
Type const      Str::info (Q(151,"str")   ,    Str::create, &Str::attrs);
Type const    Super::info (Q(154,"super") ,  Super::create, &Super::attrs);
Type const    Tuple::info (Q(157,"tuple") ,  Tuple::create, &Tuple::attrs);
Type const     Type::info (Q(158,"type")  ,   Type::create, &Type::attrs);

auto    BoundMeth::type () const -> Type const& { return info; }
auto       Buffer::type () const -> Type const& { return info; }
auto     Bytecode::type () const -> Type const& { return info; }
auto     Callable::type () const -> Type const& { return info; }
auto         Cell::type () const -> Type const& { return info; }
auto      Closure::type () const -> Type const& { return info; }
auto      Context::type () const -> Type const& { return info; }
auto     DictView::type () const -> Type const& { return info; }
auto    Exception::type () const -> Type const& { return info; }
auto     Function::type () const -> Type const& { return info; }
auto     Iterator::type () const -> Type const& { return info; }
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
auto        Super::type () const -> Type const& { return info; }
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

static auto bi_iter (ArgVec const& args) -> Value {
    assert(args.num == 1 && args[0].isObj());
    auto& o = args[0].obj();
    auto v = o.iter();
    if (v.isInt())
        v = new Iterator (o, v);
    return v;
}

static Function const f_iter (bi_iter);

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
    // exceptions must be first in the map, see Exception::findId
    //CG< exception-emit d
    { Q( 33,"BaseException")       , f_BaseException },
    { Q( 36,"Exception")           , f_Exception },
    { Q( 51,"StopIteration")       , f_StopIteration },
    { Q( 30,"ArithmeticError")     , f_ArithmeticError },
    { Q( 56,"ZeroDivisionError")   , f_ZeroDivisionError },
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
    { Q(167,"UnicodeError")        , f_UnicodeError },
    //CG>
    //CG< builtin-emit 1
    { Q(182,"array") , Array::info },
    { Q( 62,"bool")  , Bool::info },
    { Q( 66,"bytes") , Bytes::info },
    { Q(183,"class") , Class::info },
    { Q( 75,"dict")  , Dict::info },
    { Q( 94,"int")   , Int::info },
    { Q(108,"list")  , List::info },
    { Q(124,"range") , Range::info },
    { Q(140,"set")   , Set::info },
    { Q(184,"slice") , Slice::info },
    { Q(151,"str")   , Str::info },
    { Q(154,"super") , Super::info },
    { Q(157,"tuple") , Tuple::info },
    { Q(158,"type")  , Type::info },
    //CG>
    { Q(123,"print"), f_print },
    { Q(103,"iter"),  f_iter },
    { Q(116,"next"),  f_next },
    { Q(107,"len"),   f_len },
    { Q( 57,"abs"),   f_abs },
    { Q( 90,"hash"),  f_hash },
    { Q(187,"sys"),   m_sys },
#ifndef UNIT_TEST
    { Q(188,"machine"), m_machine },
#endif
#if 0
#if INCLUDE_NETWORK
    { Q(189,"network"), m_network },
#endif
#if INCLUDE_SDCARD
    { Q(190,"sdcard"), m_sdcard },
#endif
#endif
};

Lookup const Monty::builtins (builtinsMap, sizeof builtinsMap);

auto Exception::findId (Function const& f) -> int {
    for (auto& e : builtinsMap)
        if (&f == &e.v.obj())
            return &e - builtinsMap;
    // searches too many entries, but the assumption is that f will be found
    assert(false);
    return -1;
}

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

static auto d_list_clear = Method::wrap(&List::clear);
static Method const m_list_clear (d_list_clear);

static Lookup::Item const listMap [] = {
    { Q( 60,"append"), m_list_append },
    { Q( 70,"clear"), m_list_clear },
};
#endif

Lookup const List::attrs (listMap, sizeof listMap);

static auto d_dict_keys = Method::wrap(&Dict::keys);
static Method const m_dict_keys (d_dict_keys);

static auto d_dict_values = Method::wrap(&Dict::values);
static Method const m_dict_values (d_dict_values);

static auto d_dict_items = Method::wrap(&Dict::items);
static Method const m_dict_items (d_dict_items);

static Lookup::Item const dictMap [] = {
    { Q(106,"keys"), m_dict_keys },
    { Q(163,"values"), m_dict_values },
    { Q(102,"items"), m_dict_items },
};

Lookup const Dict::attrs (dictMap, sizeof dictMap);

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
Lookup const     Type::attrs {nullptr, 0};
Lookup const    Class::attrs {nullptr, 0};
Lookup const    Super::attrs {nullptr, 0};
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
        case Value::Nil: // fall through
        case Value::Int: return v;
        case Value::Str: return Int::conv(v);
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
    uint32_t n = 0;
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
    int a = args.num > 1 ? (int) args[0] : 0;
    int b = args.num == 1 ? args[0] : args[1];
    int c = args.num > 2 ? (int) args[2] : 1;
    return new Range (a, b, c);
}

auto Slice::create (ArgVec const& args, Type const*) -> Value {
    assert(1 <= args.num && args.num <= 3);
    Value a = args.num > 1 ? args[0] : Null;
    Value b = args.num == 1 ? args[0] : args[1];
    Value c = args.num > 2 ? args[2] : Null;
    return new Slice (a, b, c);
}

auto Slice::asRange (int sz) const -> Range {
    int from = off.isInt() ? (int) off : 0;
    int to = num.isInt() ? (int) num : sz;
    int by = step.isInt() ? (int) step : 1;
    if (from < 0)
        from += sz;
    if (to < 0)
        to += sz;
    if (by < 0) {
        auto t = from - 1;
        from = to - 1;
        to = t;
    }
    return {from, to, by};
}

auto Tuple::create (ArgVec const& args, Type const*) -> Value {
    if (args.num == 0)
        return Empty; // there's one unique empty tuple
    return new (args.num * sizeof (Value)) Tuple (args);
}

auto Exception::create (E exc, ArgVec const& args) -> Value {
    // single alloc: first a tuple with args.num values, then exception info
    auto sz = args.num * sizeof (Value) + sizeof (Extra);
    return new (sz) Exception (exc, args);
}

auto Array::create (ArgVec const& args, Type const*) -> Value {
    assert(args.num >= 1 && args[0].isStr());
    char type = *((char const*) args[0]);
    uint32_t len = 0;
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
        case Value::Nil: break;
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

auto Super::create (ArgVec const& args, Type const*) -> Value {
    return new Super (args);
}

auto Inst::create (ArgVec const& args, Type const* t) -> Value {
    Value v = t;
    return new Inst (args, v.asType<Class>());
}
