# Development setup

## Requirements

This project depends on three executables:

1. [PlatformIO][PIO] (the `pio` command) for native and embedded builds
2. [MicroPython](https://micropython.org/)'s `mpy-cross` to compile to `.mpy`
   files
3. [Python 3](https://www.python.org/) is used by `pio` and `src/codegen.py`

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

This project uses PlatformIO (PIO) for builds and C++ tests. It needs a
`platformio.ini` file to specify configuration settings and library
dependencies. To handle different build targets, several `.ini` files are
present in the `configs/` directory. Their name is based on the name PIO uses
for each [board type](https://platformio.org/boards).

One way to use a specific configuration, is via the `-c` option:

    pio run -c configs/bluepill_f103c8.ini

But it's more convenient to set up a `platformio.ini` file in the top directory,
e.g.

    [platformio]
    extra_configs = configs/bluepill_f103c8.ini

This allows further streamlining by also defining the serial port names for
`platformio.ini`:

    [env:bluepill_f103c8]
    upload_port = /dev/cu.usbmodemDDD8B7B81
    monitor_port = /dev/cu.usbmodemDDD8B7B83
    test_port = /dev/cu.usbmodemDDD8B7B83

Now, compiling becomes a matter of typing `pio run`, uploading is `pio run -t
upload`, etc.

The code in `lib/` is _normally_ automatically found and included by PIO, but
this relies on mentioning their header files somewhere in the main code, which
is not always convenient. That's why some configurations explicity list the
libraries as a `lib_deps = ...` setting.

As shorthand, the makefile includes some convenience targets:

* `make run` compiles and runs a native build, i.e. for macOS or Linux
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
example of how networking is enabled for this particular STM32F103 board.

This approach was chosen to fit into PIO's way of doing things, and to keep the
PIO `.ini` files very straightforward: just some _choices_, not the
actual details of _how_ these configuration choices are realised. Those belong
in the `lib/config/` area (which might become quite complex over time).
