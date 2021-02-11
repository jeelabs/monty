This is a basic processing pipeline experiment.

To build and run:

```text
$ inv run bv,-h,gr,../../pytests/hello.mpy
Monty pipeline (Feb 10 2021, 21:22:25)
  bv    show build version
  gc    trigger garbage collection
  gr    generate a GC report
  od    object dump
  vd    vector dump
  -h    this help
gc: max 10176 b, 0 checks, 0 sweeps, 0 compacts
gc: total      0 objs        0 b,      2 vecs       32 b
gc:  curr      0 objs        0 b,      2 vecs       32 b
gc:   max      0 objs        0 b,      2 vecs       32 b
hello monty pipeline
$
```
