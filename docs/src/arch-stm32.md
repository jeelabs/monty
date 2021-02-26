# arch-stm32/\*

STM32 is the embedded architecture for which Monty is currently being developed.
That doesn't mean it's the _only_ one, or will remain the main one forever, but
_ya' gotta start somewhere..._

More precisely, the current default embedded target is the
[Nucleo-L432KC](https://www.st.com/en/evaluation-tools/nucleo-l432kc.html),
which runs at up to 80 MHz, has 256 kB of flash, and 64 kB of RAM. It's an ARM
Cortex-M4 with hardware floating point.

Each architecture has its own area, with at least `arch.h` and `library.json`
files in it. The JSON file is used by PlatformIO and has a setting to make sure
that this area is _only_ used for STM32 builds:

```json
{ "platforms": "ststm32" }
```

The `arch.h` file is included from `main()`. This tells PIO to include all the
sources in this directory in the build. The idea is simple: put definitions in
`arch.h` and their implementation in, say, `arch.cpp`.

## namespace arch

By placing all the definitions in a namespace, the function names can be kept
short and to the point:

* `arch::init()` is called as first step in `main()`, with an optional int
  argument to specify how much memory to allocate for Monty's object and vector
  pool
* `arch::idle()` is called when the run loop indicates that there is currently
  no work to be done
* `arch::done()` is called when the run loop exits because there is no more - in
  an embedded context, the only meaningful action will be to either power down
  completely, or to restart

Additional code can be added if more functionality is needed which is either not
available on other platforms, or requires a (very) different implementation.

## console commands

The Nucleo includes an on-board "ST-Link" which is used to upload new code to
the µC, to provide a JTAG debugger interface, and as a virtual serial USB port
pass-through, for use as console.

This console connection is used for all `printf` output from Monty, and to
support a _tiny_ command interpreter. All communication over this serial
connection is currently in plain text mode, i.e. each input line is treated as
one command:

* if the line starts with ":", it's treated as a special Intel HEX upload
  request
* if MRFS is enabled and the command is the name of one of the bytecode files in
  MRFS, that bytecode is loaded and executed using the Python VM
* if it's one of a few built-in commands, it's run directly

The following built-in commands are currently implemented for STM32:

```text
bc *  set boot command [cmd ...]
bv    show build version
di    show device info
gc    trigger garbage collection
gr    generate a GC report
ls    list files in MRFS
od    object dump
pd    power down
sr    system reset
vd    vector dump
wd N  set watchdog [0..4095] x8 ms
```

Most of these commands are geared towards debugging, but with the ability to
"run" bytecode files stored in MRFS, it also feels suprisingly like a small
_Unix'y_ shell (without a prompt):

```text
$ inv serial
bv
Monty v1.0 (Feb 26 2021, 00:37:34)
di
flash 0x08000000..0x0800DB1C, ram 0x20000000..0x20000DA4, stack top 0x20010000
cpuid 0x410FC241, 256 kB flash, 64 kB ram, package type 8
clock 80000 kHz, devid 00300029-324B500E-20363642
hello
hello monty v1.0
gr
gc: max 11528 b, 1 checks, 0 sweeps, 0 compacts
gc: total      6 objs      416 b,    306 vecs     4736 b
gc:  curr      6 objs      416 b,      6 vecs      360 b
gc:   max      6 objs      416 b,      9 vecs      736 b
^C
```

The Intel HEX upload facility is used by the Python test "runner" (`runner.py`)
to send bytecode to the µC and then execute it. It's also used by `mrfs.py` to
re-flash the stored MRFS data files.

## module machine

The `machine` module is for use from Python. It currently defines the following:

* `machine.ticks` is the number of milliseconds since program start
* `machine.ticker(N)` can be used to set up a periodic tick event every N
  milliseconds - the event object is returned, supporting `wait`, `set`, and
  `clear` methods

The above two functions are also defined for native builds. What follows is
only on STM32:

* `machine.pins` - direct access to the GPIO pins
* `machine.spi()` - set up an SPI bus on 4 specific I/O pins
* `machine.rf69()` - a driver for the RFM69 wireless radio module
* `machine.dog()` - set up a hardware watchdog with timeout between 8 ms and 32
  sec
* `machine.kick()` - kick the watchdog so it won't reset the µC

These functions are highly experimental at this stage. The following test
scripts illustrate some of the functionality that is currently available:

* `pytests/s_blinker.py` - blink the Nucleo's on-board LED on pin PB3
* `pytests/s_spi.py` - define an SPI bus on pins PB7, PB6, PB0, and PA12
* `pytests/s_rf69.py` - connect to the RFM69 and report any received packets

The `s_rf69.py` example also illustrates how to use the hardware watchdog.

If nothing else, this code for the `machine` module is an example of how Monty
can be extended in C++ in such a way that it's also usable from the Python VM.
