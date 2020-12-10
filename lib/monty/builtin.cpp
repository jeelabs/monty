// builtin.cpp - exceptions and auto-generated built-in tables

#include "monty.h"
#include <cassert>

using namespace monty;

//XXX extern Module const m_sys;
//XXX extern Module const m_machine;

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
    { Q(166,"UnicodeError")        , 18 }, // 19 -> ValueError
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

auto Exception::create (E exc, ArgVec const& args) -> Value {
    // single alloc: first a tuple with args.num values, then exception info
    auto sz = args.num * sizeof (Value) + sizeof (Extra);
    return new (sz) Exception (exc, args);
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

Type const Object::info (Q(177,"<object>"));
auto Object::type () const -> Type const& { return info; }

//XXX Type const Inst::info (Q(178,"<instance>"));

//CG< builtin-types lib/monty/monty.h
Type const     Callable::info (Q(167,"<callable>"));
Type const      Context::info (Q(168,"<context>"));
Type const     DictView::info (Q(169,"<dictview>"));
Type const    Exception::info (Q(170,"<exception>"));
Type const     Function::info (Q(171,"<function>"));
Type const     Iterator::info (Q(172,"<iterator>"));
Type const       Lookup::info (Q(173,"<lookup>"));
Type const       Method::info (Q(174,"<method>"));
Type const         None::info (Q(175,"<none>"));

Type const     Bool::info (Q( 62,"bool")  ,   Bool::create, &Bool::attrs);
Type const    Bytes::info (Q( 66,"bytes") ,  Bytes::create, &Bytes::attrs);
Type const     Dict::info (Q( 75,"dict")  ,   Dict::create, &Dict::attrs);
Type const      Int::info (Q( 94,"int")   ,    Int::create, &Int::attrs);
Type const     List::info (Q(108,"list")  ,   List::create, &List::attrs);
Type const    Range::info (Q(124,"range") ,  Range::create, &Range::attrs);
Type const      Set::info (Q(140,"set")   ,    Set::create, &Set::attrs);
Type const    Slice::info (Q(176,"slice") ,  Slice::create, &Slice::attrs);
Type const      Str::info (Q(151,"str")   ,    Str::create, &Str::attrs);
Type const    Tuple::info (Q(157,"tuple") ,  Tuple::create, &Tuple::attrs);
Type const     Type::info (Q(158,"type")  ,   Type::create, &Type::attrs);

auto     Callable::type () const -> Type const& { return info; }
auto      Context::type () const -> Type const& { return info; }
auto     DictView::type () const -> Type const& { return info; }
auto    Exception::type () const -> Type const& { return info; }
auto     Function::type () const -> Type const& { return info; }
auto     Iterator::type () const -> Type const& { return info; }
auto       Lookup::type () const -> Type const& { return info; }
auto       Method::type () const -> Type const& { return info; }
auto         None::type () const -> Type const& { return info; }
auto         Bool::type () const -> Type const& { return info; }
auto        Bytes::type () const -> Type const& { return info; }
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

#if 0 //XXX
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
        if (i > 0)
            buf.putc(' ');
        // if it's a plain string, print as is, else print via repr()
        if (s != nullptr)
            buf.puts(s);
        else
            buf << v;
    }
    buf.putc('\n');
}
#else
static auto bi_print (ArgVec const&) -> Value {
    return {};
}
#endif

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
    { Q(166,"UnicodeError")        , f_UnicodeError },
    //CG>
    //CG< builtin-emit 1
    { Q( 62,"bool")  , Bool::info },
    { Q( 66,"bytes") , Bytes::info },
    { Q( 75,"dict")  , Dict::info },
    { Q( 94,"int")   , Int::info },
    { Q(108,"list")  , List::info },
    { Q(124,"range") , Range::info },
    { Q(140,"set")   , Set::info },
    { Q(176,"slice") , Slice::info },
    { Q(151,"str")   , Str::info },
    { Q(157,"tuple") , Tuple::info },
    { Q(158,"type")  , Type::info },
    //CG>
    { Q(123,"print"), f_print },
    { Q(103,"iter"),  f_iter },
    { Q(116,"next"),  f_next },
    { Q(107,"len"),   f_len },
    { Q( 57,"abs"),   f_abs },
    { Q( 90,"hash"),  f_hash },
};

Lookup const monty::builtins (builtinsMap, sizeof builtinsMap);

auto Exception::findId (Function const& f) -> int {
    for (auto& e : builtinsMap)
        if (&f == &e.v.obj())
            return &e - builtinsMap;
    // searches too many entries, but the assumption is that f will be found
    assert(false);
    return -1;
}

static auto d_list_append = Method::wrap(&List::append);
static Method const m_list_append (d_list_append);

static auto d_list_clear = Method::wrap(&List::clear);
static Method const m_list_clear (d_list_clear);

static Lookup::Item const listMap [] = {
    { Q( 60,"append"), m_list_append },
    { Q( 70,"clear"), m_list_clear },
};

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

// added to satisfy linker

Lookup const     Bool::attrs {nullptr, 0};
Lookup const      Int::attrs {nullptr, 0};
Lookup const    Bytes::attrs {nullptr, 0};
Lookup const      Str::attrs (nullptr, 0);
Lookup const    Range::attrs {nullptr, 0};
Lookup const    Slice::attrs {nullptr, 0};
Lookup const    Tuple::attrs {nullptr, 0};
Lookup const      Set::attrs {nullptr, 0};
Lookup const     Type::attrs {nullptr, 0};
//XXX Lookup const    Array::attrs {nullptr, 0};
//XXX Lookup const    Class::attrs {nullptr, 0};
//XXX Lookup const    Super::attrs {nullptr, 0};
//XXX Lookup const     Inst::attrs {nullptr, 0};
