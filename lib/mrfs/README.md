# Minimal Replaceable File Storage

This is a very simple mechanism to store named "files" in flash. It's sort of a
file system, although it's perhaps better described as a key-value store,
whereby later file entries supersede earlier ones.  A simple timestamp is also
saved, and a zero timestamp marks a deleted entry.


This MRFS code can be compiled natively to generate a test utility, or included
in Monty for embedded use (it does not _depend_ on Monty). The `test.py` file
contains a very simple test (and demonstration) script, which compiles the code
natively, runs some tests, and cleans up again - here is a successful test run:

```
$ ./test.py
OK 25
$
```

The basic idea is to always _append_ new entries, until space runs low, and
then to copy a few still-valid first entries _upwards_ to allow erasing the first
flash page. Once erased, all still-valid entries can be copied _downwards_ to
clear and release more flash pages, thus compacting all files to the start of
flash memory.

> Note: the current code is not nearly as clever: it can only append and will
> simply fail when space runs out. For now, the workaround is to "wipe" all
> flash and start from scratch before this happens.

This design requires very little code, acts as a form of wear-leveling for
flash, and keeps each entry as a single contiguous area in addressable memory,
as is needed for Monty (unlike LittleFS, for example).
