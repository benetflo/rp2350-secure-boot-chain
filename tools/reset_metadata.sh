#!/bin/bash
python3 -c "open('/tmp/meta_reset.bin','wb').write(b'\xff'*256)"
sudo picotool load -o 0x103C0000 /tmp/meta_reset.bin
echo "Metadata reset!"

# this script can be used to reset metadata if you run into problems when reflashing everything from the beginning