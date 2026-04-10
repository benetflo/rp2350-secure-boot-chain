import struct
import sys
import os

FW_MAGIC = 0xB00710AD
FLASH_PAGE_SIZE = 256  # header max size

fw_path = sys.argv[1]
out_path = sys.argv[2]
version = int(sys.argv[3]) if len(sys.argv) > 3 else 1

fw_size = os.path.getsize(fw_path)

# Build header: magic, version, size
header = struct.pack("<III", FW_MAGIC, fw_size, version)

# Padding to whole page
header = header.ljust(FLASH_PAGE_SIZE, b"\xFF")

with open(out_path, "wb") as f:
    f.write(header)

print(f"Header written: magic=0x{FW_MAGIC:X}, version={version}, size={fw_size}")
