#!/usr/bin/env python3

'''
>>> mrfs()
usage: ./a.out cmd args...

>>> mrfs("wipe")

>>> ff = image()
>>> len(ff)
32768
>>> ff[:2]
b'\xff\xff'
>>> ff[-2:]
b'\xff\xff'

>>> mrfs("dump")

>>> mrfs("add", "abc", 2012091201, 5, "hello")
0

>>> mrfs("dump")
0000:     5  201209.1201  abc

>>> ff = image()
>>> ff[:4]
b'mty0'
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
0000:     5  201209.1201  abc
0002:    33  201209.1202  def
0005:    31  201209.1203  ghi
0007:     0  201209.1204  jkl

>>> mrfs("add", "def", 0, 0, "") # time zero is deletion
8

>>> mrfs("add", "ghi", 2012091205, 3, "GHI") # new version
9

>>> mrfs("dump")
0000:     5  201209.1201  abc
0002:    33  201209.1202  def
0005:    31  201209.1203  ghi
0007:     0  201209.1204  jkl
0008:     0       0.0000  def
0009:     3  201209.1205  ghi

>>> mrfs("find", "abc")
0
>>> mrfs("find", "def")
-1
>>> mrfs("find", "ghi")
9
>>> mrfs("find", "jkl")
7
>>> mrfs("find", "mno")
-1

'''

import doctest, os, subprocess

subprocess.run(["g++", "-std=c++11", "-DTEST", "mrfs.cpp"], check=True)

def mrfs(*args):
    e = subprocess.run(["./a.out", *map(str, args)],
                       capture_output=True, check=True, text=True)
    print(e.stdout, end="")

def image():
    with open("rom.mrfs", "rb") as fd:
        return fd.read()

if __name__ == "__main__":
    failed, tests = doctest.testmod(optionflags=doctest.ELLIPSIS)
    if failed == 0:
        print("OK", tests)
    os.remove("a.out")
    os.remove("rom.mrfs")
