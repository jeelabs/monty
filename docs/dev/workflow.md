# Development workflow

There are two kinds of C++ development with Monty:

1. **Core development** - this is about adding new features to the core and
   chasing / fixing bugs. It involves editing files in a checked-out (and
   possibly modified and branched) copy of the Monty git repository, compiling,
   running tests, and trying out new scripts - both natively and on an attached
   µC board.

2. **Extension and application development** - in this case it's about working
   on C++ code which needs to be integrated with Monty. This could be a new
   module, a new datatype, or even just a single function to add as built-in,
   for use from PyVM. It's called _application development_ when `main.cpp` is
   (also) being customised, perhaps to alter the startup logic or because the
   platform requires a different way of embedding.

## Core development

All core development happens _inside_ the Monty source tree. Builds, uploads,
and tests are all launched from the root directory, via the `inv` command (i.e.
[PyInvoke](https://www.pyinvoke.org)). For this reason, it's also called
"in-tree" development. The purpose is to end up with a "git push" or a "pull
request" to bring the new changes into the public git repository.

This workflow relies heavily on the following tools, scripts, and configuration
files:

* **`inv <cmd>`** performs key development tasks - the list of all available
  commands can be obtained with `inv -l` - this is an installed package
* **`tasks.py`** is the central configuration file for `inv` - as Python script,
  it defines what tasks are available and how to "run" them (similar to a
  `makefile`)
* **`pio <cmd>`** is the [PlatformIO](https://platformio.org) build tool which
  takes care of all the toolchain details, e.g. compilers and uploaders - this
  too is an installed package
* **`platformio.ini`** is the central configuration file for `pio` - it has many
  sections, and is also parsed by the `tasks.py` script to extract some settings
* **`src/codegen.py`** is a custom-built _code generator_ for Monty, which it
  scans source code to insert / update / replace boilerplate code and a few
  other things - this script is launched with proper args from `tasks.py`
* **`src/runner.py`** is a _test runner_ for attached µC boards, which will send
  tests, compare the output, and report the differences - this script is also
  launched from `tasks.py`

#### Build configurations

The default for Monty is to build and test _natively_, i.e. on MacOS or Linux,
because this is (by far) the fastest way to try out any code changes. In
addition, there is a preset `pio` configuration for the `Nucleo-L432KC` board.
The details can be found in the `platformio.ini` file:

```text
[platformio]
default_envs = nucleo-l432
extra_configs = pioconf.ini

[env]
lib_compat_mode = strict
build_unflags = -std=c++11
build_flags = -std=c++17 -Wall -Wextra

[stm32]
targets = upload
platform = ststm32
framework = cmsis
build_flags = ${env.build_flags} -Wno-format -DSTM32
lib_deps = JeeH
monitor_speed = 115200
test_transport = custom

[env:nucleo-l432]
extends = stm32
board = nucleo_l432kc
build_flags = ${stm32.build_flags} -DSTM32L4
```

There are two ways to customise this:

1. add a `pioconf.ini` file with new (and possibly overriding) settings
2. switch to Extension development, i.e. "out of tree" mode, as described
   [below](#extension-development)

The first approach supports faster and more advanced development (and
is needed to debug "deep" problems), but the second one keeps the Monty
source tree clean and allows quick "git pulls" when there are updates.

The `pio` [documentation
site](https://docs.platformio.org/en/latest/projectconf/index.html)
documents the (many) useful options available in `platformio.ini`.

#### A few example workflows

As mentioned before, it's all based on the `inv` command, which knows how to
perform each task, and will automatically run the code generator when needed.
The see what `inv` _would_ do without actually performing the task, add the `-R`
flag (i.e. `inv -R ...`) to trigger a "dry run". For details about a specific
task, use `-h`, e.g. `inv -h python`.

* workflow #1: compile & run native `hello.py` script with near-instant feedback
  => **`inv`**
* workflow #2: compile and run all native C++ and Python tests: **`inv test
  python`**

To also test a basic µC setup, the default assumes that a "Nucleo-L432KC" is
plugged into USB:

* workflow #3: compile, upload, run C++ & Python tests on µC: **`inv upload
  flash runner`**
* workflow #4: clean, recompile, test everything => **`inv all`** (also
  compiles the examples)

Last but not least, there is a `watch` command, which will track a specified
Python script and re-compile / re-upload it whenever it changes. This is similar
to running Python code interactively:

* workflow #5: watch / compile / run natively on each change: **`inv watch
  myscript.py`**
* workflow #6: watch and re-send bytecode on each change: **`inv watch -r
  myscript.py`**

This last variant does _not_ upload firmware, it only re-sends new
bytecode (re-flashing can be done in a separate terminal window using `inv
flash`). In case the µC has crashed and hangs, a manual reset will be needed.

#### Additional configuration tweaks

There are a few sections in the `platformio,ini` file which are ignored by
`pio`:

* **`[invoke]`** lists some extra options to define which tests to skip during
  `inv all`
* **`[codegen]`** lists the _extra_ directories scanned by the code generator

The skipping is necessary to silence inevitable differences between native and
embedded tests (e.g. differences in garbage collector statistics on 64-bit and
32-bit).

The code generator _needs_ to go through all the source files which are included
in a build, because it collects information across the entire collection and
then generates a number of tables (e.g. the collected qstr data, known modules,
built-in functions).

!> The code generator scan needs to match exactly what `pio` will compile and
link together. See [below](#code-generation).

## Extension development

Extension development focuses on _adding_ to Monty instead of altering it. There
may well be changes needed in Monty, but that part will need to be done as
described above, i.e. "in-tree".

Extension and application development using Monty happens "out of tree", i.e. a
separate area with a small amount of _boilerplate tooling_ to support the
development process - hopefully it will be as convenient as in-tree, but with
the variety of tasks and changes it's virtually impossible to foresee.

The development workflow can be quite similar, i.e. using `inv` as the main tool
for driving all the compiles and uploads and tests. But there will be
differences, if only because dev tasks will differ.

The `examples/` folder in Monty is intended to illustrate out-of-tree
development (it's located inside the Monty repository just because that's the
easiest way to keep everything together and in sync).

#### Your first application

To start extension / application development, proceed as follows:

1. Create a new folder with a **symlink** to `examples/tasks.py` in it, e.g.

    ```text
    mkdir -p /path/to/my-monty
    cd /path/to/my-monty
    ln -s /patch/to/source/of/monty/examples/tasks.py
    ```

    This becomes the "out of tree" root area for custom builds of Monty, each
    in their own "project" directory.

2. Set up a first project where you can make custom Monty builds:

    ```text
    inv my-project
    cd my-project
    ```

3.  Now you can compile, run, and test this first not-so-custom build of Monty:

    ```text
    $ inv native
    main
    hello monty v1.0-112-ged9932d
    done
    $ inv python
    1 tests, 1 matches, 0 failures, 0 skipped, 0 ignored
    $
    ```

The `inv init ...` command created a few files. One important detail, is that
the `monty-pio.ini` file contains two paths which point _inside_ the Monty
source tree (and will have to be adjusted if this folder is ever moved):

```text
$ tree
.
├── monty-inv.py
├── monty-pio.ini
└── test
    └── py
        ├── hello.exp
        └── hello.py
```

That's it. The stage has been set to create custom versions. This could be a
modified `main.cpp` app, a port to a different platform, a new module /
datatype for use from Python, or merely a single extra function which you want
to implement in C++ and call from PyVM.

?> To summarise: custom Monty builds must be a subdirectory of such an "out of
tree" development area. The `inv` command will automatically check the parent
directory chain and find its `tasks.py` file there, which is also where all the
magic happens. The symlink will then be used to locate the Monty source tree.

## Code generation

A substantial part of the Monty source code is automatically generated. Some of
it is simply out of laziness, using code-generation as shortand. This line, for
example:

```
//CG args a1 a2 a3:o a4:i ? a5 a6:s a7:s a8 *
```

Will be expanded as follows:

```
//CG< args a1 a2 a3:o a4:i ? a5 a6:s a7:s a8 *
Value a1, a2, a5, a8;
Object *a3;
int a4;
char const *a6, *a7;
auto ainfo = args.parse("vvoi?vssv*",&a1,&a2,&a3,&a4,&a5,&a6,&a7,&a8);
if (ainfo.isObj()) return ainfo;
//CG>
```

There are several very practical reasons and benefits with this approach:

* why write repetitive code when the silicon can do it?
* it adds consistency (arg checking and type conversion, in this case)
* if there is a need to change it, just tweak and re-run the code generator
* and in that same vein: fix a bug once, re-run, and it's gone everywhere
* the code generator can also _strip_ it all out again, which makes for easy
  reading
* only the stripped version gets checked into git, why store redundant info?
* expansion and stripping are both _idem-potent_ - they're no-ops when
  re-applied
* there are no special headers or pre-processor define's, it all happens in
  plain sight
* and - by far - the best reason of all: _writing code is a lot more fun this
  way!_

Note that this style of code generation is _inline_, i.e. the generated code
becomes part of the source code. That's also why that final `//CG>` tag is
essential. On subsequent runs, the generated sections will be replaced by new
versions (and the code generator is smart enough to only rewrite source files
when there are actual changes).

But that's just a small part of the whole story. This code generator is also
used to:

* generate Tediously Boring Boilerplate Function Definitions ™
* collect all functions which need to be bound or wrapped for PyVM access
* ... and then generate the attribute table definitions for these bindings
* parse some headers in MicroPython to generate definitions for use in Monty
* enable massive insertion of debug print statements (i.e. for each VM opcode)
* insert of remove code based on the inclusion of certain features in the build
* and last but certainly not least: auto-renumber all qstrs in the source code

The effectiveness of this code generator comes from making it completely
_specialised_ for Monty's use - it serves no purpose elsewhere (other than to
demonstrate the technique, perhaps). The current `src/codegen.py` script is
about 700 LOC, and is regularly tweaked and extended to fit the evolving needs
of the project. And it already "pays for itself": well over 1,000 lines of
Monty's core source code are auto-generated.

#### Syntax

The code generator chases through all the C++ code (`.h` and `.cpp` files), and
looks for two kinds of markers via line-by-line string matching:

* directives of the form `//CG...` with only whitespace in front
* qstr references of the form `Q(<num>,"<str>")` (can be more than one per
  line)

The `CG` directives come in a few different shapes and sizes:

* `//CG ident ...` is a newly added directive, and consists of just that one
  line
* `//CG: ident ...` is a directive with no further expansion, it just informs
  the code generator
* `//CG< ident ...` + some lines + `//CG>` is a bracket range, as in the
  example above

If the expansion contains only 1 to 3 lines, then the leading line will start
with `//CG1` to `//CG3` respectively, then the expansion, but no closing
marker. This helps reduce visual clutter for very short expansions.

The `ident` field is a fixed keyword (e.g. `args` is processed in the code
generator by a def called `ARGS` (upper case). The code generator will throw a
(somewhat nasty) exception when processing mis-typed identifiers. The rest of
the line is parsed as words and passed as arguments to the codegen function.

The `Q(...)` matcher is a bit more involved. It needs to recognise multiple
uses on the same line, and it does some work behind the scenes:

* the first source file is `qstr.h`, which contains a directive to _collect_
  all the qstrs in it - these are the fixed qstrs in the system (same as in
  MicroPython, since they are also built into `mpy-cross`)
* subsequent files are parsed normally, with the numeric first arg replaced by
  whatever the codegenerator decides the proper ID should be (it will assign
  new ones as it comes across new strings)

The interaction with C++ and `constexpr` in particular is what makes this work:
the string argument is mostly for the _code generator_, but in C++, `Q(n,s)` is
simply an on-the-fly constructed instance of class `Q` - and in most contexts,
it evaluates to its first arg, i.e. an integer. This goes _very_ far, as the
following example illustrates:

```
//CG kwargs foo bar baz
```

This expands to the following code (which works in C++11 and upwards):

```
//CG< kwargs foo bar baz
Value foo, bar, baz;
for (int i = 0; i < args.kwNum(); ++i) {
    auto k = args.kwKey(i), v = args.kwVal(i);
    switch (k.asQid()) {
        case Q(186,"foo"): foo = v; break;
        case Q(187,"bar"): bar = v; break;
        case Q(188,"baz"): baz = v; break;
        default: return {E::TypeError, "unknown option", k};
    }
}
//CG>
```

It _looks_ a bit like a switch on strings (which is not possible in C/C++), but
under the hood it ends up being a switch on the qstr IDs. All of this will
vanish in the stripped version of the code, but even expanded it should be
quite clear that this is parsing keyword arguments ... in a C++ function, and
doing so in a nicely readable way.

Note that the assigned qstr IDs will change each time a new qstr is added
_anywhere_ in the source files. This has no further implications, since the
same code generator run which issues these numbers also generates the matching
constant table in the last file processed by the code generator: `qstr.cpp`.

?> No pre-processor defines were used or harmed in this design. It's all
`codegen.py` and C++17 ...
