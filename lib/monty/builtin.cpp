// builtin.cpp - exceptions and auto-generated built-in tables

#include "monty.h"
#include <cassert>

using namespace monty;

extern Lookup const sys_attrs;
static Module const m_sys (sys_attrs);

extern Lookup const machine_attrs;
static Module const m_machine (machine_attrs);

//CG1 bind print
static auto f_print (ArgVec const& args) -> Value {
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

//CG1 bind iter
static auto f_iter (ArgVec const& args) -> Value {
    assert(args.num == 1 && args[0].isObj());
    auto& o = args[0].obj();
    auto v = o.iter();
    if (v.isInt())
        v = new Iterator (o, v);
    return v;
}

//CG1 bind next
static auto f_next (ArgVec const& args) -> Value {
    assert(args.num == 1 && args[0].isObj());
    return args[0].obj().next();
}

//CG1 bind len
static auto f_len (ArgVec const& args) -> Value {
    assert(args.num == 1);
    return args[0].asObj().len();
}

//CG1 bind abs
static auto f_abs (ArgVec const& args) -> Value {
    assert(args.num == 1);
    return args[0].unOp(UnOp::Abs);
}

//CG1 bind hash
static auto f_hash (ArgVec const& args) -> Value {
    assert(args.num == 1);
    return args[0].unOp(UnOp::Hash);
}

//CG< exception-emit f
static auto e_BaseException (ArgVec const& args) -> Value {
    return Exception::create(E::BaseException, args);
}
static Function const fo_BaseException (e_BaseException);
static auto e_Exception (ArgVec const& args) -> Value {
    return Exception::create(E::Exception, args);
}
static Function const fo_Exception (e_Exception);
static auto e_StopIteration (ArgVec const& args) -> Value {
    return Exception::create(E::StopIteration, args);
}
static Function const fo_StopIteration (e_StopIteration);
static auto e_AssertionError (ArgVec const& args) -> Value {
    return Exception::create(E::AssertionError, args);
}
static Function const fo_AssertionError (e_AssertionError);
static auto e_AttributeError (ArgVec const& args) -> Value {
    return Exception::create(E::AttributeError, args);
}
static Function const fo_AttributeError (e_AttributeError);
static auto e_EOFError (ArgVec const& args) -> Value {
    return Exception::create(E::EOFError, args);
}
static Function const fo_EOFError (e_EOFError);
static auto e_ImportError (ArgVec const& args) -> Value {
    return Exception::create(E::ImportError, args);
}
static Function const fo_ImportError (e_ImportError);
static auto e_MemoryError (ArgVec const& args) -> Value {
    return Exception::create(E::MemoryError, args);
}
static Function const fo_MemoryError (e_MemoryError);
static auto e_NameError (ArgVec const& args) -> Value {
    return Exception::create(E::NameError, args);
}
static Function const fo_NameError (e_NameError);
static auto e_OSError (ArgVec const& args) -> Value {
    return Exception::create(E::OSError, args);
}
static Function const fo_OSError (e_OSError);
static auto e_TypeError (ArgVec const& args) -> Value {
    return Exception::create(E::TypeError, args);
}
static Function const fo_TypeError (e_TypeError);
static auto e_ArithmeticError (ArgVec const& args) -> Value {
    return Exception::create(E::ArithmeticError, args);
}
static Function const fo_ArithmeticError (e_ArithmeticError);
static auto e_ZeroDivisionError (ArgVec const& args) -> Value {
    return Exception::create(E::ZeroDivisionError, args);
}
static Function const fo_ZeroDivisionError (e_ZeroDivisionError);
static auto e_LookupError (ArgVec const& args) -> Value {
    return Exception::create(E::LookupError, args);
}
static Function const fo_LookupError (e_LookupError);
static auto e_IndexError (ArgVec const& args) -> Value {
    return Exception::create(E::IndexError, args);
}
static Function const fo_IndexError (e_IndexError);
static auto e_KeyError (ArgVec const& args) -> Value {
    return Exception::create(E::KeyError, args);
}
static Function const fo_KeyError (e_KeyError);
static auto e_RuntimeError (ArgVec const& args) -> Value {
    return Exception::create(E::RuntimeError, args);
}
static Function const fo_RuntimeError (e_RuntimeError);
static auto e_NotImplementedError (ArgVec const& args) -> Value {
    return Exception::create(E::NotImplementedError, args);
}
static Function const fo_NotImplementedError (e_NotImplementedError);
static auto e_ValueError (ArgVec const& args) -> Value {
    return Exception::create(E::ValueError, args);
}
static Function const fo_ValueError (e_ValueError);
static auto e_UnicodeError (ArgVec const& args) -> Value {
    return Exception::create(E::UnicodeError, args);
}
static Function const fo_UnicodeError (e_UnicodeError);
//CG>

static Lookup::Item const exceptionMap [] = {
    //CG< exception-emit h
    { Q( 33,"BaseException"),       -1 }, //  0 -> -
    { Q( 36,"Exception"),            0 }, //  1 -> BaseException
    { Q( 51,"StopIteration"),        1 }, //  2 -> Exception
    { Q( 31,"AssertionError"),       1 }, //  3 -> Exception
    { Q( 32,"AttributeError"),       1 }, //  4 -> Exception
    { Q( 34,"EOFError"),             1 }, //  5 -> Exception
    { Q( 38,"ImportError"),          1 }, //  6 -> Exception
    { Q( 44,"MemoryError"),          1 }, //  7 -> Exception
    { Q( 45,"NameError"),            1 }, //  8 -> Exception
    { Q( 48,"OSError"),              1 }, //  9 -> Exception
    { Q( 54,"TypeError"),            1 }, // 10 -> Exception
    { Q( 30,"ArithmeticError"),      1 }, // 11 -> Exception
    { Q( 56,"ZeroDivisionError"),   11 }, // 12 -> ArithmeticError
    { Q( 43,"LookupError"),          1 }, // 13 -> Exception
    { Q( 40,"IndexError"),          13 }, // 14 -> LookupError
    { Q( 41,"KeyError"),            13 }, // 15 -> LookupError
    { Q( 50,"RuntimeError"),         1 }, // 16 -> Exception
    { Q( 47,"NotImplementedError"), 16 }, // 17 -> RuntimeError
    { Q( 55,"ValueError"),           1 }, // 18 -> Exception
    { Q(176,"UnicodeError"),        18 }, // 19 -> ValueError
    //CG>
};

Lookup const Exception::bases (exceptionMap, sizeof exceptionMap);

//CG< wrappers
static Function const fo_abs (f_abs);
static Function const fo_hash (f_hash);
static Function const fo_iter (f_iter);
static Function const fo_len (f_len);
static Function const fo_next (f_next);
static Function const fo_print (f_print);

static auto m_dict_items = Method::wrap(&Dict::items);
static Method const mo_dict_items (m_dict_items);

static auto m_dict_keys = Method::wrap(&Dict::keys);
static Method const mo_dict_keys (m_dict_keys);

static auto m_dict_values = Method::wrap(&Dict::values);
static Method const mo_dict_values (m_dict_values);

static Lookup::Item const dict_map [] = {
    { Q(102,"items"), mo_dict_items },
    { Q(106,"keys"), mo_dict_keys },
    { Q(163,"values"), mo_dict_values },
};
Lookup const Dict::attrs (dict_map, sizeof dict_map);

static auto m_event_clear = Method::wrap(&Event::clear);
static Method const mo_event_clear (m_event_clear);

static auto m_event_set = Method::wrap(&Event::set);
static Method const mo_event_set (m_event_set);

static auto m_event_wait = Method::wrap(&Event::wait);
static Method const mo_event_wait (m_event_wait);

static Lookup::Item const event_map [] = {
    { Q( 70,"clear"), mo_event_clear },
    { Q(140,"set"), mo_event_set },
    { Q(182,"wait"), mo_event_wait },
};
Lookup const Event::attrs (event_map, sizeof event_map);

static auto m_list_append = Method::wrap(&List::append);
static Method const mo_list_append (m_list_append);

static auto m_list_clear = Method::wrap(&List::clear);
static Method const mo_list_clear (m_list_clear);

static auto m_list_pop = Method::wrap(&List::pop);
static Method const mo_list_pop (m_list_pop);

static Lookup::Item const list_map [] = {
    { Q( 60,"append"), mo_list_append },
    { Q( 70,"clear"), mo_list_clear },
    { Q(120,"pop"), mo_list_pop },
};
Lookup const List::attrs (list_map, sizeof list_map);
//CG>

static Lookup::Item const builtinsMap [] = {
    // exceptions must be first in the map, see Exception::findId
    //CG< exception-emit d
    { Q( 33,"BaseException"),       fo_BaseException },
    { Q( 36,"Exception"),           fo_Exception },
    { Q( 51,"StopIteration"),       fo_StopIteration },
    { Q( 31,"AssertionError"),      fo_AssertionError },
    { Q( 32,"AttributeError"),      fo_AttributeError },
    { Q( 34,"EOFError"),            fo_EOFError },
    { Q( 38,"ImportError"),         fo_ImportError },
    { Q( 44,"MemoryError"),         fo_MemoryError },
    { Q( 45,"NameError"),           fo_NameError },
    { Q( 48,"OSError"),             fo_OSError },
    { Q( 54,"TypeError"),           fo_TypeError },
    { Q( 30,"ArithmeticError"),     fo_ArithmeticError },
    { Q( 56,"ZeroDivisionError"),   fo_ZeroDivisionError },
    { Q( 43,"LookupError"),         fo_LookupError },
    { Q( 40,"IndexError"),          fo_IndexError },
    { Q( 41,"KeyError"),            fo_KeyError },
    { Q( 50,"RuntimeError"),        fo_RuntimeError },
    { Q( 47,"NotImplementedError"), fo_NotImplementedError },
    { Q( 55,"ValueError"),          fo_ValueError },
    { Q(176,"UnicodeError"),        fo_UnicodeError },
    //CG>
    //CG< type-builtin
    { Q(183,"array"), Array::info },
    { Q( 62,"bool"),  Bool::info },
    { Q( 66,"bytes"), Bytes::info },
    { Q(184,"class"), Class::info },
    { Q( 75,"dict"),  Dict::info },
    { Q(167,"event"), Event::info },
    { Q( 94,"int"),   Int::info },
    { Q(108,"list"),  List::info },
    { Q(124,"range"), Range::info },
    { Q(140,"set"),   Set::info },
    { Q(185,"slice"), Slice::info },
    { Q(151,"str"),   Str::info },
    { Q(154,"super"), Super::info },
    { Q(157,"tuple"), Tuple::info },
    { Q(158,"type"),  Type::info },
    //CG>
    { Q(123,"print"), fo_print },
    { Q(103,"iter"),  fo_iter },
    { Q(116,"next"),  fo_next },
    { Q(107,"len"),   fo_len },
    { Q( 57,"abs"),   fo_abs },
    { Q( 90,"hash"),  fo_hash },
    { Q(198,"sys"),   m_sys },
#ifndef NOARCH
    { Q(199,"machine"), m_machine },
#endif
};

Lookup const Module::builtins (builtinsMap, sizeof builtinsMap);

Exception::Exception (E exc, ArgVec const& args) : Tuple (args) {
    extra() = { .code=exc, .ipOff=0, .callee=nullptr };
}

auto Exception::findId (Function const& f) -> int {
    for (auto& e : builtinsMap)
        if (&f == &e.v.obj())
            return &e - builtinsMap;
    // searches too many entries, but the assumption is that f will be found
    assert(false);
    return -1;
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

//CG< type-info
Type    BoundMeth::info (Q(186,"<boundmeth>"));
Type       Buffer::info (Q(187,"<buffer>"));
Type         Cell::info (Q(188,"<cell>"));
Type      Closure::info (Q(189,"<closure>"));
Type     DictView::info (Q(190,"<dictview>"));
Type    Exception::info (Q(191,"<exception>"));
Type     Function::info (Q(192,"<function>"));
Type     Iterator::info (Q(193,"<iterator>"));
Type       Lookup::info (Q(194,"<lookup>"));
Type       Method::info (Q(195,"<method>"));
Type       Module::info (Q(  7,"<module>"));
Type         None::info (Q(196,"<none>"));
Type     Stacklet::info (Q(197,"<stacklet>"));

Type    Array::info (Q(183,"array") ,  Array::create, &Array::attrs);
Type     Bool::info (Q( 62,"bool")  ,   Bool::create, &Bool::attrs);
Type    Bytes::info (Q( 66,"bytes") ,  Bytes::create, &Bytes::attrs);
Type    Class::info (Q(184,"class") ,  Class::create, &Class::attrs);
Type     Dict::info (Q( 75,"dict")  ,   Dict::create, &Dict::attrs);
Type    Event::info (Q(167,"event") ,  Event::create, &Event::attrs);
Type      Int::info (Q( 94,"int")   ,    Int::create, &Int::attrs);
Type     List::info (Q(108,"list")  ,   List::create, &List::attrs);
Type    Range::info (Q(124,"range") ,  Range::create, &Range::attrs);
Type      Set::info (Q(140,"set")   ,    Set::create, &Set::attrs);
Type    Slice::info (Q(185,"slice") ,  Slice::create, &Slice::attrs);
Type      Str::info (Q(151,"str")   ,    Str::create, &Str::attrs);
Type    Super::info (Q(154,"super") ,  Super::create, &Super::attrs);
Type    Tuple::info (Q(157,"tuple") ,  Tuple::create, &Tuple::attrs);
Type     Type::info (Q(158,"type")  ,   Type::create, &Type::attrs);
//CG>

// added to satisfy linker
Lookup const  Array::attrs;
Lookup const  Bool::attrs;
Lookup const  Bytes::attrs;
Lookup const  Class::attrs;
Lookup const  Inst::attrs;
Lookup const  Int::attrs;
Lookup const  Range::attrs;
Lookup const  Set::attrs;
Lookup const  Slice::attrs;
Lookup const  Str::attrs;
Lookup const  Super::attrs;
Lookup const  Tuple::attrs;
Lookup const  Type::attrs;
