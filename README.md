This `dev` branch is undergoing a complete overhaul. Let's hope it'll end well.

[INV]: https://www.pyinvoke.org
[PIO]: https://platformio.org/install/cli
[MPY]: https://github.com/micropython/micropython

## Development setup

The Monty project is in its very early stages. It takes a bit of work to get the development setup just right, due to a number of dependencies:

* Python 3.x
* PyInvoke 1.x
* PlatformIO 5.x
* MicroPython v1.13

Python is often already present (the Monty setup needs v3).

[PyInvoke][INV] (the `inv` cmd) can be installed with `apt get python-invoke`, `brew install pyinvoke`, `pip3 install pyinvoke`, or something similar. PyInvoke is a simple task launcher, scripted in Python.

For PlatformIO (aka PIO), see [this page][PIO]. PIO is a cross-platform build & upload tool which also takes care of installing all necessary toolchains, frameworks, and libraries.

If the [MicroPython][MPY] (aka µPy) source code is present, Monty's code generator wil scan some of its header files to extract certain strings and constants (it'll keep the old settings if these files are not found).

## Directory structure

The following structure is currently suggested for Monty development:

```text
.
├── git/                # "../../git/" is hard-wired in Monty right now
│   └── micropython/    # µPy source code and mpy-cross build
├── jee/                # need not be separate, could also in "git/"
│   ├── monty/          # main Monty development area, "dev" branch
│   └── monty-conv/     # special checkout of the "conv" branch
└── pio/                # must be a separata area if JeeH is present
    └── jeeh/           # checkout of the latest JeeH version
```

This can probably be simplified and generalised later.

## Monty dev branch

All this takes is the usual `git clone`, but be sure to switch to the `dev` branch:

```text
git clone https://git.jeelabs.org/jcw/monty.git 
cd monty
git branch dev
```

There's a `tasks.py` file for PyInvoke. Try `inv -l` to see the list of commands.

## MicroPython and "mpy-cross"

Monty needs the µPy source code as well as the `mpy-cross` application (to compile Python to bytecode).  The source code can be obtained as follows:

```text
cd git
git clone https://github.com/micropython/micropython.git
```

Then build the `mpy-cross` tool, using:

```text
cd micropython/mpy-cross
make
```

This will create an executable called "mpy-cross", which needs to be in the exec search PATH, for example by doing something like:

```text
cd $HOME/bin
ln -s ../path/to/micropython/mpy-cross/mpy-cross
```

You could now run `cd; mpy-cross` to verify that this utility is found.

## The "monty-conv" build

At the time of writing, a special native build of an older version of Monty is needed to convert `.mpy` files from `mpy-cross` to the new `.mty` file format which recent versions of Monty expect (and which can be stored in MRFS).

This special build can be created as follows (on MacOS or Linux):

```text
git clone https://git.jeelabs.org/jcw/monty.git monty-conv
cd monty-conv
git checkout conv
pio run
```

This creates a native build, which can be verified as follows:

```sh
$ .pio/build/native/program -v
Monty v0.96-2
```

For Monty, it needs to be found in the exec search PATH. Perhaps using something
like this - modify as needed:

```text
cd $HOME/bin
ln -s ../path/to/monty-conv/.pio/build/native/program monty-conv
```

If all is well, `cd; monty-conv` will generate a "can't compile" error message.

## The "JeeH" library

[JeeH][JHL] is a lightweight wrapper for accessing hardware peripherals, on STM32, ESP's, and other platforms. It is used by Monty for basic SysTick, GPIO, UART, and Flash access on STM32. JeeH could be replaced with functions from STM32Cube, libopencm3, or Arduino - JeeH just happens to be efficient and concise for C++ use.

The separate `pio/jeeh/` area is only necessary if you want to use a newer version of JeeH than is currently registered in the [PIO Library index][PLX]. Note that this copy will only be used by PIO if the following setting is also in your environment:

`declare -x PLATFORMIO_LIB_EXTRA_DIRS="$HOME/path/to/pio"`

Adjust as needed (the variable must point to JeeH's _parent_ directory).

If JeeH is not present, PIO will fetch and use the last registered version. This is _usually_ good enough.

[JHL]: https://git.jeelabs.org/jcw/jeeh
[PLX]: https://platformio.org/lib/show/3082/JeeH

## Installation check

Here is a quick way to check the presence of all the tools and their versions:

```text
$ inv health
Darwin x86_64
Python 3.9.0
Invoke 1.4.1
PlatformIO, version 5.0.3
MicroPython v1.13-221-gc8b055717 on 2020-12-05; mpy-cross emitting mpy v5
Monty v0.96-2
```

For a first build on MacOS or Linux, you can now try to run `inv native`.  With an attached Nucleo-L432, try `inv utest`, which will upload and run some tests.

To build a firmware image, use `inv final`. the `monty.bin` file can then be uploaded to the Nucleo-L432 by dragging it to its USD drive, or `cp monty.bin /Volumes/NODE-L432/` (Mac), or `cp monty.bin /media/...` (Linux).
