#!/usr/bin/env python3
# Minimal Replaceable File Storage

import binascii, datetime, os, struct, sys, subprocess
from datetime import datetime

if __name__ == '__main__':
    for fn in sys.argv[1:]:
        if not os.path.isfile(fn):
            raise SystemExit(fn + '?')

        base, ext = os.path.splitext(fn)
        fnOut = base + '.mrfs'

        if ext == '.py':
            subprocess.run(['mpy-cross', fn], check=True)
            ext = '.mpy'
            fn = base + ext

        info = os.stat(fn)
        size = info.st_size
        date = datetime.fromtimestamp(info.st_mtime).strftime('%y%m%d.%H%M')

        with open(fn, 'rb') as fd:
            dat = fd.read()

        if ext == '.mty':
            print(fn, len(dat), 'b')
        else:
            txt = ('%s %s' % (date, fn)).encode() + b'\0'
            vec = struct.pack('4H', 8, 8, 8+len(txt), 8+len(txt)) + txt
            if size > 0:
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

            with open(fnOut, 'wb') as fd:
                fd.write(hdr)   # 8-byte header
                fd.write(vec)   # minimal varyvec + padding
                fd.write(dat)   # payload, i.e. file contents
                fd.write(pad)   # padding + crc32

            # CRC32 of entire file, incl the final CRC, is always 0x2144DF1C
