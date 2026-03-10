import sys
import struct

fw_signed = open(sys.argv[1], 'rb').read()
fw_size_bin = open(sys.argv[2], 'rb').read()

fw_size = struct.unpack('<I', fw_size_bin[:4])[0]
expected_total = fw_size + 64

print(f'Firmware size: {fw_size} bytes')
print(f'firmware_signed.bin size: {len(fw_signed)} bytes')
print(f'Expected: {fw_size} + 64 = {expected_total} bytes')

if expected_total == len(fw_signed):
    print('OK - size is correct')
else:
    print('FAIL - size mismatch')