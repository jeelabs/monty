// context.cpp - run time contexts and interpreter

#include "monty.h"
#include "ops.h"
#include <cassert>

using namespace monty;

volatile uint32_t Interp::pending;
uint32_t          Interp::queueIds;
List              Interp::tasks;
Dict              Interp::modules;
Context*          Interp::context;

#if 0
Callable::Callable (Value callee, Tuple* t, Dict* d, Module* mod)
        : mo (mod != nullptr ? *mod : Interp::context->globals()),
          bc (callee.asType<Bytecode>()), pos (t), kw (d) {
}

auto Callable::call (ArgVec const& args) const -> Value {
    auto ctx = Interp::context;
    auto coro = bc.isGenerator();
    if (coro)
        ctx = new Context;

    ctx->enter(*this);

    int nPos = bc.numArgs(0);
    int nDef = bc.numArgs(1);
    int nKwo = bc.numArgs(2);
    int nc = bc.numCells();

    for (int i = 0; i < nPos + nc; ++i)
        if (i < args.num)
            ctx->fastSlot(i) = args[i];
        else if (pos != nullptr && (uint32_t) i < nDef + pos->fill)
            ctx->fastSlot(i) = (*pos)[i+nDef-nPos];

    if (bc.hasVarArgs())
        ctx->fastSlot(nPos+nKwo) =
            Tuple::create({args.vec, args.num-nPos, args.off+nPos});

    uint8_t const* cellMap = bc.start() - nc;
    for (int i = 0; i < nc; ++i) {
        auto slot = cellMap[i];
        ctx->fastSlot(slot) = new Cell (ctx->fastSlot(slot));
    }

    return coro ? ctx : Value {};
}

void Callable::marker () const {
    mo.marker();
    mark(bc);
    mark(pos);
    mark(kw);
}
#endif

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

void Context::enter (Callable const& func) {
    auto frameSize = 0;//XXX func.bc.fastSlotTop() + EXC_STEP * func.bc.excLevel();
assert(false);
    int need = (frame().stack + frameSize) - (begin() + base);

    auto curr = base;           // current frame offset
    base = fill;                // new frame offset
    insert(fill, need);         // make room

    auto& f = frame();          // new frame
    f.base = curr;              // index of (now previous) frame
    f.spOff = spOff;            // previous stack index
    f.ipOff = ipOff;            // previous instruction index
    f.callee = callee;          // previous callable

    spOff = f.stack-begin()-1;  // stack starts out empty
    ipOff = 0;                  // code starts at first opcode
    callee = &func;             // new callable context
    f.ep = 0;                   // no exceptions pending
}

Value Context::leave (Value v) {
    auto& f = frame();
    auto r = f.result;          // stored result
    if (r.isNil())              // use return result if set
        r = v;                  // ... else arg

    if (base > NumSlots) {
        int prev = f.base;      // previous frame offset
        spOff = f.spOff;        // restore stack index
        ipOff = f.ipOff;        // restore instruction index
        callee = &f.callee.asType<Callable>(); // restore callee

        assert(fill > base);
        remove(base, fill - base); // delete current frame

        assert(prev >= 0);
        base = prev;            // new lower frame offset
    } else {
        // last frame, drop context, restore caller
        Interp::context = caller().ifType<Context>();
        auto n = Interp::findTask(*this);
        if (n >= 0)
            Interp::tasks.remove(n);

        fill = NumSlots; // delete stack entries
        adj(NumSlots); // release vector
    }

    return r;
}

#if 0 //XXX
auto Context::excBase (int incr) -> Value* {
    uint32_t ep = frame().ep;
    frame().ep = ep + incr;
    if (incr <= 0)
        --ep;
    return frame().stack + callee->bc.fastSlotTop() + EXC_STEP * ep;
}
#endif

void Context::raise (Value exc) {
    if (Interp::context == nullptr) {
#if 0 //XXX
        Buffer buf; // TODO wrong place: bail out and print exception details
        buf.print("uncaught exception: ");
        exc.obj().repr(buf);
        buf.putc('\n');
#else
        assert(false);
#endif
        return;
    }

    uint32_t num = 0;
    if (exc.isInt())
        num = exc;              // trigger soft-irq 1..31 (interrupt-safe)
    else
        event() = exc;          // trigger exception or other outer-loop req

    Interp::interrupt(num);     // set pending, to force inner loop exit
}

void Context::caught () {
    auto e = event();
    if (e.isNil())
        return; // there was no exception, just an inner loop exit
    event() = {};

    auto& einfo = e.asType<Exception>().extra();
    einfo.ipOff = ipOff;
    einfo.callee = callee;

    if (frame().ep > 0) { // simple exception, no stack unwind
        auto ep = excBase(0);
        ipOff = ep[0] & (FinallyTag - 1);
        spOff = ep[1];
        begin()[++spOff] = e.isNil() ? ep[2] : e;
    } else {
        leave();
        raise(e);
    }
}

auto Context::next () -> Value {
    assert(fill > 0); // can only resume if not ended
    Interp::resume(*this);
    return {}; // no result yet
}

auto Interp::findTask (Context& ctx) -> int {
    for (auto& e : tasks)
        if (&e.obj() == &ctx)
            return &e - tasks.begin();
    return -1;
}

auto Interp::getQueueId () -> int {
    static_assert (NUM_QUEUES <= 8 * sizeof pending, "NUM_QUEUES too large");

    for (uint32_t id = 1; id < NUM_QUEUES; ++id) {
        auto mask = 1U << id;
        if ((queueIds & mask) == 0) {
            queueIds |= mask;
            return id;
        }
    }
    return -1;
}

void Interp::dropQueueId (int id) {
    auto mask = 1U << id;
    assert(queueIds & mask);
    queueIds &= ~mask;

    // deal with tasks pending on this queue
    for (auto e : tasks) {
        auto& ctx = e.asType<Context>();
        if (ctx.qid == id)
            ctx.qid = 0; // make runnable again TODO also raise an exception?
    }
}

void Interp::markAll () {
    //printf("\tgc started ...\n");
    assert(context != nullptr);
    mark(context); // TODO
    assert(findTask(*context) >= 0);
}

void Interp::suspend (uint32_t id) {
    assert(id < NUM_QUEUES);

    assert(context != nullptr);
    assert(findTask(*context) >= 0);
    auto ctx = context;
    context = ctx->caller().ifType<Context>();
    //ctx->caller() = {};

    ctx->qid = id;
}

void Interp::resume (Context& ctx) {
    assert(context != &ctx);
    assert(ctx.caller().isNil());
    ctx.caller() = context;
    context = &ctx;
}

void Interp::exception (Value v) {
    assert(!v.isInt());
    assert(context != nullptr);
    context->raise(v);
}

void Interp::interrupt (uint32_t num) {
    assert(num < NUM_QUEUES);
    // see https://gcc.gnu.org/onlinedocs/gcc/_005f_005fatomic-Builtins.html
    __atomic_fetch_or(&pending, 1U << num, __ATOMIC_RELAXED);
}

auto Interp::nextPending () -> int {
    if (pending != 0)
        for (uint32_t num = 0; num < NUM_QUEUES; ++num)
            if (pendingBit(num))
                return num;
    return -1;
}

auto Interp::pendingBit (uint32_t num) -> bool {
    assert(num < NUM_QUEUES);
    auto mask = 1U << num;
    // see https://gcc.gnu.org/onlinedocs/gcc/_005f_005fatomic-Builtins.html
    return (__atomic_fetch_and(&pending, ~mask, __ATOMIC_RELAXED) & mask) != 0;
}
