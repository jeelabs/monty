# Segmented Monty notes

@idea - trying to fit all the pieces from dymolib and monty together

## It's all in 3 segments

### boot
* does clock & console init (serial/usb/net/etc)
* knows how to locate / load / replace segments
* knows how to manage Minimal Replaceable File Storage
* handles the low-power modes, watchdog, rtc, and wake-ups
* is a 100% std platformio app build, self-contained

### core
* all of Monty, maybe parts can be a C++ module like everything else
* inits and manages GC'd memory, with vectors and objects
* the main parts are: VM / interp, datatypes, import
* can load/unload the devs segment, using boot's services
* startup logic: find python startup module in MRFS and import/run it
* fixed / built-in modules: data structures, serial/i2c/spi/timers/etc
* can access all global functions, classes, variables in the boot segment

### devs
* zero, one, or more C++ modules, activated through python import
* this segment is for development, once code is stable, it can move into core
* has well-defined stable API interface to the core and boot segments
* ideally also for development of Monty itself - to have a uniform dev approach
* a special devs variant can be created for in-depth testing of the core

### open issues
* add tiny cmdline interface to boot?
* native macos / linux builds
* testing, all combinations: native / embedded, C++ / Python

## Use-cases for devs reload

1. code is too big to transfer every time: put stable large code in core and small updated code in dev. The transfer time may be "too long" because the code is very large or because the transfer is slow, e.g. wireless.
2. some code needs to keep running to keep a connection alive to enable reloading of dev, that code would be in core.
3. restarting certain functions takes too long, for example, due to protocol start-up (crypto, polling time windows, etc.
4. restarting certain functions is too disruptive, for example, due to robot arms that need to be repositioned/re-zeroed.

## How to handle malloc

It is assumed that all monty-specific code uses monty objects, which are GC'd.
Thus malloc is really there for 3rd party code.
A first-cut proposal on how to handle malloc is:
- the client module that uses the 4rd party library writes or configures a malloc wrapper to hand to the 3rd party module
- the wrapper allocates objects and is responsible for making object GC work correctly, for example by keeping a vector of such "malloc objects" serving as root pointers
- the wrapper is responsible to make unloading work in a reasonable manner, for example, if the module is unloaded then all objects used only internally should become GC eligible
- if pointers to malloc'd stuff are handed out to other modules this should always be in the form of an object that contains a pointer to the malloc'd area, this way as long as the other module hangs on to the object the malloc'd area will not be collected

## Memory segments (old)

| Name        | Size (est) | Location | Movable | Allocated when |
|-|-|-|-|-|
| boot:text     | 4..16kB  | flash | no | when linking boot |
| boot:data+bss | 1kB      | ram   | no | when linking boot |
| core:text     | 32..64kB | flash | no | when linking core |
| core:data+bss | 1 kB     | ram   | no | when linking core |
| devs:text     | 4..16kB  | flash | no | when linking devs |
| devs:data+bss | 1kB      | ram   | no | when linking devs |
| C stack       | small?   | ram, grows down | no  | when starting boot |
| malloc heap   | some?    | ram, grows up   | no  | when starting core (or devs?) |
| vector space  | lots     | ram, grows up   | yes | when starting devs, compacted down by GC |
| object space  | some     | ram, grows down | no  | when starting devs, GC collected, never moved |

Notes:

- C stack can be moved very early on, but not once there are active call frames
- malloc and object space may be able to be merged, need a way to prevent "malloc objects" from being GC'ed
- vectors need to grow up so the top-most vector can be extended without copy
- vectors can move when Monty is not in its inner loop
- malloc can be implemented to use object space, but "malloc objects" are not Monty objects - they don't derive from the Object type and have no vtable for the GC to access, they don't participate in mark and sweep GC - they are just treated as permanently marked to prevent deletion by the GC
- only two minor inconveniences remain: fixed C stack size, and devs can only be replaced when not in inner loop

### Example layers + GC

```
<RESET>
  flash 08000000..080008C4, ram 20000000..2000038C, stack top 20010000
  main -> regFun 08001149
hello from core
  flash 08001000..08001970, ram 2000038C..200003F8, stack top 20010000
  gc pool 2000041C..2000FC00
[...]
  vec 2000FFD8 ok 1 ptr 20000420
[...]
  core -> regFun 080020C5
cheers from devs
  flash 08002000..080021F8, ram 200003F8..2000041C, stack top 20010000
  vec 20000414 ok 1 ptr 20000488
[...]
  core -> deregFun 0800210D
gc: avail 63448 b, 0 checks, 1 sweeps, 1 compacts
gc: total      0 objs        0 b,      2 vecs     1112 b
gc:  curr      0 objs        0 b,      0 vecs        0 b
gc:   max      0 objs        0 b,      2 vecs     1112 b
goodbye from core
  main -> deregFun 080015F1
<EXIT>
```
  
### Memory layout

**FLASH** - starting at 0x08000000:

| Name | Size (est) | Comments |
|-|-|-|
| boot | 4..16 kB  | boot segment, runs after reset |
| core | 32..64 KB | main VM & runtime support code |
| devs | as needed | driver & application development |
| mrfs | the rest  | bytecode modules and data files |

**RAM** - starting at 0x20000000:

| Name | Size (est) | Comments |
|-|-|-|
| boot:data+bss | 3 kB | console buffers, interrupt vector |
| core:data+bss | 1 kB | static VM & runtime objects |
| devs:data+bss | as needed | static driver & app dev objects |
| vector space  | lots | grows upwards, compacted down by GC |
| object space  | some | grows downwards, collected by GC but never moved |
| C stack       | 1 kB | grows downwards, towards top of object space |

The best way to deal with scarce RAM, is to always place as much data as possible in vectors, as these can be resized, collected, and compacted for optimal memory use. Buffers which may not move should be allocated as objects in the GC pool - preferably early on, so that they end up at the top of memory and don't cause excessive GC pool fragmentation.

## minimum viable core
_Monty is small, yet it's already too monolithic._ In the context of a segmented app, as much as possible should be a "module" of some kind (C++, that is). This should include the fancier datatypes such as Array and VaryVec, but also floating point, so that they can be added as _option_ and developed / tested in isolation.

_Even the bytecode interpreter should be a module._ There is nothing fundamentally Pythonic in Monty, it could in principle be used as foundation for other languages. The essence of Monty is really about dynamic data structures, mapped into C++.

The _Minimum Viable Core_ (MVC) will be the set of types and their implementation, which are rich enough to bring everything else in as separate modules. It will require a basic set of data types all the way up to dicts/maps, to be able to define the type system in terms of itself. It also needs Value and ArgVec to support a minimal API: `Value func(ArgVec const&)`.

The MVC is not meant only for run-time extensibility. Much of the code, including the functionality added using modules, will still use shared headers and be statically-linked to avoid performance loss. The key goal is merely to allow changes to these modules without having to adjust everything else, and to be able to test new versions in isolation.

### C++ type hierarchy
The following structure is from Monty v0.96:

```
g   Obj
 +      Object : Obj
  u         None : Object
  u         Bool : Object
  u         Int : Object
            Iterator : Object
            Range : Object
            Slice : Object
            Lookup : Object
            Tuple : Object
                Exception : Tuple
            Buffer : Object
            DictView : Object
            Super : Object
            Function : Object
            Method : Object
            Callable : Object
            BoundMeth : Object
            Cell : Object
            Resumable : Object
g   Vec
 +      VecOf : private Vec
  u     ByteVec = VecOf<uint8_t>
  u         VaryVec : private ByteVec
  u         Bytes : Object, ByteVec
  u             Str : Bytes
                Array : Bytes
 +      Vector = VecOf<Value>
 +          List : Object, Vector
 +              Set : List
 +                  Dict : Set
 +                      Type : Dict
                            Class : Type
                        Inst : Dict
                        Module : Dict
                Bytecode : List
                Closure : List
                Context : List
    MethodBase
        MethodDef : MethodBase
g   GCStats
 u  Q
 +  Value
 +  ArgVec
    Interp
    InputParser

```

* g = gc.h + gc.cpp
* \+ = needed in minimum viable core
* u = undecided, may also be required

## Low-power concerns

Monty and ultra low-power use may not be compatible due to the fact that the amount of code that needs to run to get from a reset to a running Python script is substantial. This is also an issue in MicroPython. The (C++) Micro Power Snitch needed a lot of work to get this to a minimum.

The two options are to either just stick to a C/C++ app, not the Monty VM, or to stick to the lowest sleep mode that preserves RAM instead of using a mode that goes through a full reset.
In stm32 lingo that would be using stop mode instead of standby mode, which on the L432 comes out to 1.3 µA vs 0.3 µA (both incl RTC wakeup, according to the datasheet).

## MRFS

Minimal Replaceable File Storage (MRFS) offers a way to store arbitrary _binary blobs_ of data in internal flash memory, but let's call them files - for convenience. These files are accessible by name, have a time-stamp (only 1-min precision), and can be replaced by new versions. Files are stored consecutively in flash, but the MRFS driver will make sure that one flash segment can always be released, erased, and then re-used. The file size needs to be known up-front, and only a single file at the time can be saved - this is not a general-purpose disk-like system.

MRFS works by storing each new file after the existing ones, where the _highest_ one with a matching name is considered valid. At some point, the amount of free flash memory will start to run low. MRFS will then start copying the first _still valid_ files upwards, until the lowest flash segment is no longer relevant. This segment is then erased, and as many _valid_ files as will fit, are copied down. Then the process repeats, in effect compacting all valid files down at the low end of flash memory. Now, the high end is free again, and new files can be saved there.

Such a compaction cycle may take some time (seconds). But this way of using flash does have the advantage that it works as a _wear-leveling_ scheme: files get written from low to high, then we compact, then new files grow upwards again.

MRFS is intended for storing files which rarely change. It's not meant for data capture or logging. This includes _static_ data and web pages, as well as Python bytecode modules. Since all data is located in internal flash, it can be accessed as normal read-only memory. Each file is a contiguous area of memory, but it may get moved during a compaction cycle.

Each file starts with a magic number and ends with a 32-bit CRC checksum, which can be used to verify its integrity. File headers start on a 32-byte boundary.

In the context of Monty, MRFS storage will start at the next flash segment after the devs layer. When layers are replaced by larger ones, an early MRFS compaction may be needed to make enough room for all the layers to still fit before the MRFS area.

The plan is to include the MRFS driver in the boot layer, so that it can manage core + devs, as well as all of MRFS. In fact, boot could obtain new versions of core and or devs _from MRFS_, and then perform the necessary flash juggling and erasure to switch over, before continuing with the application.

Taking this even further, the initial flash image only needs to contain the boot layer + MRFS, with core and devs included in MRFS under fixed names. On first startup, boot would notice the absence of core and devs in their expected spots, and start by moving MRFS up and "installing" core and devs as part of the first-run sequence. Alternately, special variants of core + devs could be present, which then decide to replace themselves after their initial run, i.e. for a customisable welcome/config session or an in-the-field acceptance test, before switching over to the production versions.

Infrequently changed data, such as configuration settings, could also be stored as files. The last successfully-written copy will then invalidate previous ones.

_Lots of options ..._
