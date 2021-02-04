// stack.cpp - events and stacklets

#include "monty.h"
#include <cassert>
#include <csetjmp>

#if NATIVE
namespace machine { void timerHook (); }
#define INNER_HOOK  { machine::timerHook(); }
#else
#define INNER_HOOK
#endif

using namespace monty;

Stacklet* Stacklet::current;
void* Stacklet::resumer;
uint32_t volatile Stacklet::pending;
int Event::queued;

Vector monty::handlers;
List monty::tasks;

void monty::gcNow () {
    mark(Stacklet::current);
    tasks.marker();
    markVec(handlers);
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

auto Stacklet::binop (BinOp op, Value rhs) const -> Value {
    assert(op == BinOp::Equal);
    return Value::asBool(rhs.isObj() && this == &rhs.obj());
}

void Stacklet::fail (Value v) {
    v.dump();
    assert(false); // TODO unhandled exception, can't continue
}

void Stacklet::yield (bool fast) {
    assert(current != nullptr);
    if (fast) {
        if (pending == 0)
            return; // don't yield if there are no pending handlers
        tasks.push(current);
        current = nullptr;
        suspend();
    } else
        suspend(tasks);
}

void Stacklet::suspend (Vector& queue) {
    assert(current != nullptr);
    if (&queue != &handlers) // special case: used as "do not append" marker
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
        longjmp(*(jmp_buf*) resumer, 1);
    }

    // resuming: copy stack back in
    assert(Stacklet::resumer != nullptr);
    assert(Stacklet::current != nullptr);
    need = (uint8_t*) Stacklet::resumer - (uint8_t*) &top;
    duff(&top, Stacklet::current->end(), need);
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
        for (uint32_t i = 1; flags != 0 && i < handlers.size(); ++i) {
            auto bit = 1<< i;
            if (flags & bit) {
                if (!handlers[i].isNil())
                    handlers[i].asType<Event>().set();
                flags &= ~bit;
            }
        }

        if (gcCheck())
            gcNow();

        if (tasks.size() == 0)
            break;

        current = (Stacklet*) &tasks.pull().obj();
        assert(current != nullptr);

        if (current->cap() > current->fill + sizeof (jmp_buf) / sizeof (Value))
            longjmp(*(jmp_buf*) current->end(), 1);

        // FIXME careful, this won't pick up pending events while looping
        while (current->run()) {
            if (pending != 0)
                printf("pending %08x\n", pending);
        }
        if (current == nullptr)
            break;
        current->adj(current->fill);
    }

    return Event::queued > 0;
}

void monty::exception (Value v) {
    assert(Stacklet::current != nullptr);
    Stacklet::current->fail(v);
}

auto Event::regHandler () -> uint32_t {
    auto n = handlers.find({});
    if (n >= handlers.size())
        handlers.insert(n);
    handlers[n] = this;
    return n;
}

void Event::deregHandler () {
    auto n = handlers.find(this);
    if (n < handlers.size())
        handlers[n] = {};
}

void Event::set () {
    value = true;
    if (size() > 0) {
        // insert all entries at head of tasks and remove them from this event
        tasks.insert(0, size());
        memcpy(tasks.begin(), begin(), size() * sizeof (Value));
        queued -= size();
        assert(queued >= 0);
        remove(0, size());
    }
}

void Event::wait () {
    if (!value) {
        ++queued;
        Stacklet::suspend(*this);
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
