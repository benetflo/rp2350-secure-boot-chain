import struct
import sys
import os

fw = open(sys.argv[1], 'rb').read()
sig = open(sys.argv[2], 'rb').read()
size = struct.pack('<I', len(fw))

outdir = os.path.dirname(sys.argv[3])

open(sys.argv[3], 'wb').write(fw + sig)
open(os.path.join(outdir, 'firmware_size.bin'), 'wb').write(size + b'\x00' * 4)

print(f"Signed firmware: {len(fw)} bytes + 64 bytes signature + 4 bytes size = {len(fw) + 68} bytes total")