# Running & testing

The main target for Monty is _embedded microcontrollers_ ("µCs"), but it also
runs _natively_ on MacOS and Linux, at least w.r.t. the core functionality.
Native mode is much faster to develop in, as it avoids the upload cycle,
benefits from more "direct" development tools, and will run at least an order of
magnitude faster.

Embedded software development tends to cover a lot of ground: cross-compilation
vs native builds, C++ and Python test suites, firmware uploading, remote console
port access, hardware-level debugging, coverage- and performance-analysis - the
list is endless.  [PlatformIO](https://docs.platformio.org/en/latest/) ("PIO")
is an excellent tool for these tasks as it takes care of all the toolchain
details, is well-documented, and very eay to use. It's also highly extensible
via Python "extra scripts", which tie into the underlying
[SCons](https://scons.org).

But with such a variety of tasks, it can be somewhat of a burden to shoehorn
everything into PIO.  Instead of customising PIO "fron the inside out", i.e.
adjusting its behaviour from deep inside its many conventions, there is a
simpler way to accomplish the same: [PyInvoke](https://www.pyinvoke.org/).

The article titled [Building a CLI for Firmware Projects using
Invoke](https://interrupt.memfault.com/blog/building-a-cli-for-firmware-projects)
by Tyler Hoffman makes a strong case for using "inv", as the shorthand command
is called. Among the benefits:

* it's self-documenting: just type `inv -h`, or `inv -h <command>`
* it's driven by Python, as are all the other tools in the Monty project
* being Python, many basic utilities are not needed (grep, sed, wc, etc)
* complex automation is easier to read & write as Python than as Bash script
* invoke is self-documenting - _yes, again ... but this is a biggie, and worth
  repeating_

## Native development

Here's how to build the native executable and run Python scripts:

| Command | Description |
|---|---|
| `inv` | same as `inv native`, used for quick compile checks |
| `inv native` | this is the simplest test, it runs a minimal "hello" Python script |
| `inv native -f hello` | the same test, with script from `test/py/` |
| `inv native -f test/py/hello.py` | again the same, this time spelled out in full detail |
| `inv native -f test/py/hello.mpy` | refers to the bytecode file, `inv` will not launch `mpy-cross` |
| `inv python` | run the full Python test suite, reporting only differences and failures |
| `inv python -t hello,mem` | run only the specified tests (can also be a glob pattern) |
| `inv test` | run the C++ test suite, with results summarised at the end |

Tests named `<letter>_*.py` are only included if they match the platform: `n_*`
is only for native tests, `s_*` is only for tests on STM32 (such as `s_spi.py`).

The above commands work on MacOS (verified on 11.2) and on Linux (verified on
Ubuntu 20.04):

```
$ inv
main
hello monty v1.0
done
$ inv python
36 tests, 36 matches, 0 failures, 5 skipped, 0 ignored
$ inv test
Test    Environment    Status    Duration
------  -------------  --------  ------------
array   native         PASSED    00:00:00.505
data    native         PASSED    00:00:00.498
gc      native         PASSED    00:00:00.515
repr    native         PASSED    00:00:00.489
========================= 4 succeeded in 00:00:02.007 =========================
$ 
```

Without args, "inv" will run the Monty code generator and compile any changes
in the code - "inv python" will also do this, to make sure the build is up to
date. Output is only shown in the case of warnings or errors.

For all Monty development which can be done natively, by far the quickest
edit-run cycle is to edit the source as needed and run `inv`, which compiles and
shows the output from `hello.py` in under a second.  For a more complete C++ and
Python test, run `inv test python` (takes under 10s).

## Embedded development

The default µC board for Monty is STMicro's
[Nucleo-L432RC](https://www.st.com/en/evaluation-tools/nucleo-l432kc.html), a
small board containing an ARM Cortex M4 µC with 256 kB flash and 64 kB RAM, an
on-board LED, and the ST-Link programmer to upload, debug, and communicate over
USB.

The Nucleos are self-contained, low-cost,
widely available, with a modern low-power architecture and a wide range of
built-in peripherals.  PIO supports
[numerous boards](https://platformio.org/boards), but some work may be
needed to adapt the machine-dependent aspects for Monty, see the [Platform
support](platform-support) section for more details.

Here's how to build and upload the µC firmware and run Python scripts:

| Command | Description |
|---|---|
| `inv flash` | build and upload the firmware to the attached µC board |
| `inv mrfs` | wrap all test scripts as MRFS and upload to a separate flash area |
| `inv runner` | run the full Python test suite, reporting only differences and failures |
| `inv runner -t hello,mem` | run only the specified tests (can also be a glob pattern) |
| `inv upload` | run the C++ test suite, with results summarised at the end |
| `inv builds` | show firmware image sizes of a few embedded build variations |
| `inv all` | run all C++ and Python tests, native and embedded, and more ... |

(MRFS is Monty's lightweight mechanism for storing files in flash, see [this
section](/#/?id=so-what-is-monty) for more details)

Tests named `s_*` are included on STM32, all others starting with `<letter>_`
will be skipped.

These commands look very similar to the native versions, but the process is
quite different as the firmware has to be uploaded before running any Python
tests.  The C++ tests are self contained, PIO will then build and upload them,
read back their output, and report the test results. The `inv flash` command is
needed to re-build and re-upload whenever the C++ code has changed, or after
using `inv upload`, which uploads other test firmware.

A convenient workflow is to re-build & re-upload the firmware and run Python
tests, all in one step, e.g. `inv flash runner` or `inv flash runner -t hello`.

These "remote" tests take a little more time, due to the uploading and serial
communication involved (the console runs at 115200 baud). There is also the
possibility that the board will "hang" during the Python tests. The test runner
uses timeouts to detect and report this, but a reset (or re-upload) will be
needed to get the board out of this state.

## C++ test details

All C++ tests use PIO's
[Unity](https://docs.platformio.org/en/latest/plus/unit-testing.html#api) unit
test framework. There are a number of test applications at `test/*/main.cpp`,
which are built and executed separately. The main pupose of these C++ tests is
to verify the more low-level details of Monty, i.e. its data structures, garbage
collector, and such.

Adding tests is a matter or either
adding code to one of these existing apps, or creating a new one in a sibling
directory.

For a single quick test (of just the GC, for example), a direct PIO command will
be quicker: `pio test -f gc`. This is the same command that `inv upload` issues
under the hood, but with an extra "filter" to only run `test/gc/main.cpp`.

## Python test details

Python tests are performed by compiling the Python script and sending the binary
bytecode file to the remote board over serial. This assumes that the latest
firmware has already been uploaded (use `inv flash runner ...` when in doubt).

The bytecode is sent in Intel HEX fornat, i.e. as plain text lines. On normal
startup, the main app always starts a "command-line task" which listens to the
serial port and recognizes these lines, which start with a ":" (colon).

Once sent, the test runner forces a special restart which causes the board to
launch a VM to interpret the bytecode from a reserved area in RAM, _instead_ of
launching its command-line task. From here on, the bytecode takes over.

With both native and embedded Python tests, the main program starts by sending a
single line with the text "main" and when it ends normally, it sends the text
"done". This is why a simple "inv native" run produces the following output:

```text
main
monty hello v1.0
done
```

?> These main/done lines are only sent when `NDEBUG` is not defined, but for now
that is the standard case.

Adding a new Python test is a matter of adding a new `.py` file to the
`test/py/` directory. A first run will never match, but once the received `.out`
file has been verified to be correct it can simply be renamed to have a `.exp`
extension.

#### Verifying Python results

The "test runner" mentioned above is actually a separate Python script called
`src/runner.py`. It not only sends the bytecode and reads back the test results,
it also compares the output to what is "expected", and does its best to present
all outcomes in a concise manner. Matching tests do not produce any output.

All tests in `test/py/` consist of up to 4 files, e.g. in the case of `hello`:

* **hello.py** - this is the test script itself
* **hello.mpy** - this is the bytecode, as produced by `mpy-cross`
* **hello.out** - this is the output of the test, as received by the runner
* **hello.exp** - this is the _expected_ output, i.e. what `.out` _should_
  contain

The `.out` files are only created and kept if the the received and expected
output do _not_ match.

#### Output matching

In some tests the received results contain text which can vary from one run to
the next. This is the case for object addresses, for example. It also happens
when the output from native and embedded test runs do not match exactly.

To deal with this, the runner supports a limited form of output matching +
substition:

* If the first lines in a `.exp` file start with a "/" (slash), they are treated
  as "/search/replace/" instructions, which the runner then applies each to the
  output _before_ comparing it to the remainder of the `.exp` file.

Here is `test/py/hello.exp`, for example:

```
/monty .+/monty {VERS}/
main
hello monty {VERS}
done
```

Here, the test output includes a _git version ID_ embedded into the application,
which may change from one build to the next. By applying this rule, the _actual_
output will be adjusted to match the expected text. Here is some _actual_
output:

```
main
hello monty v1.0
done
```

Which - after performing the specified replacement(s) - will indeed match the
expected output. Only a few tests rely on this feature, but it can be really
effective to keep test output limited to "real" differences.

## Running a full test

The most complete build and test is `inv all`, which is shorthand for:

```text
inv clean test python upload flash mrfs runner builds examples
```

By default, this assumes that a Nucleo-L432 has been plugged into USB.  
The full test will complete in under a minute:

```text
$ inv all
Test    Environment    Status    Duration
------  -------------  --------  ------------
array   native         PASSED    00:00:00.881
data    native         PASSED    00:00:00.556
gc      native         PASSED    00:00:00.582
repr    native         PASSED    00:00:00.555
============================ 4 succeeded in 00:00:02.573 ============================
36 tests, 36 matches, 0 failures, 5 skipped, 0 ignored
Test    Environment    Status    Duration
------  -------------  --------  ------------
array   nucleo-l432    PASSED    00:00:04.588
data    nucleo-l432    PASSED    00:00:03.478
gc      nucleo-l432    PASSED    00:00:03.623
repr    nucleo-l432    PASSED    00:00:03.464
============================ 4 succeeded in 00:00:15.153 ============================
Processing nucleo-l432 (board: nucleo_l432kc; platform: ststm32; framework: cmsis)
STM32 STLink: /dev/cu.usbmodem143202 ser# 066BFF555052836687031442
upload 0x040E0 done, 16608 bytes sent
STM32 STLink: /dev/cu.usbmodem143202 ser# 066BFF555052836687031442
37 tests, 37 matches, 0 failures, 2 skipped, 2 ignored
   text	   data	    bss	    dec	    hex	filename
  56048	   2480	   2472	  61000	   ee48	.pio/build/nucleo-l432/firmware.elf
  43184	   2480	   2472	  48136	   bc08	.pio/build/noassert/firmware.elf
  33988	   2372	   2472	  38832	   97b0	.pio/build/nopyvm/firmware.elf
blinker
minimal
pipeline
structs
switcher
$
```

Where ... for both `inv python` (native) and `inv runner` (embedded):

* `tests` is the number of test which were actually performed
* `matches` is the number of these that matched the expected output
* `failures` is the number of tests which did not complete successfully
* `skipped` is the number of tests which do not apply to this platform
* `ignored` counts the number of tests which were explicitly skipped (`-i`)

The examples listed at the end are only verified to compile, not actually run.

?> The above description of `inv all` is not _100%_ accurate. To be able to
run repeatable tests, a few tests will be ignored (for example: the `gcoll.py`
output is not identical for 64-bit native and 32-bit STM32). The tests skipped
by `inv all` are listed in the `[invoke]` section of `platformio.ini`.
