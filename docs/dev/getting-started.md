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

**MacOS & Linux**

| Type | Command | Notes |
|------|---------|-------|
| single | `inv native` | use `-f` flag to specify which file |
| C++ test | `inv test` | based on Unity, see `test/*/main.cpp` |
| Python test | `inv python` | runs all the code from `pytests/` |

**Embedded µC**

| Type | Command | Notes |
|------|---------|-------|
| console | `inv serial` | keep this open in a separate window |
| flash | `inv flash` | build & upload, same as `pio run -s` |
| C++ test | `inv upload` | uploads and runs each Unity test |
| Python test | `inv runner` | uploads and runs each test (needs `flash`) |

?> The C++ and Python tests need access to the console, it should not be kept
open during those tasks.

**Other tasks**

There are several other tasks (the `x-*` tasks are mostly special-purpose):

| Type | Command | Notes |
|------|---------|-------|
| list | `inv -l` | lists all available tasks, see `tasks.py` |
| help | `inv -h` | help about the `invoke` command |
| info | `inv serial -h` | information about a specific task |

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

Other embedded platforms will require additional code in `lib/arch-*/`.

[PY3]: https://www.python.org/
[PIO]: https://docs.platformio.org/en/latest/
[MPY]: https://github.com/micropython/micropython/
[INV]: https://www.pyinvoke.org/
