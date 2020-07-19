# Monty: a stackless VM

**Monty** is a Virtual Machine which executes bytecode produced by [MicroPython][MPY].<br>
It's grossly incomplete and totally unfit for general use, but ... it _does_ work.  
There is no compiler, this VM requires an `.mpy` file from `mpy-cross` to run.  

This C++ library is written from the ground up, but it would not exist without  
the huge amount of thought and work put into the development of MicroPython,  
which proves that a modern dynamic language can run well on embedded µCs.

The reason for creating this project, is to explore some stackless design ideas.

The source code & docs are in the public domain and under active development.  
Latest commits are at: <https://git.jeelabs.org/jcw/monty/commits/branch/dev>.

[MPY]: https://micropython.org/
