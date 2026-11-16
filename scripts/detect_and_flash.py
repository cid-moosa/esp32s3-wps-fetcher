#!/usr/bin/env python3
"""
Automated ESP32-S3 Serial Port Auto-Detection, PlatformIO Build, and Flasher.
"""

import sys
import os
import subprocess
import shutil
import datetime
from pathlib import Path

try:
    import serial.tools.list_ports
    HAS_PYSERIAL = True
except ImportError:
    HAS_PYSERIAL = False

PROJECT_ROOT = Path(__file__).resolve().parent.parent

def detect_port():
    print("[1/5] Auto-detecting connected ESP32-S3 serial COM port...")
    if not HAS_PYSERIAL:
        # Fallback to COM4 if pyserial not imported
        return "COM4"
    
    ports = list(serial.tools.list_ports.comports())
    if not ports:
        print("[!] No COM ports detected!")
        return None
    
    # Priority matching
    for p in ports:
        desc = (p.description or "").lower()
        hwid = (p.hwid or "").lower()
        if any(k in desc or k in hwid for k in ["ch343", "esp32", "cp210", "1a86:55d3", "usb-enhanced-serial"]):
            print(f"      Found matching device: {p.device} ({p.description})")
            return p.device
            
    print(f"      Defaulting to first detected port: {ports[0].device} ({ports[0].description})")
    return ports[0].device

def build_firmware():
    print("[2/5] Compiling firmware via PlatformIO (pio run)...")
    res = subprocess.run(["pio", "run"], cwd=PROJECT_ROOT)
    if res.returncode != 0:
        print("[!] PlatformIO compilation failed!")
        sys.exit(res.returncode)
    print("      Compilation successful.")

def flash_firmware(port):
    print(f"[3/5] Flashing firmware to {port} via PlatformIO...")
    res = subprocess.run(["pio", "run", "--target", "upload", "--upload-port", port], cwd=PROJECT_ROOT)
    if res.returncode != 0:
        print(f"[!] PlatformIO flash upload failed on port {port}!")
        sys.exit(res.returncode)
    print("      Flashing complete.")

def archive_binary():
    print("[4/5] Archiving firmware binary artifact...")
    firmware_bin = PROJECT_ROOT / ".pio" / "build" / "esp32s3" / "firmware.bin"
    builds_dir = PROJECT_ROOT / "builds" / "firmware"
    builds_dir.mkdir(parents=True, exist_ok=True)
    
    if firmware_bin.exists():
        timestamp = datetime.datetime.now().strftime("%Y%m%d-%H%M%S")
        dest = builds_dir / f"wps-esp32s3-{timestamp}.bin"
        shutil.copy2(firmware_bin, dest)
        size_kb = dest.stat().st_size / 1024.0
        print(f"      Archived: {dest} ({size_kb:.1f} KB)")
        return dest
    else:
        print(f"[!] Binary not found at {firmware_bin}")
        return None

def main():
    print("=" * 60)
    print("   ESP32-S3 WPS Password Fetcher - Auto Flash & Build")
    print(f"   Project: {PROJECT_ROOT}")
    print("=" * 60)
    
    port = sys.argv[1] if len(sys.argv) > 1 else detect_port()
    if not port:
        print("Error: Could not locate connected ESP32-S3 port.")
        sys.exit(1)
        
    build_firmware()
    flash_firmware(port)
    archive_binary()
    print("=" * 60)
    print("   All operations completed successfully!")
    print("=" * 60)

if __name__ == "__main__":
    main()
