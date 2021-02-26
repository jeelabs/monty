# Objects and vectors

!> The description below does not clearly describe the distinction between `Obj`
and `Object`, and between `Vec` and `Vector`, because these types are still
somewhat in flux. Suffice to say that `Obj` is the base class for `Object` (and
`Vec` for `Vector`), to keep GC-related logic separate from the higher-level
data types. Exact details will be added later, once the dust settles.

There are two core data structures in Monty: objects and vectors.

**Objects** are often small (in the 10's of bytes) and have a fixed address once
allocated. Objects are instances of the C++ class `Obj`, and can contain
other member fields. Objects are usually allocated in `object space`, which is
at the high end of the global memory pool, 

?> The main property of an object is that it is _garbage-collected_, and can
automatically be deleted by the GC once no more references to it exist.  In
short: **objects are tidy**.

**Vectors** are used for data which can change in size, such as strings and
lists. They are instances of the C++ class `Vec`, and contain a pointer into
_vector space_, located at the low end of the global memory pool. Note:
vector instances themselves are just a few bytes, including that pointer to
actual vector _contents_. Instances of `Vec` are _not_ Objects (i.e. not a
subclass of `Obj`), they may be allocated on the stack or as field of an `Obj`.

?> The main property of a vector is that its _contents_ can be resized as needed
and moved by the GC to reclaim contiguous memory space. In short:
**vectors are flexible**.

There is a third ingredient, which brings everything together and makes it all
really convenient in C++: a **Value** is an instance of the C++ class `Value`
which represent a _typed reference_. A value can hold either an integer, a
string, a _qstr_ ("quick string"), a pointer to an object, or nil (`nullptr` in
C++11).

!> In C++, "things" are called _instances_, which have _member fields_ and
_member functions_. In Python, "things" are called _objects_, and they have
_attributes_ and _methods_. It's hard to always get this right!

All objects, vectors, and values are instances in C++, but they tend to be
allocated and used differently, due to their very specific properties:

* a value is just a 32-bit word, with some fancy tagging tricks to be able to
  distinguish its current type (on 64-bit CPUs, a value is 64 bits) - passing a
  value around is as efficient as an int or pointer
* a value is easily constructed on-the-fly, due to its many C++ constructors -
  this is the main reason why so much of the code in Monty is compact and almost
  ... _Pythonic?_
* values have no C++ destructor, nothing special happens when they go out of
  scope
* `Obj` and `Vec` instances may **not** move around in memory, as this would
  break any pointers referring to them
* since, by design, the _contents_ of vectors will move, vectors
  cannot _contain_ object or other vector instances - only simple (scalar) values and
  _pointers_ (to objects or other vectors, for example)
* this is where values come in really handy: a _vector of values_ is the C++
  equivalent of a Python list

The interaction between objects, vectors, and values is at the heart of Monty's
design.  Their properties and limitations drive most of the implementation
choices in Monty.
