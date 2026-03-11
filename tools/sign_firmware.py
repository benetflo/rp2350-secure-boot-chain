import struct
import sys
import hashlib
import subprocess

fw = open(sys.argv[1], 'rb').read()
sig_file = sys.argv[2]
out_file = sys.argv[3]
key_path = sys.argv[4] if len(sys.argv) > 4 else '../firmware_private_key.pem'

# SHA-256 hash av firmware
fw_hash = hashlib.sha256(fw).digest()

# Signera hashen
with open('/tmp/fw_hash.bin', 'wb') as f:
    f.write(fw_hash)

subprocess.run(['openssl', 'pkeyutl', '-sign', '-inkey', 
                key_path, '-rawin', 
                '-in', '/tmp/fw_hash.bin', '-out', sig_file])

sig = open(sig_file, 'rb').read()
open(out_file, 'wb').write(fw + sig)

print(f"Signed firmware: {len(fw)} bytes, hash signed with ED25519")