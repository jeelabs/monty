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
    markVec(Event::triggers);
    mark(current);
    tasks.marker();
    tasks.marker();
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
        case 0: do { *to++ = *from++;
        case 7:      *to++ = *from++;
        case 6:      *to++ = *from++;
        case 5:      *to++ = *from++;
        case 4:      *to++ = *from++;
        case 3:      *to++ = *from++;
        case 2:      *to++ = *from++;
        case 1:      *to++ = *from++;
                } while (--n > 0);
    }
#endif
}

void Event::deregHandler () {
    if (id >= 0) {
        assert(&triggers[id].obj() == this);
        triggers[id] = {};
        set(); // release queued tasks
        id = -1;
    }
}

void Event::set () {
    value = true;
    auto n = queue.size();
    if (n > 0) {
        // insert all entries at head of tasks and remove them from this event
        Stacklet::tasks.insert(0, n);
        memcpy(Stacklet::tasks.begin(), queue.begin(), n * sizeof (Value));
        if (id >= 0)
            queued -= n;
        assert(queued >= 0);
        queue.remove(0, n);
    }
}

void Event::wait () {
    if (!value) {
        if (id >= 0)
            ++queued;
        Stacklet::suspend(queue);
    }
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
    assert(args.num == 0);
    return new Event;
}

Stacklet::~Stacklet () {}

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

    current->adj(current->fill + need / sizeof (Value));
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

        if (current->cap() > current->fill + sizeof (jmp_buf) / sizeof (Value))
            longjmp(*(jmp_buf*) current->end(), 1);

        // FIXME careful, this won't pick up pending events while looping
        while (current->run()) {}

        if (current != nullptr) {
            current->adj(current->fill);
            assert(tasks.find(current) >= tasks.size());
            tasks.push(current);
        }
    }

    return Event::queued > 0 || tasks.size() > 0;
}

auto Event::regHandler () -> uint32_t {
    id = triggers.find({});
    if (id >= (int) triggers.size())
        triggers.insert(id);
    triggers[id] = this;
    return id;
}

auto BoundMeth::call (ArgVec const& args) const -> Value {
    assert(args.num > 0 && this == &args[-1].obj());
    args[-1] = self; // overwrites the entry before first arg
    return meth.call({args.vec, (int) args.num + 1, (int) args.off - 1});
}

Closure::Closure (Object const& f, ArgVec const& args)
        : func (f) {
    insert(0, args.num);
    for (int i = 0; i < args.num; ++i)
        begin()[i] = args[i];
}

auto Closure::call (ArgVec const& args) const -> Value {
    int n = size();
    assert(n > 0);
    Vector v;
    v.insert(0, n + args.num);
    for (int i = 0; i < n; ++i)
        v[i] = begin()[i];
    for (int i = 0; i < args.num; ++i)
        v[n+i] = args[i];
    return func.call({v, n + args.num});
}
