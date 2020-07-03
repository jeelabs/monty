This area is for exploring a number of ideas for a stackless VM called "Monty".  
The Monty source code is in a separate repository and needs to be fetched with:

```text
git submodule update --init
```

The default builds are for "native", i.e. MacOS and Linux hosts:

pio run
: compile the native app with [PlatformIO][PIO] (also used by `make`)

pio test
: compile and run all native C++ tests in the `test/` tree

make
: compile native app and run it with `runner/demo.mpy`

make runner
: compile native app and run all tests in `runner/`

make bluepill
: compile for STM32F103 , upload via BMP, and run all tests in `runner/`

make f4disco
: compile for STM32F407 , upload via ST-Link, and run all tests in `runner/`

These last two require the matching board to be connected, using settings  
for serial ports which will need to be adjusted. See `runner/Makefile`.

Sample output for a native build with tests and gc stats:

```text
$ make
./codegen.py
MALLOC_CHECK_=3 pio test
[...]
Testing...
test/vec.cpp:77:smokeTest	[PASSED]
test/vec.cpp:78:instance	[PASSED]
[...]
mpy-cross runner/demo.py
MALLOC_CHECK_=3 .pio/build/native/program runner/demo.mpy
main qstr #166 1276b
0
1
[...]
48
49
done
gc: total    167 allocs    17072 b
gc:  curr     63 allocs     4144 b
gc:   max     66 allocs     4784 b
$
```

Sample output for a remote build, with a number of test scripts:

```text
$ make bluepill
pio run -e bluepill -t upload
[...]
Looking for BlackMagic port...
Use manually specified: /dev/cu.usbmodemDDD8B7B81
[...]
make -C runner BOARD=bluepill
target: bluepill on /dev/cu.usbmodemDDD8B7B83
demo.out
      55      73     275 demo.out
features.out
hello.out
mem.out
$
```

Known issues:

* Too many to list here ... this project is at a very early stage.

-jcw, 2020-06-30

[PIO]: https://docs.platformio.org/en/latest/core/installation.html
