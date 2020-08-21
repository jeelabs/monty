This is a test runner for a µC board connected via a Black Magic probe.  
The current setup is configured for a Blue Pill (STM32F103C8).

```
$ make
target: bluepill on /dev/cu.usbmodemDDD8B7B83
      51      54     168 demo.out
features.out
hello.out
$
```

Each `xyz.py` file is compiled to bytecode, then `gdb` is used to store it  
in the µC's RAM and reset the board. The resulting output is collected and  
saved in `xyz.out`. If there's an `xyz.exp` file, the reults are compared.  
If they do not match, the first 10 lines of the diff are shown. If there's no  
`xyz.exp` file, a word count of the `xyz.out` results is shown instead.  
There is no output for tests which generate exactly their expected output.

> Note: with `make BOARD=native`, the tests will be executed on the host.

To make all of the above work, the following is needed:

* hardware: a [Blue Pill][bp], connected to USB via a [Black Magic Probe][bmp]
* firmware: latest `monty`, built via `make bluepill_f103c8` one directory up
* tools: `mpy-cross` and `arm-none-eabi-gdb`, both must be in `$PATH`
* dog: this is a small cat-like (heh) utility, see `dog.c` - needs `gcc`

[bp]: https://stm32duinoforum.com/forum/wiki_subdomain/index_title_Blue_Pill.html
[bmp]: https://github.com/blacksphere/blackmagic

The purpose of the `dog` utility is to greatly speed up testing: it waits for  
at least one character, then quits when no more output appears for 100 ms.  
For convenience, it also sets the serial port to raw mode and 115200 baud.  
A slower alternative could be created using `stty`, `timeout`, and `cat`.
