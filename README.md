# ESP32-S3 WPS Password Fetcher & Captive Portal

[![PlatformIO](https://img.shields.io/badge/PlatformIO-Framework%20Arduino-orange.svg)](https://platformio.org/)
[![Hardware](https://img.shields.io/badge/Hardware-ESP32--S3%20(N16R8)-blue.svg)](https://www.espressif.com/en/products/socs/esp32-s3)
[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
[![Status LED](https://img.shields.io/badge/WS2812%20RGB-GPIO%2048-purple.svg)]()

> A 100% standalone Wi-Fi credential harvester firmware for the ESP32-S3. Captures router credentials via WPS Push Button Configuration (PBC), launches an open SoftAP captive portal with a dark-mode dashboard and dynamic offline QR code, and signals operating phases on the onboard WS2812 RGB LED.

---

## ✨ Features

- **WPS PBC Credential Extraction**: Automatically pairs with routers when the WPS button is pressed using the native ESP-IDF WPS Enrollee engine. Extracts SSID and WPA2-PSK directly from the Wi-Fi driver buffers via `esp_wifi_get_config`.
- **Standalone Architecture**: 100% standalone firmware utilizing only onboard ESP32-S3 resources (Internal Flash, SRAM/PSRAM, and onboard WS2812 LED). Zero external SD card modules or peripherals required.
- **Captive Web Portal**: Automatically hosts an open SoftAP (`ESP32-WPS-Portal` at `192.168.4.1`) with a port 53 DNS wildcard server that triggers native OS captive portal dialogs on iOS, Android, and Windows.
- **Offline Dark-Mode Dashboard**: Sleek glassmorphic web portal stored in Flash (`PROGMEM`) featuring:
  - Network SSID with one-click clipboard copy.
  - Bullet-masked password with instant reveal toggle (`Show`/`Hide`) and copy button.
  - **Offline Wi-Fi QR Code**: Pure client-side vanilla JavaScript QR engine (Kazuhiko Arase QR Model) generating camera-scannable `WIFI:T:WPA;S:...;P:...;;` QR codes without requiring external CDN access.
- **JSON REST API**: Exposes `GET /api/credentials` returning structured JSON for automation and headless integration.
- **WS2812 RGB LED Signaling**: Non-blocking `millis()` state machine on GPIO 48:
  - 🟡/🔵 **Breathing Pulse (Yellow/Blue)**: Active WPS PBC listening mode.
  - 🟢 **Double Green Flash**: Handshake successful, credentials captured.
  - 🟢 **Solid Emerald Green**: SoftAP & Web Portal active (capped brightness).
  - 🔴 **Flashing Red**: WPS timeout, overlap, or pairing failure with auto-retry.
- **Automated Tooling**: Includes automated COM port auto-detection, PlatformIO compilation, and flashing scripts (`scripts/detect_and_flash.ps1` & `scripts/detect_and_flash.py`).

---

## 🛠️ Hardware Specification

| Parameter | Specification |
|---|---|
| **Microcontroller** | Espressif ESP32-S3 (QFN56 revision v0.2) |
| **Processor** | Dual-Core 32-bit Xtensa LX7 @ 240 MHz |
| **Flash Memory** | 16 MB Quad SPI Flash |
| **PSRAM** | 8 MB Octal SPI PSRAM |
| **Onboard RGB LED** | WS2812 Addressable RGB LED on **GPIO 48** |
| **USB Interface** | USB-to-UART Bridge (CH343 / UART0) on `COM4` |
| **Radio** | 2.4 GHz 802.11 b/g/n Wi-Fi |

---

## 🚀 Quickstart

### Prerequisites
- [PlatformIO Core](https://platformio.org/) or [Arduino IDE 2.x](https://www.arduino.cc/en/software)
- Python 3.x + `esptool`

### Option 1: Automated Detection & Flash (PowerShell)
Connect your ESP32-S3 via USB and run:
```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\detect_and_flash.ps1
```
The script will:
1. Auto-detect the connected serial COM port.
2. Interrogate the chip features via `esptool`.
3. Compile the firmware via PlatformIO.
4. Flash the binary to the device.
5. Archive release artifacts to `builds/firmware/`.

### Option 2: PlatformIO CLI
```bash
# Compile firmware
pio run

# Flash to device (auto-detects COM port)
pio run --target upload

# Open serial monitor
pio device monitor --baud 115200
```

---

## 📡 REST API

When the portal is active at `192.168.4.1`, connect to `ESP32-WPS-Portal` and query:

### `GET /api/credentials`
```json
{
  "status": "ok",
  "captured": true,
  "ssid": "MyHomeNetwork",
  "password": "CorrectHorseBatteryStaple",
  "security": "WPA2-PSK",
  "gateway": "192.168.4.1",
  "uptime_seconds": 42
}
```

---

## 🧪 Testing

The repository includes a comprehensive test suite verifying the firmware architecture, absence of dummy/hardcoded passwords, offline portal self-containment, and driver contracts:

```bash
python tests/test_firmware.py
```

---

## 👤 Author

Developed by **[cid-moosa](https://github.com/cid-moosa)**.

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
