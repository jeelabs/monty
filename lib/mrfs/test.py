#!/usr/bin/env python3

'''
>>> 1+2
3

>>> mrfs()
usage: ./a.out cmd args...

>>> mrfs("wipe")

>>> ff = image()
>>> len(ff)
65536
>>> ff[:2]
b'\xff\xff'
>>> ff[-2:]
b'\xff\xff'

>>> mrfs("dump")

>>> mrfs("add", "abc", 2012091201, 5, "hello")
0

>>> mrfs("dump")
00000:     5  202012091201  abc

>>> ff = image()
>>> ff[:4]
b'MTY0'
>>> ff[8:14]
b'hello\xff'
>>> ff[60:64]
b'DCBA'

>>> mrfs("add", "def", 2012091202, 33, "DEF")
2

>>> mrfs("add", "ghi", 2012091203, 31, "GHI")
5

>>> mrfs("add", "jkl", 2012091204, 0, "")
7

>>> mrfs("dump")
00000:     5  202012091201  abc
00002:    33  202012091202  def
00005:    31  202012091203  ghi
00007:     0  202012091204  jkl

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
