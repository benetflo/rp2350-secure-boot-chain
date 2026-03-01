import os
import subprocess
import shutil
import platform

FIRMWARE = "build/bootloader/ThesisProject.uf2"

def get_system_name():
    return platform.system()

def find_pico_drive(system_name: str):
    
    result = None

    if system_name == "Windows":
        result = subprocess.check_output(
            "wmic logicaldisk get name, volumename",
            shell=True
        ).decode(errors="ignore")

    elif system_name == "Linux":
        result = subprocess.check_output(
            "lsblk -o LABEL,MOUNTPOINT -nr",
            shell=True
        ).decode(errors="ignore")

    for line in result.splitlines():
        if "RPI-RP2" in line:
            if system_name == "Windows":
                return line.strip().split()[0] + "\\"
            elif system_name == "Linux":
                return line.strip().split()[1] # return mountpoint
    return None

def flash_pico(firmware: str, drive: str, system_name: str):
    
    if not os.path.exists(firmware):
        print("Firmware file not found:", firmware)
        return
        
    try:
        dest = os.path.join(drive, os.path.basename(firmware))
        shutil.copy(firmware, dest)
        print("Firmware was successfully flashed to the Pico!")

    except Exception as e:
        print(f"Error during flashing: {e}")

def run():
    system_name = get_system_name()
    drive_found = find_pico_drive(system_name)

    if drive_found:
        print(f"Pico found at: {drive_found}")
        print("Flash this device? (y/n)")
        
        try:
            while True:
                answ = input()
            
                if answ.strip().lower() == 'y':
                    print("Flashing...")
                    flash_pico(FIRMWARE, drive_found, system_name)
                    print("Flash was successful!")
                    break
                elif answ == 'n':
                    print("Flash cancelled")
                    break
        except KeyboardInterrupt:
            print("\nAborted by user (Ctrl+C).")
    else:
        print("Pico not found. Is it in BOOTSEL mode?")


run()
