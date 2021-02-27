#!/usr/bin/env python3

# Monty file watcher
#
# Usage: watcher.py myfile.py
#
#   watch a Python script for changes, and send it to the remote board

import os, sys, time
from runner import openSerialPort, compileAndSend

def watcher(fname):
    stamp = 0
    while True:
        try:
            t = os.stat(fname).st_mtime
        except FileNotFoundError:
            stamp = 1
        else:
            if t != stamp:
                yield fname
            stamp = t
        # show any output coming back, while waiting a bit
        for line in ser.readlines():
            if line[:1] == b'\xFF':
                continue
            line = line.decode().rstrip("\n")
            if line != "main":
                print(line)

py = sys.argv[1]
ser = openSerialPort()

try:
    count = 0
    for fn in watcher(py):
        e = compileAndSend(ser, fn)
        if e:
            print("%s: %s" % (fn, e))
        else:
            ser.readline() # eat first output, which is a 0xFF (???)
            count += 1
            print("<<<", count, ">>>")

except KeyboardInterrupt:
    pass
