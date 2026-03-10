import sys
import struct

def reblock(data):
    blocks = []
    for i in range(0, len(data), 512):
        blocks.append(bytearray(data[i:i+512]))
    return blocks

a = open(sys.argv[1], 'rb').read()
b = open(sys.argv[2], 'rb').read()

blocks = reblock(a) + reblock(b)
total = len(blocks)

out = bytearray()
for i, block in enumerate(blocks):
    # Update block number and total blocks
    struct.pack_into('<I', block, 20, i)
    struct.pack_into('<I', block, 24, total)
    out += block

open(sys.argv[3], 'wb').write(bytes(out))
print(f"Merged {total} blocks into {sys.argv[3]}")