// stack.cpp - events, stacklets, and various call mechanisms

#include "monty.h"
#include <cassert>
#include <csetjmp>

#if NATIVE
extern void timerHook ();
#define INNER_HOOK  { timerHook(); }
#else
#define INNER_HOOK
#endif

using namespace monty;

List Stacklet::tasks;
uint32_t volatile Stacklet::pending;
Stacklet* Stacklet::current;

int Event::queued;
Vector Event::triggers;

static jmp_buf* resumer;

void Stacklet::gcAll () {

    // careful to avoid infinite recursion: the "sys" module has "modules" as
    // one of its attributes, which is "Module::loaded", i.e. a dict which
    // chains to the built-in modules (see "qstr.cpp"), which includes "sys",
    // which has "modules" as one of its attributes, and on, and on, and on ...

    // this turns into infinite recursion because all these objects are static,
    // so there is no mark bit to tell the marker "been here before, skip me",
    // and the solution is simple: break this specific chain while marking
    auto save = Module::loaded._chain;
    Module::loaded._chain = nullptr;

    markVec(Event::triggers);
    mark(current);
    tasks.marker();
    save->marker();

    // restore the broken chain, now that marking is complete
    Module::loaded._chain = save;

    sweep();
    compact();
}

static void duff (void* dst, void const* src, size_t len) {
#if 0
    memcpy(dst, src, len);
#else
    // see https://en.wikipedia.org/wiki/Duff%27s_device
    auto to = (uint32_t*) dst;
    auto from = (uint32_t const*) src;
    auto count = len/4;
    auto n = (count + 7) / 8;
    switch (count % 8) {
        case 0: do { *to++ = *from++; // fall through
        case 7:      *to++ = *from++; // fall through
        case 6:      *to++ = *from++; // fall through
        case 5:      *to++ = *from++; // fall through
        case 4:      *to++ = *from++; // fall through
        case 3:      *to++ = *from++; // fall through
        case 2:      *to++ = *from++; // fall through
        case 1:      *to++ = *from++;
                } while (--n > 0);
    }
#endif
}

void Event::deregHandler () {
    if (_id >= 0) {
        assert(&triggers[_id].obj() == this);
        triggers[_id] = {};
        set(); // release queued tasks
        _id = -1;
    }
}

void Event::set () {
    _value = true;
    auto n = _queue.size();
    if (n > 0) {
        // insert all entries at head of tasks and remove them from this event
        Stacklet::tasks.insert(0, n);
        memcpy(Stacklet::tasks.begin(), _queue.begin(), n * sizeof (Value));
        if (_id >= 0)
            queued -= n;
        assert(queued >= 0);
        _queue.remove(0, n);
    }
}

void Event::wait () {
    if (!_value) {
        if (_id >= 0)
            ++queued;
        Stacklet::suspend(_queue);
    }
}

auto Event::regHandler () -> uint32_t {
    _id = triggers.find({});
    if (_id >= (int) triggers.size())
        triggers.insert(_id);
    triggers[_id] = this;
    return _id;
}

auto Event::unop (UnOp op) const -> Value {
    assert(op == UnOp::Boln);
    return Value::asBool(*this);
}

auto Event::binop (BinOp op, Value rhs) const -> Value {
    assert(op == BinOp::Equal);
    return Value::asBool(rhs.isObj() && this == &rhs.obj());
}

auto Event::create (ArgVec const& args, Type const*) -> Value {
    assert(args._num == 0);
    return new Event;
}

auto Event::repr (Buffer& buf) const -> Value {
    return Object::repr(buf); // don't print as a list
}

auto Stacklet::binop (BinOp op, Value rhs) const -> Value {
    assert(op == BinOp::Equal);
    return Value::asBool(rhs.isObj() && this == &rhs.obj());
}

void Stacklet::raise (Value v) {
    v.dump();
    assert(false); // TODO unhandled exception, can't continue
}

void Stacklet::exception (Value e) {
    assert(current != nullptr);
    current->raise(e);
}

void Stacklet::yield (bool fast) {
    assert(current != nullptr);
    if (fast) {
        if (pending == 0)
            return; // don't yield if there are no pending triggers
        assert(tasks.find(current) >= tasks.size());
        tasks.push(current);
        current = nullptr;
        suspend();
    } else
        suspend(tasks);
}

void Stacklet::suspend (Vector& queue) {
    assert(current != nullptr);
    if (&queue != &Event::triggers) // special case: use as "do not append" mark
        queue.append(current);

    jmp_buf top;
    assert(resumer > &top);

    uint32_t need = (uint8_t*) resumer - (uint8_t*) &top;
    assert(need % sizeof (Value) == 0);
    assert(need > sizeof (jmp_buf));

    current->adj(current->_fill + need / sizeof (Value));
    if (setjmp(top) == 0) {
        // suspending: copy stack out and jump to caller
        duff(current->end(), &top, need);
        longjmp(*resumer, 1);
    }

    // resuming: copy stack back in
    assert(resumer != nullptr);
    assert(current != nullptr);
    need = (uint8_t*) resumer - (uint8_t*) &top;
    duff(&top, current->end(), need);
}

auto Stacklet::runLoop () -> bool {
    jmp_buf bottom;
    if (resumer == nullptr)
        resumer = &bottom;
    assert(resumer == &bottom); // make sure stack is always in same place

    setjmp(bottom); // suspend will always return here

    while (true) {
        INNER_HOOK

        auto flags = allPending();
        for (uint32_t i = 1; flags != 0 && i < Event::triggers.size(); ++i)
            if ((flags & (1U<<i)) && !Event::triggers[i].isNil())
                Event::triggers[i].asType<Event>().set();

        if (tasks.size() == 0)
            break;

        current = (Stacklet*) &tasks.pull().obj();
        assert(current != nullptr);

        if (current->cap() > current->_fill + sizeof (jmp_buf) / sizeof (Value))
            longjmp(*(jmp_buf*) current->end(), 1);

        // FIXME careful, this won't pick up pending events while looping
        while (current->run()) {}

        if (current != nullptr) {
            current->adj(current->_fill);
            assert(tasks.find(current) >= tasks.size());
            tasks.push(current);
        }
    }

    return Event::queued > 0 || tasks.size() > 0;
}

auto Stacklet::repr (Buffer& buf) const -> Value {
    return Object::repr(buf); // don't print as a list
}

auto BoundMeth::call (ArgVec const& args) const -> Value {
    assert(args._num > 0 && this == &args[-1].obj());
    args[-1] = _self; // overwrites the entry before first arg
    return _meth.call({args._vec, (int) args._num + 1, (int) args._off - 1});
}

Closure::Closure (Object const& f, ArgVec const& args)
        : _func (f) {
    insert(0, args._num);
    for (int i = 0; i < args._num; ++i)
        begin()[i] = args[i];
}

auto Closure::call (ArgVec const& args) const -> Value {
    int n = size();
    assert(n > 0);
    Vector v;
    v.insert(0, n + args._num);
    for (int i = 0; i < n; ++i)
        v[i] = begin()[i];
    for (int i = 0; i < args._num; ++i)
        v[n+i] = args[i];
    return _func.call({v, n + args._num});
}

auto Closure::repr (Buffer& buf) const -> Value {
    return Object::repr(buf); // don't print as a list
}

auto Module::repr (Buffer& buf) const -> Value {
    buf.print("<module '%s'>", (char const*) _name);
    return {};
}
