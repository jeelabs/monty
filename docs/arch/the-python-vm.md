# The Python VM

The Python Virtual Machine is what drives the development of Monty. Most of
Monty's features exist to support the VM and Python's data structures.

## Bytecode

The VM runs bytecode, as emitted by MicroPython's `mpy-cross` tool, which is
portable and can be used on Windows, MacOS, and Linux. This `hello.py` file:

```
import sys
print('hello', sys.implementation, sys.version)
```

Will be turned into the following binary data by calling `mpy-cross.py`:

```text
$ xxd hello.mpy
00000000: 4d05 021f 2081 1c18 0c00 0700 0128 0080  M... ........(..
00000010: 511b 0673 7973 1601 1100 7b10 0a68 656c  Q..sys....{..hel
00000020: 6c6f 1103 131c 696d 706c 656d 656e 7461  lo....implementa
00000030: 7469 6f6e 1103 130e 7665 7273 696f 6e34  tion....version4
00000040: 0359 5163 0000                           .YQc..
```

These files always start with the bytes 0x4D (ASCII "M") and 0x05 (i.e. file
format 5). The file can be seen to contain the strings "sys", "hello",
"implementation", and "version" - but not "import" (which is compiled to an
opcode, or "print" (which is one of the 150+ built-in symbols.

The `.mpy` files are relatively complex, due to a range of tricks used to try
and keep the final size minimal.  Each `def` and other sequence of Python
statements is stored as a separate "bytecode snippet". These form a tree
structure because classes and defs can all nest. There is a separate `loader.h`
source file in Monty which takes raw bytes, and decodes them into a form
suitable for execution by the Monty VM.

## Quick strings

Another aspect of the bytecode, is that it works with [string
interning](String_interning) as a way to avoid duplication of strings and to
speed up lookups: when you know that a string can only exist in a single copy in
the entire system, then string equality is a matter of simplify comparing
pointers (or a table index).

As part of the build of Monty itself, a code generator will scan through all the
strings in the source code, assign an ID to each unique string, and build a
table which is used at runtime to support this quick lookup and matching. Like
MicroPython, Monty calls them _quick strings_ or "qstrs".

But that's just half the story: when a bytecode file is loaded and prepared for
execution, all the qstr's in that file are also matched to the existing qstr
table, and replaced in the executable code version created in RAM. All _new_
qstrs are then re-assigned new unique IDs and added to a separate in-memory
table.  This ensures that at any point in time, all running code uses the same
consistent ID numbering for all quick strings.

Due to this bytecode re-numbering when loading a file, and because each file is
independently compiled, it cannot currently be executed directly out of
read-only memory.

## Interpreting opcodes

Apart from the above, the bytecode is fairly straightforward. An "inner loop" in
Monty keeps track of the _instruction pointer_ and _stack pointer_, and then a
big switch statement performs the task indicated by each opcode. Some opcodes
are followed by one or more data bytes. This is the software equivalent of a
CPU: fetch, decode, and execute each "instruction", _ad infinitum_. With as main
property that the instruction set is entirely geared towards Python operations.

Calling a function or method is done by reserving a "call frame" section on the
current VM stack context (the interpreter stack that is, _not_ the C stack!).
These VM stacks are implemented on top of Monty's `Vector` type, and can be
expanded as needed.  Many VM opcodes are stack-based, performing various
operations on a small stack within each frame, and they never need to grow the
VM stack since the proper amount of space has been pre-calculated by the
bytecode compiler.

The inner loop ends with a check of a _volatile_ global variable, called
`Stacklet::pending`. If zero, it loops, else it exits. This is the _only_ way in
which Monty's VM can stop interpreting bytecodes - there are no returns or other
exists, and there are no non-local jumps (other than "stacklets", which are
described elsewhere).

The `pending` variable is used as a set of 32 flag bits, and because they are
volatile, they can also be set from hardware interrupt handlers. This is how h/w
interrupts _ask_ for attention: the VM inner loop exits, and control returns to
a point where such "soft interrupts" are dealt with.

## Garbage collection

The garbage collector does two things in Monty: it finds and releases
unreachable object memory for re-use, and it will compact vector space to make
optimal use of RAM when vectors are resized.

Vector compaction is driven by some heuristices and automatic, but it requires
great care, since it can move vector data around and invalidate all pointers
(in)to these vectors.

Since the VM uses vectors for its interpreter stack state and for bytecode,
these too could move when compaction is triggered. This is avoided by adhering
to a strict - _and fairly severe_ - rule:

> The garbage collector may never be called from the VM's "inner loop".

Instead, when space runs low, one of the bits in `pending` is set, which forces
the VM out of its inner loop as soon as possible. Once ouf of this loop, all
pointers into the stack (and into the bytecode, which is also a vector) are
saved as _offsets_ instead of pointers, and then the GC is free to do its thing.

!> Pointers **into** vector data can (and usually will) become invalid after a
GC cycle.

## Coroutine switching

There is a second loop in the VM, called ... very predictably: the "outer loop".
it handles everything that could not be done inside the inner loop: run the GC
if requested, decide how to deal with other pending-bit requests, and - if no
other action is needed - re-enter the inner loop again, to keep bytecode
execution running as much as possible.

This is also where coroutine switching takes place. In Monty, each coroutine has
its own growable stack (i.e. `Vector`). Or more accurately: each coroutine is a
`Stacklet`, which is derived from `Vector`. Stacklets are special in that they
can be suspended at the C++ level. This allows the VM to suspend in a very
fundamental way (waiting for I/O, for example), while allowing another coroutine
to resume execution.

Switching stacklet contexts is done at yet another level, _outside_ the Python
VM, in the stacklet `runLoop()` code. To make this happen, the VM saves and
adjusts all relevant data structures and _returns_ to its caller. This is why
Monty's VM is called "stackless": it does not nest at the C level, it returns to
its caller to perform context switching, by resuming a different stacklet. That
stacklet may well be a Python coroutine, in which case the VM will be entered
afresh.

To clarify the distinction: "stacklets" in Monty are a mechanism implemented in
C++, with the Python VM called inside if this context is about running bytecode.
The Python VM is not really aware of running as a stacklet. It just gets called
to execute some bytecode, and returns when that bytecode has finished.

Stacklets can also be used outside of the VM, in pure C++ code.
