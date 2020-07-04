**Monty** is a Virtual Machine which executes bytecode produced by [MicroPython][MPY].  
It's grossly incomplete and totally unfit for general use, but ... it _does_ work.  
There is no compiler, this VM requires an `.mpy` file from `mpy-cross` to run.  

This C++ library is written from the ground up, but it would not exist without  
the huge amount of thought and work put into the development of MicroPython,  
which proves that a modern dynamic language can run well on embedded µCs.

The reason for creating this project, is to explore some stackless design ideas.

This project depends on three executables: 1) [PlatformIO][PIO] (the `pio` command),  
2) MicroPython's `mpy-cross`, and 3) `python3` for `pio` & `src/codegen.py`.  
These should be in the shell's `$PATH`. PlatformIO will automatically install all  
the toolchains it needs, but for native builds, `gcc` needs to be present as well.

Once these requirements are met, just run `make` to get started.

The Monty source code is in the public domain and "under active development".

[MPY]: https://micropython.org/
[PIO]: https://docs.platformio.org/en/latest/core/installation.html
