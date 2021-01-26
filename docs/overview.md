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

## Development

Dependencies: [PlatformIO][PIO], [PyInvoke][INV], and [MicroPython][MPY].

* native run: `inv native` (builds and runs on Linux or MacOS hosts)
* native test `inv test` (builds and runs C++ tests on host)
* embedded console: `inv serial` (keep open in a separate shell)
* embedded upload: `pio run` (will build, upload, and reset)
* embedded test: `pio test` (note: don't keep a separate console open)

For other commands, see `inv -l` (some don't work) and `pio -h`.

> **Warning:** for embedded builds, you must add a line in the `[stm32]`
section of `platformio.ini` file, containing `lib_deps = JeeH`. See
[pio#3810](https://github.com/platformio/platformio-core/issues/3810).  It's not
needed when using a local copy of JeeH.

By default, the embedded builds are for a Nucleo-32 board with STM32L432KC µC.
To use another board, create PIO definitions in `platformio-local.ini`, for
example:

```
[platformio]
default_envs = bluepill

[env:bluepill]
extends = stm32
board = bluepill_f103c8
build_flags = ${stm32.build_flags} -DSTM32F1
upload_protocol = blackmagic
```

[PIO]: https://docs.platformio.org/en/latest/
[MPY]: https://github.com/micropython/micropython
[INV]: https://www.pyinvoke.org
