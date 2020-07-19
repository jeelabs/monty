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
