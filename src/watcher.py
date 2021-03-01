#!/usr/bin/env python3

# Monty file watcher
#
# Usage: watcher.py [-r] myfile.py
#
#   watch a Python script for changes, and re-run the script whenever it does
#   default mode is native, if "-r" is specified: send it to the remote board

import os, subprocess, sys, time
from runner import openSerialPort, compileIfOutdated, compileAndSend

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
        if ser:
            for line in ser.readlines():
                if line == b'\xFF\n':
                    continue
                try:
                    line = line.decode().rstrip("\n")
                except UnicodeDecodeError:
                    line = repr(line)
                if line in ["done", "abort"]:
                    print("\x1B[33m%s\x1B[0m" % line) # yellow
                elif line != "main":
                    print(line)
        else:
            time.sleep(0.1)

py = sys.argv[1]
ser = None
if py == "-r":
    py = sys.argv[2]
    ser = openSerialPort()

try:
    count = 0
    for fn in watcher(py):
        # the logic and order-of-events for remote vs native differ completely
        if ser:
            e = compileAndSend(ser, fn)
            if e:
                print("\x1B[31m%s: %s\x1B[0m" % (fn, e)) # red
            else:
                ser.readline() # eat first output, which is a 0xFF (???)
                count += 1
                print("\x1B[32m<<<", count, ">>>\x1B[0m") # green
        else:
            try:
                mpy = compileIfOutdated(fn)
                count += 1
                print("\x1B[32m<<<", count, ">>>\x1B[0m") # green
                subprocess.run([".pio/build/native/program", mpy])
            except Exception as e:
                print("\x1B[31m%s: %s\x1B[0m" % (fn, e)) # red

except KeyboardInterrupt:
    pass
