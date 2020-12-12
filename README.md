**Monty** is a Virtual Machine which runs bytecode produced by [MicroPython][MPY].  
It's grossly incomplete and totally unfit for general use, but ... it _does_ work.  
There is no compiler, this VM uses converted `.mpy` files from `mpy-cross`.  

This project is written from the ground up in C++, but it would not exist without  
the huge amount of thought and work put into the development of MicroPython,  
which proves that a modern dynamic language can run well on embedded µCs.

The reason for creating this project, is to explore some stackless design ideas.

The source code & docs are in the public domain and under active development.  
Latest commits are always at: <https://git.jeelabs.org/jcw/monty/src/branch/dev>  
Build and test notes: <https://git.jeelabs.org/jcw/monty/wiki/Development-setup>  

[MPY]: https://micropython.org/
