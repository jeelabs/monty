#!/usr/bin/env python3

# Monty test runner
#
# Usage: runner.py file...
#
#   files with a .py extension are first compiled to .mpy using mpy-cross,
#   but only if the .mpy does not exist or is older than the .py source

import os, subprocess, sys, time
import serial, serial.tools.list_ports

def findSerialPorts():
    """map[serial#] = (prod, dev)"""
    found = []
    for p in serial.tools.list_ports.comports():
        if p.product:
            prod = p.product.split(',')[0]
            if prod.startswith('Black Magic Probe') and p.device[-1] == '1':
                continue # ignore BMP's upload port
            found.append((prod, p.device, p.serial_number))
    return found

def openSerialPort():
    port = None
    serials = findSerialPorts()
    for prod, dev, serid in serials:
        print(f"{prod}: {dev} ser# {serid}")
        port = serial.Serial(dev, 115200, timeout=0.1)
    assert len(serials) == 1, f"{len(serials)} serial ports found"
    return port

# convert bytes to intel hex, lines are generated per 32 bytes, e.g.
#   :200000004D05021F2054100A00071068656C6C6F2E70790011007B100A68656C6C6F100AC9
#   :0C002000776F726C643402595163000069
#   :00000001FF

def genHex(data):
    for i in range(0, len(data), 32):
        chunk = data[i:i+32]
        csum = -(len(chunk) + (i>>8) + (i&0xFF) + sum(chunk)) & 0xFF
        yield f":{len(chunk):02X}{i:04X}00{chunk.hex().upper()}{csum:02X}\n"
    yield ":00000001FF\n"

ser = openSerialPort()

args = sys.argv[1:]
fail = 0

for fn in args:
    print(fn + ":")
    ser.reset_input_buffer()

    ser.write(b'\nbc\nwd 250\n')
    failed = False

    try:
        with open(fn, "rb") as f:
            for line in genHex(f.read()):
                ser.write(line.encode())
                ser.flush()
    except FileNotFoundError:
        print("file?", fn)
        failed = True

    for line in ser.readlines():
        try:
            line = line.decode().rstrip("\n")
        except UnicodeDecodeError:
            pass # keep as bytes if there is raw data in the line
        print(line)
        if line == "abort":
            time.sleep(1.1);
            failed = True

    if failed:
        fail += 1

print(f"{len(args)} tests, {fail} failures")
