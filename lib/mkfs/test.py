#!/usr/bin/env python3

'''
>>> 1+2
3

>>> mkfs("wipe")

>>> img = image()
>>> len(img)
65536
>>> img[:2]
b'\xff\xff'
>>> img[-2:]
b'\xff\xff'

>>> mkfs("dump")
mkfs entries:

'''

import doctest, os, subprocess

subprocess.run(["g++", "-std=c++11", "-DTEST", "mkfs.cpp"], check=True)

def mkfs(*args):
    e = subprocess.run(["./a.out", *map(str, args)],
                       capture_output=True, check=True, text=True)
    print(e.stdout, end="")

def image():
    with open("flash.img", "rb") as fd:
        return fd.read()

if __name__ == "__main__":
    try:
        doctest.testmod()
    finally:
        os.remove("a.out")
    print("OK")
