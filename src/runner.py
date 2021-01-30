#!/usr/bin/env python3

# Monty test runner
#
# Usage: runner.py file...
#
#   file with a .py extension are first compiled to .mpy using mpy-cross
#   this is only done if the .mpy file is absent or older

import serial, serial.tools.list_ports
import time

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
        port = serial.Serial(dev, 115200)
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

with open("hello.mpy", "rb") as f:
    for line in genHex(f.read()):
        time.sleep(0.01)
        ser.write(line.encode())
