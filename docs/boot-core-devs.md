# Boot + Core + Devs
> A way to structure embedded applications, tailored to Monty.

## App development
Embedded applications need to deal with a variety of functions, such as managing the startup process, serial and network communication, idle and low-power modes. And of course the main application logic itself, i.e. that which it is meant to do in the first place.

In the case of the Monty VM, there are also two different layers involved: a low-level architecture (written in C++) and high-level application logic (written in Python).

C++ is efficient but static: it has to be compiled on a "host", such as a laptop, running a compiler toolchain. Changes in C++ code may need a complete re-flash, since code is "linked" together in a way which fixes all instructions and addresses used into a firmware image.

Python is slower but dynamic: different modules can be combined at will, with all interactions between them determined at run-time. Replacing a module, or adding a new one, at run time is not only easy, it's the only way to go. Python scripts are pieced together progressively, and easily adjusted as time and insights and requirements progress.

There is tension between these two technologies. You need efficient C++ code for performance and low-level hardware tasks, but the associated compiles and uploads make it a tedious process to go through all the time. Python scripts also need to be compiled (in Monty's case), but it's much easier to make small changes and replace a small Python module, because such changes do not affect the rest, and do not need a full recompile and upload.

With larger applications, such as on ESP32 with an RTOS and network + security stacks, small C++ changes become time consuming, while Python changes remain snappy. In such larger projects, each change to C++ code leads to frustrating slowdowns in development.

## The basic idea
Wouldn't it be nice if we could have the best of both worlds: snappy changes and additions, in Python as well as in C++? This is what this new design aims to address.

The way to get faster C++ development cycles, is by splitting that code into different parts: a "core" part which has the bulk of the code that changes infrequently, and a "devs" part which is small, quick to compile, and can replace the previous version without a full re-flash. Working on a new driver for a specific hardware peripheral, or a function which needs high performance (both good candidates for coding in C++) should be possible without full compile-all + link-all + upload-all steps.

The idea is now to split the C++ code into three independently-compiled, -linked, and -uploaded parts: a small boot loader, a large core segment, and a small devs segment.

The **boot** loader knows how to replace the other segments, prefereably in a way which does not even need a restart. The boot loader is uploaded once, and only rarely replaced (i.e. a bug fix or essential new feature). It knows how to communicate to the outside world (using a serial or USB port, a network connection, whatever). This is how the system can be told to replace the other segments.

The **core** segment contains as much "runtime" as possible, which is not currently in active development. In the case of Monty, it will contain the VM, data structures, and all the other "generic" parts needed to run Python code. The core can also include some stable and common hardware drivers, such as serial, SPI, I2C, RTC, DMA, as well as "standard" network protocols.

The **devs** segment is optional. It exists to support quick C++ development, i.e. custom code which benefits from being written in C++, but may be application specific, or may simply not yet have been implemented for future addition to the core.

## The mechanism
A key requirement is that C++ code needs to be efficient, in CPU cycles and in memory use. That's the whole point, else we could write everything in Python. This efficiency must not be sacrificed in the split-application design being described here.

The trick is to keep linking code references as before, by carefully controlling what can be linked to what:

* The boot segment must be self-contained, i.e. it does not reference anything outside itself. It is in fact built like any other embedded application. The only difference, is that at the end of the build, a list is generated of all the global symbols present in the boot segment.

* The core segment is then linked to place code in a higher location in flash (above the end of the boot loader), and to assign variables to unused RAM, i.e. also above the RAM used by the boot loader. By including all the boot segment definitions extracted earlier, the core segment then has access to all global functions and variables of the boot segment. The core segment can only be used with the exact boot loader version used to link it. A boot loader change implies a core segment re-link. Here again, the core segment cannot refer to anothing outside itself, other than the boot loader globals, that is.

* Lastly, the devs segment repeats the same approach: all globals defined in the core segment are extracted and handed to the linker when building the devs segment. The devs code can tie into everything defined in the boot plus core segments. A change to boot or core requires that the devs segment must be re-linked to update all references into those two segments.

## No forward references
The one remaining issue is how a boot loader can activate a core segment, and how a core can in its turn activate code in the devs segment. After all, there is no way to refer to addresses which are not known to them, and might change in future builds.

This is solved through _run-time_ linkage, and can easily be implemented in C++ using statically-defined objects:

> C++ has a mechanism whereby the constructors of static and global objects are called as part of the startup sequence, before `main()` runs. Likewise, there is a standard mechanism in C++ to call all corresponding destructors when `main()` exits (or `exit()` is called).

This is re-purposed to good advantage here: the core and devs segments do not have a `main()` (to avoid confusion), but a call to `init()` which serves a similar role: to initialise all static data (as well as the `.data` and `.bss` segments from the C world). The address of this `init()` routine is stored in a fixed position at the start of the segment. When boot loads core, or when core loads devs, a call is made to perform these initializations. Later on, before replacing a segment, a similar mechanism can be used to "de-initialise" the objects.

Lastly, to make this all work, we need a run-time registration mechanism: a C++ class which chains all its objects so that the _parent segment_ can iterate over all the objects created in a _child segment_. There is nothing particularly complicated about this: objects registering themselves at run time are a common technique in C and C++.

## Dynamic load & unload

Here's what happens on startup when the boot, core, and devs segments are present in flash:

* The boot loader knows the starting address of the core segment in flash. From there, it knows how to find and call the core's `init()` code. Once that is done, the objects in core will have been initialised and registered themselves in a linked list available to the boot loader.

* The boot loader can then search its list for an object called "core" and call it. This object, located in the core segment then does the real work: running app code. It can work with all the functions and variables in the boot and core segment, using normal C++ linkage.

* Similarly, if the core segments wants to manage a secondary "devs" segment, it repeats the same process: locate and init the devs segment, and pass control to it, if so desired. The boot loader does not need to know about this - it just loads the core and launches it.

When a segment needs to be replaced, the reverse of this whole process needs to take place:

* The child segment (e.g. devs) must signal its parent (core) that it is done.

* The parent then calls the child's de-init code to call all relevant destructors of the child (in reverse order). Care must be taken that the child no longer uses any functionality of its parent. In particular, it must reset all its hardware devices so that no further activity (such as interrupts) can take place. This is essential, as all the child's functions and variables are about to become unusable.

* At this point, the child segment is no longer in use. The parent can then re-flash the memory with a different version of the child, or a completely different type of segment (as long as it has been built with up-to-date linkage information).

It is up to the boot loader (or for devs: the core) to de-init a child segment, replace it, and re-init it. There is in principle no need to go through a system reset, these segment replacements could be done while the remaining parts of the application are still running.

## Conclusion
At the end of all this, such an approach should have considerable potential:

* the load process is very similar to a normal power-up, just a few startup code changes
* the first segment (boot) is a 100% standard build, no adjustments needed at all
* none of this is tied to Monty or to any specific dynamic registration mechanism
* there is no performance penalty for using a split-design, other than the run-time registration of child segment functionality in one of its parents
* the only extra overhead is a small header for each segment, and the fact that segments must start on flash-erase boundaries
* bonus: for testing, a special devs segment could be loaded, which exposes more of the core to Python for exercising and inspecting its "deeper" details

There are also a few inconveniences:

* segments must be built for _specific_ flash and RAM locations, some planning will be needed to assign good ranges to each segment
* a bunch of additional load maps (or at least .elf files) need to be managed and saved, for building new child segments or updating existing ones
* this is not a general 100% cross-linkable module system, in that segments can not forward-reference addresses in later segments
* when a segment is changed, all segments depending on it (i.e. loaded after it) will need to be re-built and re-loaded as well

The three-segment split seems like a reasonable trade-off, to support development _of_ the core as well as development _with_ the core, for app-specific extensions.