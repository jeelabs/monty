# Stacklets & coros

A **stacklet** is a freshly-baked term used in Monty to describe "a concurrent
C++ task", similar to a [green
thread](https://en.wikipedia.org/wiki/Green_threads) or a
[goroutine](https://golangr.com/goroutines/) in Go. Stacklets enable
_collaborative_ multi-tasking, i.e. switching contexts _voluntarily_.

The underlying mechanism uses `setjmp` & `longjmp` from the standard C library
and is - in principle - completely portable (all the hard works is done in those
two functions). There is no assembly code involved, it's all implemented in just
150 lines of C++ code, but the internal details are suprisingly tricky.

Stacklets differ from conventional multitasking in that there is only **one C
stack**, which is never switched. Instead, stacklets copy their C stack context
**out of** and **in to** the current C stack when they switch. This leads to
some copying overhead and has some limitations, but it has the benefit that
there is no need to allocate one stack area for each stacklet, which would have
to be sized for worst-case stack consumption. Instead, copying now uses the
dynamic nature of Monty's vectors, which can grow and shrink as needed, and get
compacted when memory runs low.

?> It may be possible to reimplement stacklets to use real (RT)OS tasks, but
this has not been tried.

What stacklets bring to Monty, is the ability to _block_ and _suspend_ in C++
and switch to another stacklet.

Note that stacklets are a C++ mechanism and can be used anywhere in the C++
code. The Python VM's "tasks" and "coroutines" do not need stacklets to switch
_between_ them, but by turning them into stacklets, the switching can now
include blocking at the C++ level. In other words: without stacklets, a blocking
I/O call will block the entire VM (and hence all of Python), whereas with
stacklets, Monty can now block a task/coro and resume another one.

!> Unlike CPython and MicroPython, Monty can suspend and switch Python tasks /
coros _without_ the `async`/`await` mechanism. This is similar to Go and was one
of Monty's main design goals.

The main restriction with stacklets is that only one of them can run (i.e. it's
single-CPU, and that all stacklet context switches occur in a single part of the
application. The reason for this is that the _base_ of the C stack must always
remain the same for the copy-out/-in to work. This is easily enforced by making
sure that `Stacklet::runLoop()` only appears _once_ in the entire C++
application.

Since stacklets are a concerrency mechanism, they need a way to synchronise in a
robust way. This is accomplished with `Event` objects (as in Python's `asyncio`).
A stacklet can "wait" on an event, "set" it, or "clear" it. When waiting on an
event which is not currently set, the stacklet is placed on the event's queue,
and suspended. Then the first stacklet on the `ready` queue is resumed.

When there are no more stacklets ready to run, the `runLoop()` call returns. The
main application should have code which looks like this:

```
while (Stacklet::runLoop()) {
    // idle
}
```

On an embedded system, "idling" could be to enter sleep mode (e.g. `asm
("wfi")`), on a native host a brief delay could be used to avoid pegging (one
of) its CPU(s) at 100%.

Once the above loop exits, this means that there are no stacklets which can run
_and_ that there is nothing which might trigger a stacklet to resume later. The
only thing left to do is to power-down (or exit).

## Hardware interrupts

Stacklets are collaborative, which means they don't really care about hardware
interrupts, they just pass control between them as they wish.  Still, in
embedded context, hardware interrupts are a key feature.

The way Monty deals with this is with a global variable, declared as `volatile
uint32_t pending;` and used as 32 individually-settable bits.  This is done
_atomically_, i.e. setting / clearing these bits is interrupt safe.  On
architectures which support it, this is done without disabling / enabling
hardware interrupts.

Hardware interrupts do not affect the stacklets directly. All they can do is
_ask_ for attention, i.e. a voluntary context switch. In the VM, this happens
after every opcode, as a form of "soft interrupt". In other stacklets, it's
really up to their implementation to stay receptive to such requests.

Once a stacklet relinquishes control, the run loop checks all pending bits, and
triggers their associated events (i.e. "sets" them). This in turn will make all
waiting stacklets runnable again, so when the run loop continue, it will start
each of these stacklets in sequence. If new pending bits are set, the process
repeats itself, always causing the triggered stacklets to run first.

Due to the delay between a hardware interrupt routine _asking_ for attention,
and the actual stacklet dealing with the request, there are clearly limits on
the reponsiveness in the entire system.

The proper way to deal with this is:

1. split all hardware driver code in a "lower" and an "upper" half
2. the lower half is the raw (and time-critical) hardware code which runs at
   interrupt time, i.e.  immediately
3. it must save all time-critical data (e.g. incoming UART bytes) in a buffer
   which is large enough to store everything until the upper half gets to run
4. the upper half runs in stacklet context, and _apart from its interaction with
   the lower-half interrupt code_, has no time-critical or atomicity
   requirements
5. the rest of the system need not be concerned with atomicity, potential race
   conditions, etc

The relation between events and pending bits is defined at run time: events can
"register" and "deregister" to aquire a unique id, i.e. a value in the range
1..31 which identifies the pending bit.

Keep in mind is that as long as any _registered_ event has stacklets queued up,
the run loop will return true, i.e. loop in main. The way to have the main
program end normally, is to deregister all events so that no source of
interrupts can possibly resume a suspended stacklet: timer ticks, serial ports,
everything!
