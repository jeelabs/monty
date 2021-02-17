// builtin.cpp - exceptions and auto-generated built-in tables

#include "monty.h"
#include <cassert>

using namespace monty;

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
    { Q(166,"UnicodeError")        , 18 }, // 19 -> ValueError
    //CG>
};

Lookup const Exception::bases (exceptionMap, sizeof exceptionMap);

Exception::Exception (E exc, ArgVec const& args) : Tuple (args) {
    extra() = { .code=exc, .ipOff=0, .callee=nullptr };
}

auto Exception::binop (BinOp op, Value rhs) const -> Value {
    if (op == BinOp::ExceptionMatch) {
        auto id = findId(rhs);
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
static auto e_Exception (ArgVec const& args) -> Value {
    return Exception::create(E::Exception, args);
}
static auto e_StopIteration (ArgVec const& args) -> Value {
    return Exception::create(E::StopIteration, args);
}
static auto e_ArithmeticError (ArgVec const& args) -> Value {
    return Exception::create(E::ArithmeticError, args);
}
static auto e_ZeroDivisionError (ArgVec const& args) -> Value {
    return Exception::create(E::ZeroDivisionError, args);
}
static auto e_AssertionError (ArgVec const& args) -> Value {
    return Exception::create(E::AssertionError, args);
}
static auto e_AttributeError (ArgVec const& args) -> Value {
    return Exception::create(E::AttributeError, args);
}
static auto e_EOFError (ArgVec const& args) -> Value {
    return Exception::create(E::EOFError, args);
}
static auto e_ImportError (ArgVec const& args) -> Value {
    return Exception::create(E::ImportError, args);
}
static auto e_LookupError (ArgVec const& args) -> Value {
    return Exception::create(E::LookupError, args);
}
static auto e_IndexError (ArgVec const& args) -> Value {
    return Exception::create(E::IndexError, args);
}
static auto e_KeyError (ArgVec const& args) -> Value {
    return Exception::create(E::KeyError, args);
}
static auto e_MemoryError (ArgVec const& args) -> Value {
    return Exception::create(E::MemoryError, args);
}
static auto e_NameError (ArgVec const& args) -> Value {
    return Exception::create(E::NameError, args);
}
static auto e_OSError (ArgVec const& args) -> Value {
    return Exception::create(E::OSError, args);
}
static auto e_RuntimeError (ArgVec const& args) -> Value {
    return Exception::create(E::RuntimeError, args);
}
static auto e_NotImplementedError (ArgVec const& args) -> Value {
    return Exception::create(E::NotImplementedError, args);
}
static auto e_TypeError (ArgVec const& args) -> Value {
    return Exception::create(E::TypeError, args);
}
static auto e_ValueError (ArgVec const& args) -> Value {
    return Exception::create(E::ValueError, args);
}
static auto e_UnicodeError (ArgVec const& args) -> Value {
    return Exception::create(E::UnicodeError, args);
}
//CG>

Type const Object::info (Q(182,"<object>"));
Type const Inst::info (Q(183,"<instance>"));

//CG< builtin-types lib/monty/monty.h
Type    BoundMeth::info (Q(167,"<boundmeth>"));
Type       Buffer::info (Q(168,"<buffer>"));
Type         Cell::info (Q(169,"<cell>"));
Type      Closure::info (Q(170,"<closure>"));
Type     DictView::info (Q(171,"<dictview>"));
Type    Exception::info (Q(172,"<exception>"));
Type     Iterator::info (Q(173,"<iterator>"));
Type       Lookup::info (Q(174,"<lookup>"));
Type       Method::info (Q(175,"<method>"));
Type       Module::info (Q(  7,"<module>"));
Type         None::info (Q(176,"<none>"));
Type     Stacklet::info (Q(177,"<stacklet>"));

Type    Array::info (Q(178,"array") ,  Array::create, &Array::attrs);
Type     Bool::info (Q( 62,"bool")  ,   Bool::create, &Bool::attrs);
Type    Bytes::info (Q( 66,"bytes") ,  Bytes::create, &Bytes::attrs);
Type    Class::info (Q(179,"class") ,  Class::create, &Class::attrs);
Type     Dict::info (Q( 75,"dict")  ,   Dict::create, &Dict::attrs);
Type    Event::info (Q(180,"event") ,  Event::create, &Event::attrs);
Type      Int::info (Q( 94,"int")   ,    Int::create, &Int::attrs);
Type     List::info (Q(108,"list")  ,   List::create, &List::attrs);
Type    Range::info (Q(124,"range") ,  Range::create, &Range::attrs);
Type      Set::info (Q(140,"set")   ,    Set::create, &Set::attrs);
Type    Slice::info (Q(181,"slice") ,  Slice::create, &Slice::attrs);
Type      Str::info (Q(151,"str")   ,    Str::create, &Str::attrs);
Type    Super::info (Q(154,"super") ,  Super::create, &Super::attrs);
Type    Tuple::info (Q(157,"tuple") ,  Tuple::create, &Tuple::attrs);
Type     Type::info (Q(158,"type")  ,   Type::create, &Type::attrs);
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
        if (i > 0)
            buf.putc(' ');
        // if it's a plain string, print as is, else print via repr()
        if (s != nullptr)
            buf.puts(s);
        else
            buf << v;
    }
    buf.putc('\n');
    return {};
}

static auto bi_iter (ArgVec const& args) -> Value {
    assert(args.num == 1 && args[0].isObj());
    auto& o = args[0].obj();
    auto v = o.iter();
    if (v.isInt())
        v = new Iterator (o, v);
    return v;
}

static auto bi_next (ArgVec const& args) -> Value {
    assert(args.num == 1 && args[0].isObj());
    return args[0].obj().next();
}

static auto bi_len (ArgVec const& args) -> Value {
    assert(args.num == 1);
    return args[0].asObj().len();
}

static auto bi_abs (ArgVec const& args) -> Value {
    assert(args.num == 1);
    return args[0].unOp(UnOp::Abs);
}

static auto bi_hash (ArgVec const& args) -> Value {
    assert(args.num == 1);
    return args[0].unOp(UnOp::Hash);
}

static Lookup::Item const builtinsMap [] = {
    // exceptions must be first in the map, see Exception::findId
    //CG< exception-emit d
    { Q( 33,"BaseException")       , e_BaseException },
    { Q( 36,"Exception")           , e_Exception },
    { Q( 51,"StopIteration")       , e_StopIteration },
    { Q( 30,"ArithmeticError")     , e_ArithmeticError },
    { Q( 56,"ZeroDivisionError")   , e_ZeroDivisionError },
    { Q( 31,"AssertionError")      , e_AssertionError },
    { Q( 32,"AttributeError")      , e_AttributeError },
    { Q( 34,"EOFError")            , e_EOFError },
    { Q( 38,"ImportError")         , e_ImportError },
    { Q( 43,"LookupError")         , e_LookupError },
    { Q( 40,"IndexError")          , e_IndexError },
    { Q( 41,"KeyError")            , e_KeyError },
    { Q( 44,"MemoryError")         , e_MemoryError },
    { Q( 45,"NameError")           , e_NameError },
    { Q( 48,"OSError")             , e_OSError },
    { Q( 50,"RuntimeError")        , e_RuntimeError },
    { Q( 47,"NotImplementedError") , e_NotImplementedError },
    { Q( 54,"TypeError")           , e_TypeError },
    { Q( 55,"ValueError")          , e_ValueError },
    { Q(166,"UnicodeError")        , e_UnicodeError },
    //CG>
    //CG< builtin-emit 1
    { Q(178,"array") , Array::info },
    { Q( 62,"bool")  , Bool::info },
    { Q( 66,"bytes") , Bytes::info },
    { Q(179,"class") , Class::info },
    { Q( 75,"dict")  , Dict::info },
    { Q(180,"event") , Event::info },
    { Q( 94,"int")   , Int::info },
    { Q(108,"list")  , List::info },
    { Q(124,"range") , Range::info },
    { Q(140,"set")   , Set::info },
    { Q(181,"slice") , Slice::info },
    { Q(151,"str")   , Str::info },
    { Q(154,"super") , Super::info },
    { Q(157,"tuple") , Tuple::info },
    { Q(158,"type")  , Type::info },
    //CG>
    { Q(123,"print"), bi_print },
    { Q(103,"iter"),  bi_iter },
    { Q(116,"next"),  bi_next },
    { Q(107,"len"),   bi_len },
    { Q( 57,"abs"),   bi_abs },
    { Q( 90,"hash"),  bi_hash },
    { Q(184,"sys"),   m_sys },
#ifndef NOARCH
    { Q(185,"machine"), m_machine },
#endif
};

Lookup const monty::builtins (builtinsMap, sizeof builtinsMap);

auto Exception::findId (Value f) -> int {
    for (auto& e : builtinsMap)
        if (f.id() == e.v.id())
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

static auto d_event_wait = Method::wrap(&Event::wait);
static Method const m_event_wait (d_event_wait);

static auto d_event_set = Method::wrap(&Event::set);
static Method const m_event_set (d_event_set);

static auto d_event_clear = Method::wrap(&Event::clear);
static Method const m_event_clear (d_event_clear);

static Lookup::Item const eventMap [] = {
    { Q(186,"wait"), m_event_wait },
    { Q(140,"set"), m_event_set },
    { Q( 70,"clear"), m_event_clear },
};

Lookup const Event::attrs (eventMap, sizeof eventMap);

// added to satisfy linker
Lookup const   Bool::attrs;
Lookup const    Int::attrs;
Lookup const  Bytes::attrs;
Lookup const    Str::attrs;
Lookup const  Array::attrs;
Lookup const  Range::attrs;
Lookup const  Slice::attrs;
Lookup const  Tuple::attrs;
Lookup const    Set::attrs;
Lookup const   Type::attrs;
Lookup const  Class::attrs;
Lookup const  Super::attrs;
Lookup const   Inst::attrs;
