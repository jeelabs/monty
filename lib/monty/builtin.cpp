// builtin.cpp - exceptions and auto-generated built-in tables

#include "monty.h"
#include <cassert>

//CG1 if dir extend
#include <extend.h>

using namespace monty;

//CG1 bind argtest
extern auto f_argtest (ArgVec const& args) -> Value {
    //CG: args a1 a2 a3:o a4:i ? a5 a6:s a7:s a8 *
    //CG: kwargs foo bar baz
    if (a1.isInt()) // special, returns parse result: N<0 = missing, N>0 = extra
        return ainfo;
    auto n = a1.id()+a2.id()+a5.id()+a8.id()+(int)(uintptr_t)a3+a4;
    return n + (a6 != nullptr ? *a6 : 0) + (a7 != nullptr ? *a7 : 0);
}

//CG1 bind print
extern auto f_print (ArgVec const& args) -> Value {
    //CG: kwargs end sep
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
        if (i > 0) {
            if (sep.isStr())
                buf << (char const*) sep;
            else
                buf << ' ';
        }
        // if it's a plain string, print as is, else print via repr()
        if (s != nullptr)
            buf << s;
        else
            buf << v;
    }
    if (end.isOk())
        buf << end;
    else
        buf << '\n';
    return {};
}

//CG1 bind iter
extern auto f_iter (ArgVec const& args) -> Value {
    //CG: args obj:o
    auto v = obj->iter();
    return v.isObj() ? v : new Iterator (args[0], 0);
}

//CG1 bind next
extern auto f_next (ArgVec const& args) -> Value {
    //CG: args arg
    auto v = arg->next();
    return v.isNil() && Stacklet::current != nullptr ? Value {E::StopIteration} : v;
}

//CG1 bind len
extern auto f_len (ArgVec const& args) -> Value {
    //CG: args arg
    return arg.isStr() ? strlen(arg) : arg->len();
}

//CG1 bind abs
extern auto f_abs (ArgVec const& args) -> Value {
    //CG: args arg
    return arg.unOp(UnOp::Abso);
}

//CG1 bind hash
extern auto f_hash (ArgVec const& args) -> Value {
    //CG: args arg
    return arg.unOp(UnOp::Hash);
}

//CG1 bind id
extern auto f_id (ArgVec const& args) -> Value {
    //CG: args arg
    return arg.id();
}

//CG1 bind dir
extern auto f_dir (ArgVec const& args) -> Value {
    //CG: args arg

    Object const* obj = &arg.asObj();
    if (obj != &Module::builtins &&
            obj != &Module::loaded &&
            &obj->type() != &Type::info &&
            &obj->type() != &Module::info)
        obj = &obj->type();

    auto r = new Set;
    do {
        if (obj != &Module::builtins && obj != &Module::loaded)
            for (auto e : *(Dict const*) obj)
                r->has(e) = true;
        //obj->type()._name.dump("switch");
        switch (obj->type()._name.asQid()) {
            case Q(0,"<module>"):
            case Q(0,"type")._id:
            case Q(0,"dict")._id:
                obj = ((Dict const*) obj)->_chain; break;
            case Q(0,"<lookup>"): 
                obj = ((Lookup const*) obj)->attrDir(r); break;
            default: obj = nullptr;
        }
    } while (obj != nullptr);
    return r;
}

//CG: exception-emit f

static Lookup::Item const exceptionMap [] = {
    //CG: exception-emit h
};

Lookup const Exception::bases (exceptionMap);

//CG: wrappers

static Lookup::Item const builtinsMap [] = {
    // exceptions must be first in the map, see Exception::findId
    //CG: exception-emit d
    //CG: type-builtin
    //CG: builtins
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

//CG: type-info
