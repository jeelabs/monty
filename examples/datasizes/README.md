Dump the struct sizes of most datatypes in Monty.

To build and run on native (64b), i.e. MacOS or Linux:

```text
$ inv native
   24 b  Array
    8 b  Bool
   24 b  BoundMeth
[...]
    8 b  Value
   16 b  VaryVec
   16 b  Vec
$
```

To build and run on STM32 (32b):

```text
$ inv stm32
   16 b  Array
    4 b  Bool
   12 b  BoundMeth
[...]
    4 b  Value
   12 b  VaryVec
    8 b  Vec
^C
$
```

The STM32 build expects a Nucleo-L432KC board.
