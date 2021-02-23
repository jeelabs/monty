# Troubleshooting

?> There's not much here at the moment - to be expanded as more troublemakers
show up ...

## New C++ code

#### Problems DURING garbage collection

There are some important rules for Monty objects, i.e. anything derived from
`Object`:

* they can be allocated on the heap, using `new ...`
* they can be used as local objects, i.e. on the stack
* they can be defined as static / global objects, inited at startup time
* they ***cannot*** be used as member variables in other objects

The reason for this last restriction, is that the garbage collector needs to
mark every reachable object, and to avoid infinite recursion it will also set a
"mark bit", just before traversing the object. This is not possible for objects
which are outside the GC's memory pool (i.e. the heap), because this mark bit is
located in a small header just _before_ the object itself. So `mark()` will
always check an object's _address_ to decide whether this header is present
(local and static objects will still be traversed, but perhaps more than once).

This heuristic fails for objects _inside_ other objects, even when in the heap.

The workaround is to use a reference variable instead, and set it to `*new ...`
in the object's constructor.

#### Problems AFTER garbage collection

A more common GC problem is to forget to mark all objects. This can cause
_reachable_ objects to be missed and then deleted by `sweep()`.  Such objects
are made unusable by _intentionally_ setting their VTable to nil.
