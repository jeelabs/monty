// stack.cpp - events and stacklets

#include "monty.h"
#include <cassert>
#include <csetjmp>

using namespace monty;

Stacklet* Stacklet::current;
void* Stacklet::resumer;
uint32_t volatile Stacklet::pending;

Vector monty::stacklets;
Vector monty::handlers;
Vector monty::ready;

auto Event::regHandler () -> uint32_t {
    auto n = handlers.find({});
    if (n >= handlers.size())
        handlers.insert(n);
    handlers[n] = this;
    return n;
}

void Event::deregHandler () {
    auto n = id();
    if (n < handlers.size())
        handlers[n] = {};
}

void Event::set () {
    value = true;
    if (size() > 0) {
        // insert all entries at head of ready and remove them from this event
        ready.insert(0, size());
        memcpy(ready.begin(), begin(), size() * sizeof (Value));
        remove(0, size());
    }
}

void Event::wait () {
    if (value)
        return;
    assert(Stacklet::current != nullptr);
    Stacklet::suspend(*this);
}

Stacklet::Stacklet () {
    auto n = stacklets.find({});
    if (n >= stacklets.size())
        stacklets.insert(n);
    stacklets[n] = this;
    ready.append(this);
}

Stacklet::~Stacklet () {
    auto n = id();
    assert(n < stacklets.size());
    stacklets[n] = {};
}

// see https://en.wikipedia.org/wiki/Duff%27s_device
static void duff (uint32_t* to, uint32_t const* from, size_t count) {
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
}

// note: don't make this static, as then it might get inlined - see below
// TODO not sure this is still needed, it might have been an adj/cap-check bug
void resumeFixer (void* p) {
    assert(Stacklet::resumer != nullptr);
    assert(Stacklet::current != nullptr);
    auto need = (uint8_t*) Stacklet::resumer - (uint8_t*) p;
#if 0
    memcpy(p, (uint8_t*) Stacklet::current->end(), need);
#else
    duff((uint32_t*) p, (uint32_t*) Stacklet::current->end(), need/4);
#endif
}

void Stacklet::yield (bool fast) {
    assert(current != nullptr);
    if (fast) {
        if (pending == 0)
            return; // don't yield if there are no pending handlers
        ready.insert(0);
        ready[0] = current;
        suspend(handlers); // hack: used as "do not append" marker
    } else
        suspend(ready);
}

void Stacklet::suspend (Vector& queue) {
    assert(current != nullptr);
    if (&queue != &handlers) // special case: see yield above
        queue.append(current);

    jmp_buf top;
    assert(resumer > &top);

    uint32_t need = (uint8_t*) resumer - (uint8_t*) &top;
    assert(need % sizeof (Value) == 0);
    assert(need > sizeof (jmp_buf));

    current->adj(current->fill + need / sizeof (Value));
    if (setjmp(top) == 0) {
        // suspending: copy stack out and jump to caller
#if 0
        memcpy((uint8_t*) current->end(), (uint8_t*) &top, need);
#else
        duff((uint32_t*) current->end(), (uint32_t*) &top, need/4);
#endif
        longjmp(*(jmp_buf*) resumer, 1);
    }

    // resuming: copy stack back in
    // note: the key is that this calls ANOTHER function, to avoid reg issues
    resumeFixer(&top);
}

auto Stacklet::runLoop () -> bool {
    jmp_buf bottom;
    if (resumer == nullptr)
        resumer = &bottom;
    assert(resumer == &bottom); // make sure stack is always in same place

    setjmp(bottom); // suspend will always return here

    while (true) {
        auto flags = allPending();
        for (uint32_t i = 1; flags != 0 && i < handlers.size(); ++i) {
            auto bit = 1<< i;
            if (flags & bit) {
                if (!handlers[i].isNil())
                    handlers[i].asType<Event>().set();
                flags &= ~bit;
            }
        }

        if (ready.size() == 0)
            break;

        current = (Stacklet*) &ready.pull(0).obj();
        if (current->cap() > current->fill + sizeof (jmp_buf) / sizeof (Value))
            longjmp(*(jmp_buf*) current->end(), 1);

        // FIXME careful, this won't pick up pending events while looping
        while (current->run()) {}
        if (current != nullptr)
            current->adj(current->fill);
    }

    // return true if there is still at least one active interrupt handler
    for (uint32_t i = 1; i < handlers.size(); ++i)
        if (!handlers[i].isNil())
            return true;

    handlers.adj(0); // there are none, might as well clean up
    return false;
}

void Stacklet::dump () {
    for (auto e : stacklets)
        if (!e.isNil()) {
            auto& p = e.asType<Stacklet>();
            printf("st: %3d [%p] size %d cap %d ms %d sema %d\n",
                    p.id(), &p, p.size(), p.cap(), p.ms, p.sema);
        }
}
