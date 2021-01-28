# Getting started

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

By default, the embedded builds are for a Nucleo-32 board with STM32L432KC
µC.<br/>
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

[PY3]: https://www.python.org/
[PIO]: https://docs.platformio.org/en/latest/
[MPY]: https://github.com/micropython/micropython/
[INV]: https://www.pyinvoke.org/
