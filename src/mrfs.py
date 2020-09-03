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

def dumpVaryVec(vv):
    v0 = unpack('H', vv[:2])[0]
    vpos = unpack('%dH' % (v0//2), vv[:v0])
    vnum = v0 // 2 - 1
    #print('vnum',vnum,vpos)

    def vAt(i):
        return vv[vpos[i]:vpos[i+1]]

    hashes = vAt(0)
    print(vnum, 'items')
    print('  0: %d hashes & qstrs, file format 0x%02X' % (len(hashes)-1, hashes[0]))
    for i in range(1,len(hashes)):
        print('%3d: hash 0x%02X, str "%s"' % (i, hashes[i], vAt(i).decode()))
    for i in range(len(hashes), vnum-2):
        bc = vAt(i)
        nbc = len(bc)
        fbc = '0x' + hexlify(bc[9:]).decode()
        if len(fbc) > 6:
            fbc = fbc[:6] + '...'
        print('%3d: bytecode %4db %-10s'
              'of %3d ip %3d st %2d fl %d ex %d np %d nk %d dn %d nc %d'
              % (i, nbc, fbc, *bc[:9]))
    consts = vAt(vnum-2)
    hc = hexlify(consts).decode()
    if len(hc) > 48:
        hc = hc[:48] + '...'
    print('%3d: consts %6db' % (vnum-2, len(consts)), '0x' + hc)
    print('%3d: last          ' % (vnum-1), vAt(vnum-1))

dflag = False
ofile = None
args = iter(sys.argv[1:])
for fn in args:
    if fn == '-d':
        dflag = True # dump mode
        continue
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

    if ext == '.mty' and dflag:
        dumpVaryVec(dat)
    elif ext == '.mrfs':
        while dat[:4] == b'mty0':
            siz = unpack('I', dat[4:8])[0]
            end = unpack('H', dat[14:16])[0]
            txt = '20' + dat[16:8+end-1].decode()
            while end % 8 != 0:
                end += 1
            # file starts at dat[end:]
            byt = siz - end - 8
            while siz % 8 != 4:
                siz += 1
            crc = unpack('I', dat[siz:siz+4])[0]
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
        vec = pack('4H', 8, 8, 8, 8+len(txt)) + txt
        while len(vec) % 8 != 0:
            vec += b'\0'

        hdr = pack('4s I', b'mty0', 8 + len(vec) + size)

        pad = b''
        while (size + len(pad)) % 8 != 4:
            pad += b'\0'
        crc = crc32(hdr)
        crc = crc32(vec, crc)
        crc = crc32(dat, crc)
        crc = crc32(pad, crc)
        pad += pack('I', crc)

        fd = ofile or open(fnOut, 'wb')

        fd.write(hdr)   # 8-byte header
        fd.write(vec)   # minimal varyvec + padding
        fd.write(dat)   # payload, i.e. file contents
        fd.write(pad)   # padding + crc32

        if fd != ofile:
            close(fd)

        # CRC32 of entire file, incl the final CRC, is always 0x2144DF1C
