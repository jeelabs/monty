# Monty - a stackless VM

There's almost no documentation ... just what's on on this page.

> **January 2021** - This project is in its **very** early stages.

## Directories
```
├── docs/               # this documentation
├── examples/
│   ├── blinker/        # minimal stacklet demo, see the README
│   └── ...
├── lib/
│   ├── arch-native/    # only included in native builds
│   ├── arch-stm32/     # only included in ... yeah, STM32 builds
│   ├── monty/          # main code area for Monty
│   └── ...
├── src/                # small main.cpp and some utility scripts
├── test/               # unity tests, used by "pio test"
│   └── ...
└── ...
```

## Requirements

Dependencies: [Python 3.x][PY3], [PyInvoke][INV], [PlatformIO][PIO], and
[MicroPython][MPY].<br/>
Use `inv health` to verify that everything is found and working:

```
$ inv health
Darwin x86_64
Python 3.8.7
Invoke 1.5.0
PlatformIO Core, version 5.1.0rc1
MicroPython v1.13-221-gc8b055717 on 2020-12-05; mpy-cross emitting mpy v5
```

## Development

* native run: `inv native` (builds and runs on Linux or MacOS hosts)
* native test `inv test` (builds and runs C++ tests on host)
* embedded console: `inv serial` (keep open in a separate shell)
* embedded upload: `pio run` (will build, upload, and reset)
* embedded test: `pio test` (note: don't keep a separate console open)
* for options, use invoke's built-in help, e.g. `inv native -h`.
* for other commands, see `inv -l` (work in progress) and `pio -h`.

> **Warning:** for embedded builds, add a line in the `[stm32]`
section of `platformio.ini` file, containing `lib_deps = JeeH`. See
[pio#3810](https://github.com/platformio/platformio-core/issues/3810).  It's not
needed when using a local copy of JeeH.

By default, the embedded builds are for a Nucleo-32 board with STM32L432KC µC.
To use another STM32 board, create PIO definitions in `platformio-local.ini`,
for example:

```
[platformio]
default_envs = bluepill

[env:bluepill]
extends = stm32
board = bluepill_f103c8
build_flags = ${stm32.build_flags} -DSTM32F1
upload_protocol = blackmagic
```

Other types of embedded boards will require additional code in `lib/arch-*/`.

## Code generation

Some parts of the source code in Monty are generated using the `src/codegen.py`
script, which is exclusively tailored for Monty.  Code generation is done by
scanning all source code at the start of each build and _replacing_ certain
markers with _automatically_ generated results. These makers start with the text
`//CG`. There are a few variants:

* `//CG tag ...` - this is a newly-entered source line which has never been
  processed
* `//CG: tag ...` - the same, but now the code generator has seen it (and
  presumably saved the request for other parts of its processing)
* `//CG1 tag ...` to `//CG3 tag ...` - this line and the next 1..3 lines go
  together, the code generator "owns" those next lines, and changes then as
  needed
* `//CG< tag ...` - this marks a range owned by the code generator, which runs
  until a line with `//CG>` is seen

Note that "owned by" means that the text will be **replaced** by the code
generator as it sees fit. The text inside these ranges should not be edited
manually.

Code generation is used for a wide range of tasks, from repetitive expansions
that would be cumbersome to write by hand in many places, to parsing external
headers and transforming them to information needed in Monty. Another very
common use is to collect information about the code in various places, and then
_generate_ associated code elsewhere, such as dispatch tables for several
classes used all over Monty.

Qstr IDs are also assigned here: the code generator will identify **every**
occurrence of the form `Q(<NNN>,"...")` and replace `<NNN>` with a unique id.
Then, in `qstr.cpp`, all these ids and strings are dumped in `VaryVec` format.

See Invoke's `tasks.py` for details about how and when code generation is
triggered.

[PY3]: https://www.python.org/
[PIO]: https://docs.platformio.org/en/latest/
[MPY]: https://github.com/micropython/micropython/
[INV]: https://www.pyinvoke.org/
