# Development workflow

There are two kinds of C++ development with Monty:

1. Core development - this is about adding new features to the core and chasing
   / fixing bugs. This involves editing files in a checked-out (and possibly
   modified) copy of the git repository, compiling, and testing the code - both
   natively and on embedded µC boards.

2. Extension and application development - in this case, you're working on C++
   code which needs to be integrated into Monty. This could be a new module, a
   new datatype, or even just a single function to add as built-in, for use from
   PyVM. Application development is when `main.cpp` is also being customised.

## Core development

All core development happens _inside_ the Monty source tree. Builds, uploads,
and tests are all launched from the root directory, via the `inv` command (i.e.
[PyInvoke](https://www.pyinvoke.org)). For this reason, it's also called
"in-tree" development. The purpose is to end up with a "git push" or a "pull
request" to bring the new changes into the public git repository.

This workflow relies heavily on the following tools, scripts, and configuration
files:

* **`inv <cmd>`** performs key developmen tasks - the list of all available
  commands can be obtained with `inv -l` - this is an installed package
* **`tasks.py`** is the central configuration file for `inv` - as Python script,
  it defines what tasks are available and how to "run" them (similar to
  `makefile`)
* **`pio <cmd>`** is the [PlatformIO](https://platformio.org) build tool which
  takes care of all the toolchain details, e.g. compilers and uploaders - this
  too is an installed packaga bit like ae
* **`platformio.ini`** is the central configuration file for `pio` - it has many
  sections, and is also parsed by the `tasks.py` script to extract some settings
* **`src/codegen.py`** is a custom-built _code generator_ for Monty, which it
  scans source code to insert / update / replace boilerplate code and a few
  other things - this script is launched with the proper args from `tasks.py`
* **`src/runner.py`** is a _test runner_ for attached µC boards, which will send
  tests, compare the output, and report the differences - this script is also
  launched from `tasks.py`
* **`src/mrfs.py`** is a tool to compile Python scripts to bytecode (using
  `mpy-cross`), combine them into a "MRFS image" and upload it to the attached
  µC - (mostly) for use from `tasks.py`

Just to mention this for completeness: both `tasks.py` and `mrfs`.py will import
`runner.py` to reuse some of its code (in particular the code to connect to an
embedded board).

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
2. switch to Extension developmen, i.e. "out of tree" mode, as described below

The first approach allows somewhat faster and more advanced development (and may
be needed to debug "deep" problems), but the second one is preferred as it keeps
the Monty source tree clean, and allows quick "git pulls" when there are
updates.

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
* workflow #4: clean and recompile + test everything => **`inv all`** compile
  the examples

Last but not least, there is a `watch` command, which will track a specified
Python script and re-compile / re-upload it whenever it changes. This is similar
to running Python code interactively:

* workflow #5: watch / compile / run natively on each change: **`inv watch
  myscript.py`**
* workflow #6: watch and re-send bytecode on each change: **`inv watch -r
  myscript.py`**

This last variant does _not_ reflash the firmware, it only re-sends new bytecode
(re-flashing can be done separately using `inv flash`). In case the µC has
crashed and hangs, a manual reset will be needed.

#### Additional configuration tweaks

There are a few sections in the `platformio,ini` file which are ignored by
`pio`:

* **`[invoke]`** lists some extra options to define which tests to skip during
  `inv all`
* **`[codegen]`** lists the _extra_ directories scanned by the code generator

The skipping is necessary to silence inevitable differences between native and
embedded tests (e.g. differences in garbage collector statistics on 64-bt and
32-bit).

The code generator _needs_ to go through all the source files which are included
in a build, because it collects information across the entire collection and
then generates a number of tables (e.g. the collected qstr data, known modules,
built-in functions).

!> The code generator scan needs to match exactly what `pio` will compile and
link together. There is much more to say, but this will be documented on another
page.

## Extensions development

Extension development focuses on _adding_ to Monty instead of altering it. There
may well be changes needed in Monty, but that part will need to be done as
described above, i.e. "in-tree".

Extension and application development using Monty happens "out of tree", i.e. a
separate area with a small amount of _boilerplate tooling_ to support the
development process - hopefully it will be as convenient as in-tree, but the
variety of tasks and changes is essentially impossible to foresee.

The development workflow can be quite similar, i.e. using `inv` as the main tool
for driving all the compiles and uploads and tests. But there will be
differences, if only because dev tasks will differ.

The `examples/` folder in Monty is intended to illustrate out-of-tree
development (it's located inside the Monty repository just because that's the
easiest way to keep everything together and in sync).

#### Your first application

To start extension / application development, proceed as follows:

1. Set up the `MONTY_ROOT` environment variable to point to the root on the
   Monty checkout clone you want to use - probably easest to do this in
   `~/.bashrc`, etc, e.g.

    ```text
    export MONTY_ROOT=/path/to/my/monty/
    ```

    Make sure it's now properly set (re-login if necessary), but also note that
    for convenience there is _one relaxation_ of this rule: if the out-of-tree
    area is located _next_ to the Monty source tree (i.e. a sibling), then it
    will be found even when `MONTY_ROOT` is not set.

2. Grab a copy of the examples folder as starting template for your own work:

    ```text
    cp -a $MONTY_ROOT/examples/ $HOME/path/to/my/monty/projects
    ```

3. Create a subfolder for your first monty project (it _has to be in a
   subfolder):

    ```text
    inv init myproject
    ```

4. Verify that the new area can build Monty out-of-tree (this will be default to
   a native build):

    ```text
    inv build
    ```

    If all is well, you should now have a working `.pio/build/native/program`
    executable.

That's it. The stage has been set to create a custom build of Monty. It could be
a modified `main.cpp` app, or a port to a different platform, or a new module /
datatype for use from Python, or merely a single extra function which you want
to implement in C++ and use from PyVM.

You can try out the examples to see what each of them do, or delete
them all to start from a clean slate (using `rm -rf
$HOME/path/to/my/monty/projects/*/` - just be sure to keep the files!).

> To summarise: all custom Monty builds must be a subdirectory of such an "out
> of tree" development area. The `inv` command will automatically check the
> parent directory and find its `tasks.py` file there, which is also where all
> the magic happens. In addition, the `MONTY_ROOT` environment variable needs to
> be set to tie this all into the checked out version of Monty.
