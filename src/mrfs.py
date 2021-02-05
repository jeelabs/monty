#!/usr/bin/env python3

# Minimal Replaceable File Storage
#
# Usage: mrfs.py [-o outfile] infile...
#
#   if -o is specified all files are combined to that single output
#   else each "abc.def" file is wrapped up and written to "abc.mrfs"
#
#   files with a ".py" extensions are compiled to ".mpy" and then wrapped
#   (the ".mpy" extension is always omitted from names stored internally)
#
#   input files with extension ".mrfs" are listed instead of wrapped

import os, sys, subprocess
from binascii import crc32, hexlify
from datetime import datetime
from struct import pack, unpack

ofile = None
args = iter(sys.argv[1:])
for fn in args:
    if fn == '-o':
        ofile = open(next(args), 'wb')
        continue

    if not os.path.isfile(fn):
        raise SystemExit(fn + '?')

    base, ext = os.path.splitext(fn)
    fnOut = base + '.mrfs'

    info = os.stat(fn)
    date = int(datetime.fromtimestamp(info.st_mtime).strftime('%y%m%d%H%M'))

    if ext == '.py':
        subprocess.run(['mpy-cross', '-s', '', fn], check=True)
        ext = '.mpy'
        fn = base + ext

    info = os.stat(fn)
    size = info.st_size

    with open(fn, 'rb') as fd:
        dat = fd.read()
        assert len(dat) == size

    if ext == '.mrfs':
        files = {}
        while dat[:4] == b'mty0':
            siz = unpack('I', dat[4:8])[0]
            end = 8 + siz + (-siz&31)
            nam = dat[end:end+16].split(b'\0', 1)[0].decode()
            tim, crc = unpack('II', dat[end+16:end+24])
            dat = dat[end+24:]
            # only keep last version of each file
            files[nam] = (crc, siz, tim//10000, tim%10000, nam)
        for k in files:
            v = files[k]
            if v[2] != 0: # skip deleted files
                print('[%08X] %5d  %6d.%04d  %s' % v)
        if len(dat) > 0 and dat[:4] != b'\xff\xff\xff\xff':
            raise SystemExit('%s: bad header at offset %d' %
                                (fn, size - len(dat)))
    else:
        nam = fn
        if ext in ['.mpy','.mty']: # TODO just .mty ...
            nam = base

        hdr = pack('4s I', b'mty0', size)
        pad = (-size&31) * b'\0'
        ftr = pack('16s I', os.path.basename(nam)[:15].encode(), date)

        crc = crc32(hdr)
        crc = crc32(dat, crc)
        crc = crc32(pad, crc)
        crc = crc32(ftr, crc)
        ftr += pack('I', crc)

        fd = ofile or open(fnOut, 'wb')

        fd.write(hdr)   # 8-byte header
        fd.write(dat)   # payload, i.e. file contents
        fd.write(pad)   # padding to multiple of 32
        fd.write(ftr)   # 24-byte footer

        if fd != ofile:
            close(fd)

        # CRC32 of each file entry, incl. its final CRC, is always 0x2144DF1C
