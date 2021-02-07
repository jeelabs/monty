# Running & testing

The main target for Monty is _embedded microcontrollers_ ("µCs"), but it also
runs _natively_ on MacOS and Linux, at least w.r.t. the core functionality.
Native mode is much faster to develop in, as it avoids the upload cycle,
benefits from more "direct" development tools, and can run at least an order of
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
everything into PIO, its Scons Python script, and its `extra_scripts.py`
extensibility. Instead of customising PIO "fron the inside out", i.e. adjusting
its behaviour from deep inside its highly automated conventions, there is a much
simpler way to accomplish the same: [PyInvoke](https://www.pyinvoke.org/).

There's an article titled [Building a CLI for Firmware Projects using
Invoke](https://interrupt.memfault.com/blog/building-a-cli-for-firmware-projects)
by Tyler Hoffman, which makes a very strong case for using "inv", as the
shorthand command is called. AMong the benefits:

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
| `inv native` | this is the simplest test, it runs a minimal "hello" Python script |
| `inv native -f pytests/hello.py` | the same test, spelled out in full detail |
| `inv python` | run the full Python test suite, reporting only differences and failures |
| `inv python -t hello,iter,mem` | run only the specified tests (can also be a glob pattern) |
| `inv test` | run the C++ test suite, with results summarised at the end |

The above command work on MacOS (verified on 11.1) and on Linux (verified on
Ubuntu 20.04), and run in a matter of seconds. Here is some sample output:

```
$ inv native
main
hello monty cdd7e0d
done
$ inv python
main
hello monty cdd7e0d
done
32 tests, 32 matches, 0 failures
$ inv test
[...]
Test    Environment    Status    Duration
------  -------------  --------  ------------
array   native         PASSED    00:00:00.504
data    native         PASSED    00:00:00.350
gc      native         PASSED    00:00:00.346
repr    native         PASSED    00:00:00.341
========================= 4 succeeded in 00:00:01.541 =========================
$ 
```

Note that "inv native" includes running the Monty code generator and compiling
any changes in the code. Output will only be shown in the case of warnings or
errors - "inv python" will also do "inv native" to make sure the build is up to
date.

For all Monty development which can be done natively, by far the quickest
edit-run cycle is to edit the source as needed, and then run either `inv python`
or `inv test`, or possibly both: `inv native test`.

## Embedded development

The default µC board for Monty is STMicro's
[Nucleo-L432RC](https://www.st.com/en/evaluation-tools/nucleo-l432kc.html), a
small low-cost board containing an STM32 Cortex M4 µC with 256 kB flash
and 64 kB RAM, an on-board LED, and their ST-Link programmer to
upload/debug/connect over USB.

There is no "big" reason to stick to this board, but it's a convenient
self-contained default for now.  PIO supports [numerous
boards](https://platformio.org/boards), but some work may be needed to adapt the
machine-dependent code in Monty, see `lib/arch-*/`.

Here's how to build and upload the µC firmware and run Python scripts:

| Command | Description |
|---|---|
| `inv flash` | build and upload the firmware to the attached µC board |
| `inv runner` | run the full Python test suite, reporting only differences and failures |
| `inv runner -t hello,iter,mem` | run only the specified tests (can also be a glob pattern) |
| `inv upload` | run the C++ test suite, with results summarised at the end |

These commands look very similar to the native versions, but the process is
quite different as the firmware has to be uploaded before running any Python
tests.  The C++ tests are self contained, PIO will build and upload them, read
back their output, and report the test results, all in one step.

A common workflow is to re-build/-upload the firmware and run Python tests in
one step, e.g. `inv flash python` or `inv flash python -t hello`.

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
fimrware has already been uploaded (use `inv flash runner ...` when in doubt).

The bytecode is sent in Intel HEX fornat, i.e. as plain text lines. On normal
startup, the main app always starts a "command-line task" which listens to the
serial port and recognizes these lines, which start with a ":" (colon).

Once sent, the test runner forces a special restart which causes the board to
launch a VM to interpret the bytecode from a reserved area in RAM, _instead_ of
launching its command-line task. From here on, the bytecode takes over.

With both native and embedded Python tests, the main program starts by sending a
single line with the text "main" and when it ends normally, it sends the text
"done". This is why a simple "inv native" run produces the following output:

```
main
monty hello [etc...]
done
```

?> These main/done lines are only sent when `NDEBUG` is not defined, but for now
that is the standard case.

Adding a new Python test is a matter of adding a new `.py` file to the
`pytests/` directory. A first run will never match, but once the received `.out`
file has been verified to be correct it can simply be renamed to have a `.exp`
extension.

#### Verifying Python results

The "test runner" mentioned above is actually a separate Python script called
`src/runner.py`. It not only sends the bytecode and reads back the test results,
it also compares the output to what is "expected", and does its best to present
all outcomes in a concise manner. Matching tests do not produce any output.

All tests in `pytests/` consist of up to 4 files, e.g. in the case of `hello`:

* **hello.py** - this is the test script itself
* **hello.mpy** - this is the bytecode, as produced by `mpy-cross`
* **hello.out** - this is the output of the test, as received by the runner
* **hello.exp** - this is the _expected_ output, i.e. what `.out` _should_
  contain

Note that the `.out` files are only created and kept if the the received and
expected output do _not_ match.

#### Output matching

In some tests the received results contain text which can vary from one run to
the next. This is the case for object addresses, for example. It also happens
when the output from native and embedded test runs do not match exactly.

To deal with this, the runner supports a limited form of output matching +
substition:

* If the first lines in a `.exp` file start with a "/" (slash), they are treated
  as "/search/replace/" instructions, which the runner then applies each to the
  output _before_ comparing it to the remainder of the `.exp` file.

Here is `pytests/hello.exp`, for example:

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
hello monty cdd7e0d
done
```

Which - after modification - will indeed match the expected output. Only a few
tests rely on this feature, but it can be really effective at keeping tests
working properly.
