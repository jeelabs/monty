This is a basic processing pipeline experiment.

To build and run:

```text
$ inv run bv,-h,gr,../../pytests/hello.mpy,gr
Monty pipeline (Feb 15 2021, 15:17:54)
  bv    show build version
  gc    trigger garbage collection
  gr    generate a GC report
  od    object dump
  vd    vector dump
  -h    this help
gc: max 10032 b, 0 checks, 0 sweeps, 0 compacts
gc: total      4 objs      144 b,      2 vecs       32 b
gc:  curr      4 objs      144 b,      2 vecs       32 b
gc:   max      4 objs      144 b,      2 vecs       32 b
hello monty pipeline
gc: max 6944 b, 1 checks, 0 sweeps, 0 compacts
gc: total      9 objs      464 b,      9 vecs     2880 b
gc:  curr      9 objs      464 b,      7 vecs     2656 b
gc:   max      9 objs      464 b,      8 vecs     2800 b
$
```
