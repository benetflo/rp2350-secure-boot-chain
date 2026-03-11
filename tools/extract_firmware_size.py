import struct
import sys

size = len(open(sys.argv[1], 'rb').read())
with open(sys.argv[2], 'wb') as f:
    f.write(struct.pack('<I', size) + b'\x00' * 4)
print(f"Firmware size: {size} bytes")