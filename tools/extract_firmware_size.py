import struct
import sys

size = len(open(sys.argv[1], 'rb').read())
version = int(sys.argv[3]) if len(sys.argv) > 3 else 1

with open(sys.argv[2], 'wb') as f:
    f.write(struct.pack('<I', size))
    f.write(struct.pack('<I', version))

print(f"Firmware size: {size} bytes, version: {version}")