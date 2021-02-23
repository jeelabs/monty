Dump the struct sizes of most datatypes in Monty.

To build and run on MacOS or Linux (64b):

```text
$ inv
   24 b  Array
    8 b  Bool
   24 b  BoundMeth
[...]
   16 b  VaryVec
   16 b  Vec
   16 b  Vector
$
```

To build and run on STM32 (32b):

```text
$ inv stm32
   16 b  Array
    4 b  Bool
   12 b  BoundMeth
[...]
   12 b  VaryVec
    8 b  Vec
   12 b  Vector
^C
$
```

The STM32 build expects a Nucleo-L432KC board to be connected.
