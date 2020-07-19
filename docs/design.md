?> Monty was started in mid-June, 2020 - it's as young and temperamental as a
puppy. Everything in the following notes should be viewed as no more than a
snapshot of an evolving experimental design.

## Data structures

There are two types of data structures in Monty: objects and vectors, with
(surprise!) `Object` and `Vector` as the names of their corresponding C++ base
classes.

**Objects** are fairly small (typically in the 10's of bytes) and are allocated
at a fixed memory location. Objects can point to other objects and to vectors.
There is an entire class hierarchy, sharing a number of common virtual methods,
each derived class implementing various bits of functionality. Objects can be
used from Python code.

**Vectors** consist of two parts: a small part resides inside objects, but the
main part is an indexable & growable contiguous byte range, allocated in a
separate memory area. Vectors can grow and shrink (when grown, they may get
copied to a new spot). Vectors are not visible as distinct entities from Python
code.

Objects are not vectors, but they can _contain_ vectors (their "small part",
that is). Likewise, vectors are not objects, but they may _contain_ (lists of)
pointers to objects.

Almost every C++ data structure in Monty is either an object or a vector.

## Object hierarchy

The C++ class hierarchy for `Object` is as follows (a few parts omitted for
brevity):

```
Object
    NoneObj
    BoolObj
    IntObj
    SeqObj
        BytesObj
            StrObj
        TupleObj
        LookupObj
        MutSeqObj
            ArrayObj
            ListObj
                SetObj
            DictObj
                TypeObj
                    ClassObj
                InstanceObj
                FrameObj
                ModuleObj
    IterObj
    FunObj
    BoundMethObj
    BytecodeObj
    CallArgsObj
    ResumableObj
    Context
        Interp
```

## Memory management

Monty uses stop-the-world mark & sweep garbage collection to manage all its
memory.  Every object (i.e. everything in the above hierarchy) is allocated from
a fixed memory pool and, when triggered, the garbage collector will 1) mark all
reachable objects, and 2) sweep through the pool, freeing all unmarked objects.

Vectors are allocated from a separate memory pool, from the bottom up, in a
stack-like fashion. When memory runs low, all vectors are compacted downwards,
adjusting the pointers to them in their "owning" objects. Once compacted,
subsequent new (or growing) vectors will be allocated as a growing stack again.

Object- and vector-space are currently distinct fixed-size memory arrays, with
both garbage collection steps always performed in combination: first, gc mark &
sweep gets rid of unreachable objects (and any vectors they contain), then the
compaction step reclaims unused vector space.

## Loops all the way down

The Monty VM runs as a nest of four separate loops. From inner- to outermost:

1. the **inner** loop interprets bytecodes as long as it can - it won't switch
   contexts, nor collect garbage
2. the **outer** loop gets control when there is a "soft IRQ", an exception, or
   a garbage collection trigger
3. the **run** loop gets control when a task is suspended, and starts the next
   available task while it can
4. the **main** loop cycles when there is no work, and can go to sleep until a
   hardware IRQ wakes it up

There are no goto's in Monty and no `setjmp/longjmp` calls. Four loops, that's
it.

## Stackless VM

The Monty VM uses a "stackless" design. This term is a bit confusing: of course
the C++ implementation and the VM's interpreter state both need a stack. What it
means in Monty, is that the C++ and VM stacks are carefully kept apart and do
not "get entangled". The primary mechanism is that the VM can call C++ code (how
else could it operate?), but C++ can _only_ call the VM from the "main loop"
mentioned above.

This is a far-reaching design choice, with major implications. Including some
inconvenient ones ...

The main benefit is that the that VM, i.e. bytecode execution, can be suspended
at any point between opcodes. The VM can call whatever C++ code it wants, but it
will only proceed to the next bytecode instruction when that code returns, i.e.
when the C stack is back to the state at the start of the call.

In Monty, Python scripts can suspend and resume at will, and this includes
blocking to wait for "slow" I/O operations to complete. While blocked, other
tasks (i.e. other bytecode contexts) can continue to run. There is no need to
sprinkle Python 3's recent `async` and `await` keywords all over the code. A
read or write will simply block until it can be serviced.  This is just as in
Go, with its "goroutines". They run independently, and block when they have to.
For a quick intro into this approach, see Rob Pike's
[slides](https://talks.golang.org/2012/concurrency.slide) from 2012.

The reason this works in Monty, is that everything happens in a purely stacking
LIFO fashion: main calls VM, VM calls C++ functions, they return, the VM
continues where it left off, and when it has nothing to do _right now_, it
returns to main.

The magic happens in the VM's (and hence Python's) _generators_. In Monty a
generator is simply a stack (implemented as a Vector), ready to grow and shrink
to handle nested bytecode function calls. Generators can "yield", i.e. suspend
themselves voluntarily, or they can be suspended by the VM and placed on the
waiting queue of an I/O driver, for example. Note that Python's `yield` can take
and/or return a value.

Generators and coroutines are nearly the same thing. They just differ in which
direction a value is used: as input parameter, or as result. From now on, let's
use the term **coro** for both, simply because it's shorter.

## Coro's and tasks

From a Python perspective, coro's are functions which contain the keyword
`yield` somewhere in their body (or `async` / `await` nowadays). These functions
are specially marked, and cause the VM to make one small but very significant
change: when called, instead of executing the corresponding bytecode, the VM
creates a new stack, sets things up so it will _eventually_ start running that
bytecode, and returns the stack _itself_ as result. Python calls this a
"generator object". It's only the _start_ of a new run context. It hasn't done
anything meaningful yet, but now you can pass it around, using `next()` to make
it run for a while.

This is a "continuation": all the info needed to do some work, as an _object_
which can be passed around.

A "normal" function takes arguments, needs some stack space, does its thing,
returns a result, and frees its stack space. Monty will handle this case by
expanding and shrinking the caller's stack (a resizable vector).

A coro on the other hand, once called, carries its current state with it, i.e.
it "owns" a stack frame which it can use and alter as often and as long as it
wants. The local variables in a coro don't get lost when it yields, but only
when it returns. Such a return is final, it also prohibits any further `next()`
calls.

A coro can call normal functions which in turn can nest to any depth. These
function will grow the coro's stack as needed. Since `yield` by its very
presence implies being a coro, such yields can only happen once all normal
functions calls have ended. Standard coro yields do not need to deal with
nesting.

A **task** is a coro _plus all the nested functions currently running._ Since
these all share the coro's stack, it's still a simple data structure which looks
like a coro on the outside. But here comes this special function called
`monty.suspend()`. It's perhaps best described as "yielding the entire stack" in
one big swoop: the complete stack state, nested functions and all, is put on
hold by the VM.

This has the effect of blocking an entire thread of execution. E.g. perhaps that
most deeply nested function just did a `read` call, which wants to block
until the requested data is ready. This creates two problems:

1. what to do with this "stack + nested call frames" data structure so it
   doesn't get garbage-collected?
2. what should the VM do next, given that it can't continue executing this
   bytecode?

Problem #1 is addressed by giving `monty.suspend()` a list. The suspended task
is appended to that list. In the case of a read, this list must be managed by
the read I/O handler, to resume it when the data arrives.

Problem #2 is handled in the third of the loops mentioned in the [Stackless
VM](#stackless-vm) section: the "run loop" pops the first item off the
`monty.tasks` list, and re-enters the outer and inner VM loops to resume that
task. If `monty.tasks` is empty, then there's no work at the moment, so the run
loop returns back to the main loop.

To summarise: when a coro yields, it takes itself (and the stack frame it owns)
out of the VM loop to let its caller continue. When a coro (or task, same thing,
essentially) is _suspended_, it is also taken out of the VM loop, but since
there is no caller left, the next work for the VM needs to come from the task
list instead.

The fact that `monty.suspend()` is not a keyword known to Python, makes that the
code calling it will not be marked as "asynchronous" or "a generator". Normal
functions can call what looks like yet another normal function, and ... get
suspended and resumed without noticing. Like a goroutine or POSIX process.

## A troublespot

The big problem with Monty's approach, is that C++ code cannot call the VM.
There is no way C++ can loop over something which _might_ need to run a bit of
bytecode. Even something as innocent-looking as `bytes(<iterator>)` to construct
a new `bytes` object from an iterator is trouble if coded in C++.

The **only** action a C++ function can take is to alter the VM state, and this
includes pushing a new bytecode call onto the VM's stack and state. So yes, C++
can _start_ arbitrary code, but it can't _wait_ for the result. To interpret
that bytecode, it _must_ return to its caller. This is the same as in
JavaScript, which is fundamentally asynchronous and uses callbacks, promises,
and futures to find a nice way around this very same issue.

The essence boils down to the fact that there is no (clean) way to implement
continuations in C/C++.

There is a way to work around this problem in Monty, using `ResumableObj` as a
base class to make state explicit. Finite state machine coding also offers a way
around this problem (this is used in the LwIP network library, for example). But
either way, such explicit coding techniques may require a lot of extra effort.

Another approach is to put more of the logic in Python, so that the required C++
calls only do fairly basic stuff and can be broken into smaller atomic steps.

Right now, it all doesn't look like a show-stopper for Monty. But "taming the
beast" will take more work ...
