#!/usr/bin/env python3

'''
>>> 1+2
3

>>> mrfs()
usage: ./a.out cmd args...

>>> mrfs("wipe")

>>> img = image()
>>> len(img)
65536
>>> img[:2]
b'\xff\xff'
>>> img[-2:]
b'\xff\xff'

>>> mrfs("dump")
mrfs entries:

>>> mrfs("add", "abc", 2012091234, 5, "hello")
0x...

>>> mrfs("dump")
mrfs entries:
0x000000:     5  202012091234  abc

'''

import doctest, os, subprocess

subprocess.run(["g++", "-std=c++11", "-DTEST", "mrfs.cpp"], check=True)

def mrfs(*args):
    e = subprocess.run(["./a.out", *map(str, args)],
                       capture_output=True, check=True, text=True)
    print(e.stdout, end="")

def image():
    with open("flash.img", "rb") as fd:
        return fd.read()

if __name__ == "__main__":
    failed, tests = doctest.testmod(optionflags=doctest.ELLIPSIS)
    if failed == 0:
        print("OK", tests)
    os.remove("a.out")
