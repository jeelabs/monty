The central source file in Monty is `lib/monty/monty.h`. This contains all the
key datatypes of the system. These notes act as annotations for this header, but
not always in the same order as the source code.

This is C++ code, even though most types are defined as `struct`, not `class`.
The distinction is minimal: members are public by default in structs, but
private in classes. That's really the only difference!

## `struct Value`

A `Value` encodes some very frequently-used, eh, _values_ in a single efficient
machine word (32- or 64-bit, depending on architecture). It represents one of
four C types:

```cpp
struct Value {
    enum Tag { Nil, Int, Str, Obj };
    ...
};
```

* **Value::Nil** is a "non-value", it's never visible in Python
* **Value::Int** is a small signed integer with approx. ±1,000,000,000 range on
  32-bit
* **Value::Str** is a pointer to a null-terminated C string, i.e. a `const
  char*`
* **Value::Obj** is a pointer to any instance of some `Object`-derived class

Values are passed by value (seems obvious, but it must be stressed). A `Value`
will fit in a machine register and is as efficient as passing around an `int` or
a `void*`. There's a _small_ overhead to wrap / unwrap it.

Values are easy to create: pass an `int`, C string, or object pointer where a
Value is expected, and C++ will take care of it.  When used as `int` or
C-string, C++'s automatic casting again makes things easy.

Values can be checked with `v.isNil()`, `v.isInt()`, `v.isStr()`, and
`v.isObj()`, which return a `bool`.

With `v.ifType<ListObj>()` (a template function, hence the `<>`'s), you get a
pointer of the requested object type back (or a null pointer if there's a type
mismatch).  For strict use, there's `v.asType<ListObj>()` which returns a C++
_reference_, or else fails in an assertion if the type doesn't match.

The `v.dump()` method is a convenient way to display a value on the console for
debugging. If given a string arg, it prefixes the output with that string and
adds a newline at the end.

Values are _everywhere_ in Monty. They are frequently used as function arguments
and as return type.

## `struct Vector`

The `Vector` is Monty's way of managing variable-sized indexable data. They hold
a variable number of N-bit items (N being anything from 1 to at least 64).
Vectors have a `length()` (the number of items) and a `capacity` (the size in
bytes to which a vector can grow without reallocation).

Vector items can be fetched (via `getXXX()` methods), and stored (via `set()`).
For items of at least 8 bits, there is `ins()` and `del()` to insert or remove
items, moving other items up or down as needed. If insertion needs more space
than the vector's capacity allows, the vector will be re-allocated.

## `struct VecOf<T>`

This is a convenience wrapper based on C++ templates, to simplify fetching and
storing with type-specifici `get()` and `set()` methods. It can be used for
vectors of characters, small ints of a specific size, etc.

The most frequent use for this is a `VecOf<Value>`, which is so common that it
has been defined as a separate type: `struct VecOfValue : VecOf<Value> { ... }`.

## `struct Object`

The `Object` class is the base class for everything exposed in Python, other
than small integers and constant C-strings. The class hierarchy derived from
this class is presented in the [Design Overview](design.md#object-hierarchy).

Objects have a number of virtual function members, which means that _every_
objects has some common methods. The most obvious ones are really just C++
"dispatchers" for some standard Python methods:

```cpp
virtual Value repr  (BufferObj&) const;
virtual Value call  (int, Value[]) const;
virtual Value unop  (UnOp) const;
virtual Value binop (BinOp, Value) const;
virtual Value attr  (const char*, Value&) const;
virtual Value at    (Value) const;
virtual Value iter  () const;
virtual Value next  ();
```

There are two other essential virtual methods: `type()` and `mark()`.

**`virtual const TypeObj& type () const`**

This defines the "type" of an instance, i.e. the runtime equivalent of the
compile-time C++ class. Through its type, Monty can access some additional
information about each object, such as its display `name`, the `create()`
factory function which can create new instances of that type, and an `attrs`
lookup table which defines the built-in attributes of each type.

**`virtual void mark (void (*gc)(const Object&)) const {}`**

<!-- *** this line just clears Vim's confused Markdown rendering state -->

This is the core of the garbage collector. For each object type, it knows how to
mark all reachable objects from that instance. I.e. for an `IntObj`, there is
nothing to mark, whereas `ListObj::mark` will mark all objects currently stored
in the instance's list (which is, perhaps confusingly ... managed in a Vector).

Also defined in the `Vector` class are these two (implied static) functions:

```cpp
void* operator new (size_t);
void operator delete (void*);
```

These take over all heap allocations for `Object` instances and all derived
classes. With a few exceptions (notably `BytesObj` and `TupleObj`), this is all
that's needed in C++ to make sure objects end up in a memory area managed by the
garbage collector.

?> Not **all** objects are allocated on the heap. Objects _can_ be declared as
local variables, on the C stack. They are cleaned up in C's usual way: when the
stack frame is left, i.e. when "local scope" ends.

## struct NoneObj, etc ...

There is only a single NoneObj instance, it's available as `Value::None`.
Similarly, there are only two `BoolObj` instances: `Value::True` and
`Value::False`.

A few other subclasses of `Object` are fairly obvious, e.g. `IntObj`, `IterObj`.
Only classes which deserve special mention are listed after this point.

## struct BytesObj

In Python, `bytes` are constant arrays of, eh, "bytes" (`uint8_t` in C).  To
efficiently deal with short byte sequences, the `BytesObj` plays some tricks
with its memory representation. For "large" byte sequences, a `Vector` is used,
but below a certain threshold (currently 16), the memory occupied but that
Vector instance is _overwritten_ with the bytes themselves (and a length).

## struct StrObj

Strings are UTF-8 aware in Python and Monty. They are derived from `BytesObj`,
but with additional logic to deal with UTF-8's variable-length encoding for
Unicode charcter 0x0080 and above.

> As of mid-July, there's only a crude non-bytes implementation, but it's on the
> TODO list ...

## struct TupleObj

A tuple is an immutable object in Python. To efficiently deal with small tuples,
Monty will allocate the values of the tuple _right after_ the memory occupied by
`TupleObj` itself. So tuples are _sequences_ (and support indexed access), but
unlike `ListObj` instances, these sequences are not managed as a `VecOfValue`.

## struct LookupObj

A "lookup object" is like a `DictObj`, i.e. a map, but the associated table is
stored is constant (i.e. `const`) so it can be stored in flash memory.

## struct MutSeqObj

This is an abstract base class for all objects which hold variable amounts of
mutable data. The base class includes a `VecOfValue`, which is used in different
ways for lists, sets, dicts, arrays, classes, and more.

Several classes are derived from `MutSeqObj`:

* `ArrayObj` - stores compact arrays of elements of the same scalar type
* `ListObj` - stores Python lists, i.e. arrays of arbitrary other objects
* `SetObj` - a collection of unique values (these must be hashable)
* `DictOb` - to store a mapping from unique keys to arbitrary objects

> Hashing is not yet implemented in the Monty VM (and "unique" is not
> well-defined).

## struct ClassObj

This type is used for Python classes. _Instances_ of these classes are of type
`InstanceObj`, with a `type()` which points back to the corresponding `ClassObj`
_instance._ It's very unfortunate but unavoidable: some of this stuff leads to
mind-twisting terminology (or mind-expanding, depending on your perspective).

> Base classes are not yet implemented in Monty. There may be some snakes hiding
in there ...

## struct FunObj

This is object type wraps a C or C++ function (_not_ a Python fuction, which is
a `BytecodeObj`).

There are three additional classes which can wrap a C++ _method_, which comes
surprisingly close to what a Python method is. These require quite a bit of
template magic to make it all work properly from C++:

```cpp
struct MethodBase {
    virtual Value call (Value self, int argc, Value argv[]) const =0;

    template< typename T >
    static Value argConv (Value (T::*meth)() const,
                            Value self, int argc, Value argv[]) { ... }
    ...
};

template< typename M >
struct Method : MethodBase { ... };

struct MethObj : Object { ... };
```

## struct BytecodeObj

This class represent bytecode objects and everything they carry with them, i.e.
call details, constants, and additional bytecode objects. Bytecode objects only
consist of constant data. They are constructed by the `.mpy` file loader on
import (see `lib/monty/loader.h`).

## struct CallArgsObj

Bytecode can not be called as is. It is always part of a context, i.e. the
module or the class where it is defined. This is also where default arguments
are specified. The `CallArgsObj` class represents a "declared" function, i.e.
with the current context and default arguments prepared for use.

## struct ModuleObj

A module owns some bytecode, which needs to be executed when the module is
imported. Most often, this bytecode will then define some variables and
functions, which end up getting stored in the module's dictionary (`ModuleObj`
is derived from `DictObj`).

## struct FrameObj

Instances of type `FrameObj` represent stack frames to hold arguments, locals,
and temporary values during bytecode execution by the VM. The top of the stack
is pointed to by `sp`, a pointer into the stack, but in stack frames, stack
pointers are saved as _offsets_ into the stack. This allows stacks to move and
resize without disturbing the stored stack of a stack frame.

The _currently active_ stack pointer is not stored in `FrameObj` instances, only
stack pointers of callers (and their callers) which are waiting for nested calls
and stack frames to return. In other words: when a call pushes the current
instruction pointer and current stack pointer to prepare for this new call,
those values are stored in the caller's `spOffset` and `savedIp`, respectively.

Frame objects don't _own_ a stack, they merely point into the stack owned by the
current `Context` instance.

Frame objects also contain pointer to the caller's frame, and a pointer to the
local dictionary used to store additional locals. In the case of class
instances, this dictionary contains the instance attributes. In the case of
modules, the frame points to the module's dictionary.

?> Some of these details are inaccurate and even incorrect. Please consider them
"still in flux".

## struct Context

The `Context` represents the VM's _current execution state_. It includes a
vector (of type `VecOfValue`) which is used as stack, expanded and shrunk as
frame objects come and go, to handle the process of executing a function or some
other bytecode (e.g. class or module setup).

Generators, coroutines, and tasks (all to be called "coro" from now on) are
implemented as stack frames pointing to the context each of them owns. As
described in the [Design overview](design.md#stackless-vm), coros _own_ a stack
(i.e. the context they point to), whereas normal functions _borrow_ (and expand
/ shrink their caller's stack.

There are as many `Context` instances at any given point in time, as there are
active coros (i.e. generators which have been started by calling them). There is
one additional context: the VM interpreter. This context is used to start the
`__main__` module running, i.e. its bytecode which is a normal function.

A context is the equivalent of a "generator object" in standard Python.

Frame objects and contexts are at the heart of Monty's VM execution model. The
code is in `frame.cpp`.

## struct Interp

There is one special context: the Virtual Machine interpreter. Class `Interp` is
a subclass of `Context`, and therefore it's a "context-plus". There is a single
`Interp` instance, it's created by the main application.

This datatype is not defined in `monty.h` but in `interp.h`, and is not meant to
be included anywhere else than in the `main.cpp` application. The `Interp`
object contains the actual implementation of the bytecode interpreter. It
defines `run()`, `outer()`, and `inner()` methods, matching three of the four VM
loops, as described in the [Design overview](design.md#loops-all-the-way-down).
The "main" loop is architecture-specific and part of the main application.
