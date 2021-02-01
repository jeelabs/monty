#!/usr/bin/env python3

# Monty test runner
#
# Usage: runner.py file...
#
#   files with a .py extension are first compiled to .mpy using mpy-cross,
#   but only if the .mpy does not exist or is older than the .py source

import os, subprocess, sys, time
# moved: import serial, serial.tools.list_ports

def findSerialPorts():
    """map[serial#] = (prod, dev)"""
    import serial.tools.list_ports
    found = []
    for p in serial.tools.list_ports.comports():
        if p.product:
            prod = p.product.split(',')[0]
            if prod.startswith('Black Magic Probe') and p.device[-1] == '1':
                continue # ignore BMP's upload port
            found.append((prod, p.device, p.serial_number))
    return found

def openSerialPort():
    import serial
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

def compileIfOutdated(fn):
    root, ext = os.path.splitext(fn)
    if ext != '.py':
        return fn
    mpy = root + ".mpy"
    mtime = os.stat(fn).st_mtime
    if not os.path.isfile(mpy) or (mtime >= os.stat(mpy).st_mtime):
        subprocess.run(["mpy-cross", "-s", "", fn])
    return mpy

def compareWithExpected (fn, output):
    root = os.path.splitext(fn)[0]
    exp = root + ".exp"
    out = root + ".out"

    if os.path.isfile(exp):
        with open(exp) as f:
            expected = f.read()
        if output == expected:
            try:
                os.remove(out)
            except FileNotFoundError:
                pass
            return True

    with open(out, "w") as f:
        f.write(output)
    printSeparator(fn)
    if os.path.isfile(exp) and expected:
        subprocess.run(f"diff {out} {exp} | head", shell=True)

def printSeparator(fn, e=None):
    root = os.path.splitext(fn)[0]
    if e:
        msg = "FAIL"
    else:
        msg = ""
        try:
            with open(root + ".exp") as f:
                exp = f.readlines()
            nexp = len(exp)
        except:
            nexp = None
        try:
            with open(root + ".out") as f:
                out = f.readlines()
            nout = len(out)
            if nexp is None:
                msg = f"out: {nout}"
            elif nout == nexp:
                msg = f"out & exp: {nexp}"
            else:
                msg = f"out/exp: {nout}/{nexp}"
            if not nexp:
                e = ''.join(out[:10])[:-1]
        except:
            if nexp:
                msg = f"exp: {nexp}"
    sep = (50 - len(fn) - len(msg)) * "-"
    print("---------------------------", fn, sep, msg)
    if e:
        print(e)

if __name__ == "__main__":
    ser = openSerialPort()

    args = sys.argv[1:]
    fail, match = 0, 0

    for fn in args:
        ser.reset_input_buffer()
        ser.write(b'\nbc\nwd 250\n')

        try:
            with open(compileIfOutdated(fn), "rb") as f:
                for line in genHex(f.read()):
                    ser.write(line.encode())
                    ser.flush()
        except:
            print("file?", fn)
            fail += 1
            continue

        results = []
        failed = True
        while True:
            line = ser.readline()
            if len(line) == 0:
                break
            if line == b'\xFF\n':
                continue # yuck: ignore power-up noise from UART TX

            try:
                line = line.decode()
            except UnicodeDecodeError:
                line = "binary: " + line[:-1].hex() + "\n" # oops, not utf8
            if line == "main\n":
                results = []
                failed = False
            if failed:
                print("?", line[:-1])
            results.append(line)
            if line == "done\n":
                break
            if line == "abort\n":
                time.sleep(0.3)
                failed = True
                break
        if failed:
            fail += 1

        if compareWithExpected(fn, ''.join(results)):
            match += 1

    print(f"{len(args)} tests, {match} matches, {fail} failures")
