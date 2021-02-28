# extend/\*

The `lib/extend/` area is a first example of how optional functionality can be
managed in Monty. This area is only included if it's mentioned in the
`[codegen]` section in `platformio.ini` (or its extension:
`platformio-local.ini`).

There are currently two different extension categories in here:

* `mod-sys.cpp` defines a `sys` module for using in Python as `import sys`
* `typ-array.cpp` implements an "Array" datatype, which then automagically
  becomes available as `array(...)` in Python, and as `struct Array` in C++

## module sys

This module can hardly be called "optional", as it provides functionality which
is essential for certain tasks in Python.

The `sys` module has the following attributes:

* `sys.builtins` - a dict for extending built-in types and functions
* `sys.event()` - create a new event object, for coro/task synchronisation
* `sys.gc()` - trigger an object garbage collection and vector compaction
* `sys.gcmax()` - report size of the largest memory area which can be allocated
* `sys.gcstats()` - print out some GC statistics
* `sys.implementation` - name of this implementation, i.e. `"monty"`
* `sys.modules` - a map of all currently-loaded bytecode modules
* `sys.ready` -  this is the list of ready-to-run coroutines
* `sys.version` - version of this build, i.e. `"v1.0"` (based on `git describe`)

The `//CG ...` code generator directives in `mod-sys.cpp` have as side effect
that this module is automatically included on the list of built-in modules.

## type Array

The other source code in this area defines a new datatype, and is implemented
in the files `typ-array.h` and `typ-array.cpp`. As with `sys`, the `//CG`
directives in the source code take care of all necessary setup to bind this
optional datatype into the rest of Monty.

The array type goes beyond what is available in CPython and MicroPython. The
following types are available:

* `array("P")` - array of 1-bit "packed" values (0..1)
* `array("T")` - array of 2-bit "tiny" values (0..3)
* `array("N")` - array of 4-bit "nibble" values (0..15)
* `array("b")` - array of 8-bit signed bytes (-128..127)
* `array("B")` - array of 8-bit unsigned bytes (0..255)
* `array("h")` - array of 16-bit signed "half" ints (-32,768..32,767)
* `array("H")` - array of 16-bit unsigned "half" unisgned ints (0..65,335)
* `array("i")` - same as "h"
* `array("I")` - same as "H"
* `array("l")` - array of 32-bit signed "long" ints
* `array("L")` - array of 32-bit unsigned "long" unisgned ints
* `array("q")` - array of 64-bit signed "quad" ints
* `array("V")` - array of variable-sized byte entries

The 1-/2-/4-bit versions are tightly packed, making them very memory-efficient
for small values. The current implementation does not know how to shift partial
bytes however, so slice insertion/removal can only be done in steps of 8, 4, or
2, respectively.

There is no "Q" type for unsigned 64-bit integer values, because Monty cannot
distinguish 64-bit signed from 64-bit unsigned.

All arrays can be printed in a lossless textual representation, which has the
following structure:

```text
<number-of-entries> <type-as-char> <raw-data-as-hex>
```

For examples, see the `pytests/array.py` code and `pytests/array.exp` output.

## VaryVec

The "V" type is unusual in that each entry is variable-sized. Each of the
entries is a `bytes` object (i.e. `struct Bytes` in C++).

This array type is called a "VaryVec" in Monty. Internally, it's all based on a
byte array, with an "index" (i.e. table of offsets) at the start, followed by
all the array entries. This too is a very compact representation, as it's all
fitted into a single `ByteVec` in memory, but there are several trade-offs:

* the total size of a VaryVec, i.e. index + all items, is limited to ≈ 65,000
  bytes
* read access is quick, and item sizes can be queried without retrieving the
  item itself
* write access is only quick when the size remains the same, else considerable
  amounts of data may have to be moved up or down in the vector
* adding or removing an item, even an empty one, always has to move all data

VaryVecs are used for the qstr table, both the built-in read-only one in flash
and the one extended with new entries at run time. They may find other uses in
Monty later, and might be useful for other "read-mostly" variable-sized indexed
data in Python, such as font tables and symbol tables.
