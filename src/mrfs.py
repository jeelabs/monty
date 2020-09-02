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

import binascii, os, struct, sys, subprocess
from datetime import datetime

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
    date = datetime.fromtimestamp(info.st_mtime).strftime('%y%m%d@%H%M')

    if ext == '.py':
        subprocess.run(['mpy-cross', fn], check=True)
        ext = '.mpy'
        fn = base + ext

    info = os.stat(fn)
    size = info.st_size

    with open(fn, 'rb') as fd:
        dat = fd.read()

    if ext == '.mrfs':
        while dat[:4] == b'mty0':
            siz = struct.unpack('I', dat[4:8])[0]
            end = struct.unpack('H', dat[14:16])[0]
            txt = '20' + dat[16:8+end-1].decode()
            while end % 8 != 0:
                end += 1
            # file starts at dat[end:]
            byt = siz - end - 8
            while siz % 8 != 4:
                siz += 1
            crc = struct.unpack('I', dat[siz:siz+4])[0]
            dat = dat[siz+4:].lstrip(b'\xFF')
            print('%08X %8d %s' % (crc, byt, txt))
        if len(dat) > 0:
            raise SystemExit('%s: bad header at offset %d' %
                                (fn, size - len(dat)))
    else:
        nam = fn
        if ext in ['.mpy','.mty']: # TODO just .mty ...
            nam = base
            if '.' not in nam:
                nam = nam.replace('/', '.')
        txt = ('%s %s' % (date, nam)).encode() + b'\0'
        vec = struct.pack('4H', 8, 8, 8, 8+len(txt)) + txt
        while len(vec) % 8 != 0:
            vec += b'\0'

        hdr = struct.pack('4s I', b'mty0', 8 + len(vec) + size)

        pad = b''
        while (size + len(pad)) % 8 != 4:
            pad += b'\0'
        crc = binascii.crc32(hdr)
        crc = binascii.crc32(vec, crc)
        crc = binascii.crc32(dat, crc)
        crc = binascii.crc32(pad, crc)
        pad += struct.pack('I', crc)

        fd = ofile or open(fnOut, 'wb')

        fd.write(hdr)   # 8-byte header
        fd.write(vec)   # minimal varyvec + padding
        fd.write(dat)   # payload, i.e. file contents
        fd.write(pad)   # padding + crc32

        if fd != ofile:
            close(fd)

        # CRC32 of entire file, incl the final CRC, is always 0x2144DF1C
