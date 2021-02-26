# Data structures

All major data structures in Monty are built with classes derived from `Object`.
This is what allows them to also be used from Python. Several of these classes form a
hierarchy, since they build upon - and extend - the features implemented in
other classes. The main classes in this hierarchy are:

```
Object
├─ None
├─ Bool
├─ Int
├─ Event
│  └─ Stacklet
├─ Bytes
│  ├─ Str
│  └─ Array
├─ Tuple
│  └─ Exception
└─ List
   ├─ Set
   │  └─ Dict
   │     ├─ Module
   │     ├─ Type
   │     │  └─ Class
   │     └─ Inst
   └─ Closure
```

A few of these deserve special mention:

* **Str** is like `Bytes`, but represents (and indexes) variable-length UTF-8
  characters, not 8-bit bytes
* **Array** is also derived from `Bytes`, because it implements data structures
  with elements that are all represented internally as a sequence of raw bytes
* **List** combines the `Object` and `Vector` (aka `VecOf<Value>`) classes
* **Dict** derives from `Set` because it is implemented as a set of keys, with
  the values saved separately in the same vector
* **Stacklet** is an `Event`, but it's also a Vector which is used extensively
  as a "stack of call frames" in the VM (and very similar to a Python "generator
  object")

## Bytes

This datatype is defined as `struct Bytes : Object, ByteVec {...};`,
using C++'s _multiple inheritance_.

From C++, such objects can be treated mostly as a ByteVec. From Python, it's an
object (and a sequence in this case), supporting indexing, `len()`, and such.

?> Monty uses multiple inheritance sparingly.  The virtual methods in `Object`
allow all objects to be used from Python. The other class never has
virtual members, VTable indirections, or _override_ definitions.

Bytes are used for immutable data, but this is not enforced in C++. They simply
represent "a bunch of `uint8_t` values", with other interpretations mapped on
top when created as `Str` or `Array` objects.

## List

The "list" is a central data structure in Monty, for both C++ and Python. It is
used for all collections of objects, and uses the resizable `Vector` type to
store pointers to all its element objects.

From C++, a list object can be treated as a `Vector` most of the time, with
convenient indexed access using natural `list[i]` notation for both getting
_and_ setting elements. Because vectors also define STL's `begin()` and
`end()` methods, it is possible to write concise loops using C++11 syntax:

```
for (auto& e : myList)
    if (e == 123)  // e is a Value, which supports automatic cast-to-int
        e = 456;
```

(note the `auto&`, which is essential if `e` is to be used on the left-hand
side)

A more traditional approach also works fine:

```
for (size_t i = 0; i < myList.size(); ++i)
    if (myList[i] == 123)  // see above comment
        myList[i] = 456;
```

Like all vectors, a list has a size and a capacity. The "size" is the count of
the current number of elements in the list. The capacity is the current amount
of space reserved for the vector. At the lowest level (in `Vec), counts are
stored as byte counts. The capacity of a list increases as more space is needed
to store the elements of a list, but does not always decrease immediately when
elements are removed. This _slack_ can be used to reduce (re-)allocation
overhead.

## Dict

The "dict" is another important data structure, used as base class for a few
other types. It builds on `Set` (and `List`) in that it too contains a number of
elements, but in this case the elements are keys and associated values.

?> Key lookup in Monty is currently very inefficient for non-trivial sets and
dicts, as it uses _linear search_. This is a case where hashing could greatly
improve performance.

Like lists, dicts have a size, but this size is no longer related to the length
of the underlying vector. Instead, "size" now means "number of keys" (or values,
same thing).

In C++, a reasonably _Pythonic_ notation is supported, but to avoid type-casting
surprises it does not re-use the `[]` operator notation. Instead, there is an
`at()` method, which takes a `Value` as argument:

```
myDict.at(123) = "abc";
myDict.at("def") = 456;
if (myDict.at("def") == 456) {...}
if (strcmp(myDict.at(123), "abc") == 0) {...}
```

(there are limits to how far C++ can take this, but the above implicit casts
should work)

Modules are dicts, because that's where the module's _variables_ are stored.

Types are dicts, used to store the type's _attributes_. Since these dicts are
modifiable, the attributes of a type can be extended at run time. This is why
Monty supports "polyfills", just like JavaScript: the ability to extend the
attributes of any built-in type.

There is some vagueness here, since strictly speaking a type **_has-a_** dict,
not **_is-a_** dict. But so far in Monty, this difference does not appear to be
very important.
