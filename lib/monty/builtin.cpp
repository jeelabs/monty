// builtin.cpp - exceptions and auto-generated built-in tables

#include "monty.h"
#include <cassert>

//CG1 if dir extend
#include <extend.h>

using namespace monty;

//CG1 bind print
static auto f_print (ArgVec const& args) -> Value {
    Buffer buf;
    for (int i = 0; i < args.size(); ++i) {
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
            buf << ' ';
        // if it's a plain string, print as is, else print via repr()
        if (s != nullptr)
            buf << s;
        else
            buf << v;
    }
    buf << '\n';
    return {};
}

//CG1 bind iter
static auto f_iter (ArgVec const& args) -> Value {
    assert(args.size() == 1 && args[0].isObj());
    auto v = args[0]->iter();
    return v.isObj() ? v : new Iterator (args[0], 0);
}

//CG1 bind next
static auto f_next (ArgVec const& args) -> Value {
    assert(args.size() == 1);
    auto v = args[0]->next();
    return v.isNil() && Stacklet::current != nullptr ? Value {E::StopIteration} : v;
}

//CG1 bind len
static auto f_len (ArgVec const& args) -> Value {
    assert(args.size() == 1);
    return args[0].isStr() ? strlen(args[0]) : args[0]->len();
}

//CG1 bind abs
static auto f_abs (ArgVec const& args) -> Value {
    assert(args.size() == 1);
    return args[0].unOp(UnOp::Abso);
}

//CG1 bind hash
static auto f_hash (ArgVec const& args) -> Value {
    assert(args.size() == 1);
    return args[0].unOp(UnOp::Hash);
}

//CG1 bind id
static auto f_id (ArgVec const& args) -> Value {
    assert(args.size() == 1);
    return args[0].id();
}

//CG1 bind dir
static auto f_dir (ArgVec const& args) -> Value {
    assert(args.size() == 1);
    auto r = new Set;

    Object const* obj = &args[0].asObj();
    if (obj != &Module::builtins &&
            obj != &Module::loaded &&
            &obj->type() != &Type::info &&
            &obj->type() != &Module::info)
        obj = &obj->type();

    do {
        if (obj != &Module::builtins && obj != &Module::loaded)
            for (auto e : *(Dict const*) obj)
                r->has(e) = true;
        //obj->type()._name.dump("switch");
        switch (obj->type()._name.asQid()) {
            case Q(7,"<module>"):
            case Q(158,"type")._id:
            case Q(75,"dict")._id:
                obj = ((Dict const*) obj)->_chain; break;
            case Q(196,"<lookup>"): 
                obj = ((Lookup const*) obj)->attrDir(r); break;
            default: obj = nullptr;
        }
    } while (obj != nullptr);

    return r;
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
    { Q(33,"BaseException"),        -1 }, //  0 -> -
    { Q(36,"Exception"),             0 }, //  1 -> BaseException
    { Q(51,"StopIteration"),         1 }, //  2 -> Exception
    { Q(31,"AssertionError"),        1 }, //  3 -> Exception
    { Q(32,"AttributeError"),        1 }, //  4 -> Exception
    { Q(34,"EOFError"),              1 }, //  5 -> Exception
    { Q(38,"ImportError"),           1 }, //  6 -> Exception
    { Q(44,"MemoryError"),           1 }, //  7 -> Exception
    { Q(45,"NameError"),             1 }, //  8 -> Exception
    { Q(48,"OSError"),               1 }, //  9 -> Exception
    { Q(54,"TypeError"),             1 }, // 10 -> Exception
    { Q(30,"ArithmeticError"),       1 }, // 11 -> Exception
    { Q(56,"ZeroDivisionError"),    11 }, // 12 -> ArithmeticError
    { Q(43,"LookupError"),           1 }, // 13 -> Exception
    { Q(40,"IndexError"),           13 }, // 14 -> LookupError
    { Q(41,"KeyError"),             13 }, // 15 -> LookupError
    { Q(50,"RuntimeError"),          1 }, // 16 -> Exception
    { Q(47,"NotImplementedError"),  16 }, // 17 -> RuntimeError
    { Q(55,"ValueError"),            1 }, // 18 -> Exception
    { Q(167,"UnicodeError"),        18 }, // 19 -> ValueError
    //CG>
};

Lookup const Exception::bases (exceptionMap);

//CG< wrappers
static Function const fo_abs (f_abs);
static Function const fo_dir (f_dir);
static Function const fo_hash (f_hash);
static Function const fo_id (f_id);
static Function const fo_iter (f_iter);
static Function const fo_len (f_len);
static Function const fo_next (f_next);
static Function const fo_print (f_print);

static auto const m_dict_items = Method::wrap(&Dict::items);
static Method const mo_dict_items (m_dict_items);

static auto const m_dict_keys = Method::wrap(&Dict::keys);
static Method const mo_dict_keys (m_dict_keys);

static auto const m_dict_values = Method::wrap(&Dict::values);
static Method const mo_dict_values (m_dict_values);

static Lookup::Item const dict_map [] = {
    { Q(102,"items"), mo_dict_items },
    { Q(106,"keys"), mo_dict_keys },
    { Q(163,"values"), mo_dict_values },
};
Lookup const Dict::attrs (dict_map);

static auto const m_event_clear = Method::wrap(&Event::clear);
static Method const mo_event_clear (m_event_clear);

static auto const m_event_set = Method::wrap(&Event::set);
static Method const mo_event_set (m_event_set);

static auto const m_event_wait = Method::wrap(&Event::wait);
static Method const mo_event_wait (m_event_wait);

static Lookup::Item const event_map [] = {
    { Q(70,"clear"), mo_event_clear },
    { Q(140,"set"), mo_event_set },
    { Q(187,"wait"), mo_event_wait },
};
Lookup const Event::attrs (event_map);

static auto const m_exception_trace = Method::wrap(&Exception::trace);
static Method const mo_exception_trace (m_exception_trace);

static Lookup::Item const exception_map [] = {
    { Q(188,"trace"), mo_exception_trace },
};
Lookup const Exception::attrs (exception_map);

static auto const m_list_append = Method::wrap(&List::append);
static Method const mo_list_append (m_list_append);

static auto const m_list_clear = Method::wrap(&List::clear);
static Method const mo_list_clear (m_list_clear);

static auto const m_list_pop = Method::wrap(&List::pop);
static Method const mo_list_pop (m_list_pop);

static Lookup::Item const list_map [] = {
    { Q(60,"append"), mo_list_append },
    { Q(70,"clear"), mo_list_clear },
    { Q(120,"pop"), mo_list_pop },
};
Lookup const List::attrs (list_map);
//CG>

static Lookup::Item const builtinsMap [] = {
    // exceptions must be first in the map, see Exception::findId
    //CG< exception-emit d
    { Q(33,"BaseException"),        fo_BaseException },
    { Q(36,"Exception"),            fo_Exception },
    { Q(51,"StopIteration"),        fo_StopIteration },
    { Q(31,"AssertionError"),       fo_AssertionError },
    { Q(32,"AttributeError"),       fo_AttributeError },
    { Q(34,"EOFError"),             fo_EOFError },
    { Q(38,"ImportError"),          fo_ImportError },
    { Q(44,"MemoryError"),          fo_MemoryError },
    { Q(45,"NameError"),            fo_NameError },
    { Q(48,"OSError"),              fo_OSError },
    { Q(54,"TypeError"),            fo_TypeError },
    { Q(30,"ArithmeticError"),      fo_ArithmeticError },
    { Q(56,"ZeroDivisionError"),    fo_ZeroDivisionError },
    { Q(43,"LookupError"),          fo_LookupError },
    { Q(40,"IndexError"),           fo_IndexError },
    { Q(41,"KeyError"),             fo_KeyError },
    { Q(50,"RuntimeError"),         fo_RuntimeError },
    { Q(47,"NotImplementedError"),  fo_NotImplementedError },
    { Q(55,"ValueError"),           fo_ValueError },
    { Q(167,"UnicodeError"),        fo_UnicodeError },
    //CG>
    //CG< type-builtin
    { Q(189,"array"), Array::info },
    { Q(62,"bool"),   Bool::info },
    { Q(66,"bytes"),  Bytes::info },
    { Q(190,"class"), Class::info },
    { Q(75,"dict"),   Dict::info },
    { Q(171,"event"), Event::info },
    { Q(94,"int"),    Int::info },
    { Q(108,"list"),  List::info },
    { Q(124,"range"), Range::info },
    { Q(140,"set"),   Set::info },
    { Q(191,"slice"), Slice::info },
    { Q(151,"str"),   Str::info },
    { Q(154,"super"), Super::info },
    { Q(157,"tuple"), Tuple::info },
    { Q(158,"type"),  Type::info },
    //CG>
    { Q(57,"abs"),    fo_abs },
    { Q(76,"dir"),    fo_dir },
    { Q(90,"hash"),   fo_hash },
    { Q(91,"id"),     fo_id },
    { Q(103,"iter"),  fo_iter },
    { Q(107,"len"),   fo_len },
    { Q(116,"next"),  fo_next },
    { Q(123,"print"), fo_print },
};

static Lookup const builtins_attrs (builtinsMap);
Dict Module::builtins (&builtins_attrs);

Exception::Exception (E code, ArgVec const& args) : Tuple (args), _code (code) {
    adj(_fill+1);
    (*this)[_fill] = _fill+1;
    // TODO nasty, fixup Exception::info, as it doesn't have the attrs set
    //  because exception is not an exposed type ("<exception>" iso "exception")
    info._chain = &attrs;
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
        auto code = (int) _code;
        do {
            if (code == id)
                return True;
            code = exceptionMap[code].v;
        } while (code >= 0);
        return False;
    }
    return Tuple::binop(op, rhs);
}

struct SizeFix {
    SizeFix (Exception& exc) : _exc (exc), _orig (exc._fill) {
        _exc._fill = _exc[_orig]; // expose the trace info, temporarily
    }
    ~SizeFix () {
        _exc[_orig] = _exc._fill;
        _exc._fill = _orig;
    }

    Exception& _exc;
    uint32_t _orig;
};

void Exception::addTrace (uint32_t off, Value bc) {
    SizeFix fixer (*this);
    append(off);
    append(bc);
}

auto Exception::trace () -> Value {
    auto r = new List;
    SizeFix fixer (*this);
    for (uint32_t i = fixer._orig + 1; i < size(); i += 2) {
        auto e = (*this)[i+1]; // Bytecode object
        r->append(e->getAt(-1));            // filename
        r->append(e->getAt((*this)[i]));    // line number
        r->append(e->getAt(-2));            // function
    }
    return r;
}

void Exception::marker () const {
    SizeFix fixer (*const_cast<Exception*>(this));
    Tuple::marker();
}

auto Exception::create (E exc, ArgVec const& args) -> Value {
    return new Exception (exc, args);
}

void Exception::repr (Buffer& buf) const {
    buf << (char const*) bases._items[(int) _code].k;
    Tuple::repr(buf);
}

//CG< type-info
Type    Buffer::info (Q(192,"<buffer>"));
Type Exception::info (Q(193,"<exception>"));
Type  Function::info (Q(194,"<function>"));
Type  Iterator::info (Q(195,"<iterator>"));
Type    Lookup::info (Q(196,"<lookup>"));
Type    Method::info (Q(197,"<method>"));
Type    Module::info (Q(7,"<module>"));
Type      None::info (Q(198,"<none>"));
Type  Stacklet::info (Q(199,"<stacklet>"));

Type     Array::info (Q(189,"array"), &Array::attrs, Array::create);
Type      Bool::info (Q(62,"bool"),    &Bool::attrs,  Bool::create);
Type     Bytes::info (Q(66,"bytes"),  &Bytes::attrs, Bytes::create);
Type     Class::info (Q(190,"class"), &Class::attrs, Class::create);
Type      Dict::info (Q(75,"dict"),    &Dict::attrs,  Dict::create);
Type     Event::info (Q(171,"event"), &Event::attrs, Event::create);
Type       Int::info (Q(94,"int"),      &Int::attrs,   Int::create);
Type      List::info (Q(108,"list"),   &List::attrs,  List::create);
Type     Range::info (Q(124,"range"), &Range::attrs, Range::create);
Type       Set::info (Q(140,"set"),     &Set::attrs,   Set::create);
Type     Slice::info (Q(191,"slice"), &Slice::attrs, Slice::create);
Type       Str::info (Q(151,"str"),     &Str::attrs,   Str::create);
Type     Super::info (Q(154,"super"), &Super::attrs, Super::create);
Type     Tuple::info (Q(157,"tuple"), &Tuple::attrs, Tuple::create);
Type      Type::info (Q(158,"type"),   &Type::attrs,  Type::create);
//CG>
