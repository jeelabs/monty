The central source file in Monty is `lib/monty/monty.h`. This contains all the
key datatypes of the system. These notes act as annotations for this header, but
not always in the same order as the source code.

This is C++ code, even though most types are defined as `struct`, not `class`.
The distinction is minimal: members are public by default in structs, but
private in classes.

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

Values are passed by value (seems obvious, but it must be stressed). A
`Value` will fit in a machine register and is as efficient as passing around an
`int` or a `void*`. There's a _small_ overhead to wrap / unwrap it.

Values are easy to create: pass an `int`, C string, or object pointer where a
Value is expected, and C++ will take care of it.  When used as `int` or
C-string, C++'s automatic casting again makes things easy.

Values can be checked with `v.isNil()`, `v.isInt()`, `v.isStr()`, and
`v.isObj()`, which return a `bool`.

With `v.ifType<ListObj>()` (a template function, hence the `<>`'s), you
get a pointer of the requested object type back (or a null pointer if there's a type
mismatch).
For strict use, there's `v.asType<ListObj>()` which returns a C++
_reference_, or else fails in an assertion if the type doesn't match.

The `v.dump()` method is a convenient way to display a value on the console for
debugging. If given a string arg, it prefixes the output with that string and
adds a newline at the end.

Values are _everywhere_ in Monty. They are frequently used as function arguments
and as return type.

## `struct Vector`

The `Vector` is Monty's way of managing variable-sized indexable data. They hold a
variable number of N-bit items (N being anything from 1 to at least 64). Vectors
have a `length()` (the number of items) and a `capacity` (the size in bytes to
which a vector can grow without reallocation).

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

### `struct Object`

...
