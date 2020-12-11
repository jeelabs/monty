# Monty: a stackless VM
> This area can be browsed using: `python3 -m http.server 9000`

Monty is a Virtual Machine which executes bytecode produced by [MicroPython][MPY].<br>
It's grossly incomplete and totally unfit for general use, but ... it _does_ work.  
There is no compiler, this VM uses converted `.mpy` files from `mpy-cross`.  

This C++ library is written from the ground up, but it would not exist without  
the huge amount of thought and work put into the development of MicroPython,  
which proves that a modern dynamic language can run well on embedded µCs.

The reason for creating this project, is to explore some stackless design ideas.

The source code & docs are in the public domain and under active development.  
Latest commits are at: <https://git.jeelabs.org/jcw/monty/src/branch/dev>.

[MPY]: https://micropython.org/
