This is how the Monty repository is currently set up and used for development.

## Requirements

This project depends on three executables:

1. [PlatformIO][PIO] (the `pio` command) for native and embedded builds
2. [MicroPython](https://micropython.org/)'s `mpy-cross` to compile to `.mpy`
   files
3. [Python](https://www.python.org/) (version 3) is required by `pio` and
   `src/codegen.py`

These should be in the shell's `$PATH`. PlatformIO will automatically install all  
the toolchains it needs. For native builds, `gcc` needs to be present as well.

Once these requirements are met, run `make` to get started.

[PIO]: https://docs.platformio.org/en/latest/core/installation.html

## Directory structure

```
├─ configs              PlatformIO .ini files
├─ docs                 published at monty.jeelabs.org
├─ ext                  git submodules, external packages
│   ├─ lwip
│   └─ ...
├─ lib                  the main components of & for Monty
│   ├─ config           architecture-specific settings
│   ├─ monty            the core VM
│   ├─ network          network module based on LwIP
│   │   ├── include
│   │   └── src
│   └─ ...
├─ src                  main applications
│   ├─ native           macOS/Linux main & networking
│   ├─ stm32            STM32 main
│   └─ ...
├─ test                 C++ tests, based on Unity
└─ verify               Python tests and expected output
```

## PlatformIO

Monty uses PlatformIO (PIO) for builds and C++ tests, with a `platformio.ini`
file to specify configuration settings and library dependencies. To handle
different build targets, several `.ini` files are present in the `configs/`
directory. Their name is based on the name PIO uses for each [board
type](https://platformio.org/boards).

One way to use a specific configuration, is via the `-c` option:

    pio run -c configs/bluepill_f103c8.ini

But it's more convenient to set up a `platformio.ini` file in the top directory,
with a default include:

    [platformio]
    extra_configs = configs/bluepill_f103c8.ini

Serial ports can also be added to `platformio.ini`, using a corresponding
section for each setup:

    [env:bluepill_f103c8]
    upload_port = /dev/cu.usbmodemDDD8B7B81
    monitor_port = /dev/cu.usbmodemDDD8B7B83
    test_port = /dev/cu.usbmodemDDD8B7B83

This way, dev actions become a matter of typing `pio run`, `pio run -t upload`,
`pio test`, etc.

The code in `lib/` is _normally_ automatically found and included by PIO, but
this relies on mentioning their header files somewhere in the main code, which
is not always convenient. That's why some configurations explicity list the
libraries as a `lib_deps = ...` setting.

As shorthand, the makefile includes some convenience targets:

* `make run` compiles and runs a native build, for testing on macOS or Linux
* `make up` compiles and uploads the configuration defined in `platformio.ini`
* `make mon` is like `make up`, and then launches PIO's built-in serial console

## Architecture specifics

Architecture-specific code is placed in the `src/*/` areas as much as possible,
next to `main.cpp`. Mostly in files called `arch.h` and `arch.cpp`, but there
may be other files, such as `src/native/network.cpp` for the POSIX network code.

The `lib/config/` area is used to pick different headers, as needed for each
specific architecture. It's placed in `lib/` because the Monty core and other
libraries will need access to these configuration details. The general idea is
that preprocessor defines (i.e. `-D<name>`) are added to the PIO `.ini` files,
which then affect how `lib/config/config.h` includes architecture-specific
headers and source code.

In summary:

* Architecture specifics which can be handled at the main application level
  should be placed in the `arch.h` and `arch.cpp` files next to their `main.cpp`
  source, i.e. as limited-scope as possible.
* Feature choices, GPIO pin assignments, serial port to use as console, etc.
  should be set up via a `-D...` build flag the corresponding `configs/*.ini`
  file for PIO and with `#if` / `#ifdef` in the relevant `lib/config/config-*.h`
  header.

See `configs/genericSTM32F103RC.ini` and `lib/config/config-stm32.h` for an
example of how the serial console is defined and how networking is enabled for
this particular STM32 board.

This approach was chosen to fit into PIO's way of doing things, and to keep the
PIO `.ini` files very straightforward: just some _choices_, not the actual
details of _how_ these configuration choices are realised. Those belong in the
`lib/config/` area (this could become quite complex over time).

## Testing

There are two kinds of test: C++ tests, using Test Driven Design techniques in
C++, and Python tests which get loaded on startup with the console output
compared to expected output. Both kinds can be used natively, which is very
quick to run, and on embedded µC boards, which is more realistic but requires
the board to be attached and properly set up.

The C++ tests are very limited at the momend. See `test/main.cpp`, which
includes a number of `.h` files where the actual tests are grouped by category.
For an example, see `test/vec.h` which runs some basic tests for the `Vector`
data type in Monty. There is a `pio test` command which will build the test
suite for the selected embedded platform, upload and run it, and report the test
results. A powerful mechanism.

The other kind of test loads each `.py` test script (after compiling to `.mpy`
by `mpy-cross`) into the Monty VM and compares the output with the expected
output.  These tests are in `verify/*.py`, which has its own makefile, but for a
quick native test, there's `make verify` as shorthand, in the top level
directory.

The output from each `.py` script is saved in a `.out` file, and compared to an
`.exp` text file, if it exists. In case they match, the `.out` file is deleted,
else the first 10 lines of their `diff` are shown.

Running Python tests on embedded boards is (again) rather tricky, and there's
obviously no support in PIO for this very Monty-specific task . The basic idea
is that the VM looks in a predefined RAM location for `.mpy` data, which is
placed there by `gdb` or by `stlink`, followed by a board reset to restart the
VM (for STM32 boards). There is a small `verify/dog.c` utility to capture
output (similar, but not identical, to "cat").

For a quick manual sequence to run a series of tests natively and embedded, try:

1. `make run` - this builds and runs native with `verify/demo.py` as test script
2. `make -k verify` - this runs all native tests, without stopping at errors
3. `pio test` - upload and run the C++ test suite to the attached board
4. `make up` - build and upload a standard Monty VM build to the board
5. `cd test; make` - with the VM loaded in 4), run & compare all Python tests

Currently, a quick way to check something or try a new feature, is to add some
Python code to the end of `verify/demo.py` and use `make run` to see the
results.
