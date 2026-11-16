#!/usr/bin/env python3
"""
Comprehensive E2E & Unit Test Suite for ESP32-S3 WPS Password Fetcher Firmware
Testing Requirements R1, R2, R3, R4 across Tiers 1-4.
"""

import os
import sys
import re
import unittest
from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parent.parent

class TestFirmwareConfiguration(unittest.TestCase):
    """Tier 1: Configuration & Layout Validation"""

    def test_platformio_ini_exists_and_configured(self):
        ini_path = PROJECT_ROOT / "platformio.ini"
        self.assertTrue(ini_path.exists(), "platformio.ini must exist in project root")
        content = ini_path.read_text(encoding="utf-8")
        self.assertIn("esp32-s3-devkitc1-n16r8", content, "Board must target esp32-s3-devkitc1-n16r8")
        self.assertIn("framework = arduino", content, "Framework must be arduino")
        self.assertIn("src_dir = src", content, "src_dir must be configured")
        self.assertIn("include_dir = include", content, "include_dir must be configured")

    def test_gitignore_covers_local_memory_and_builds(self):
        gitignore_path = PROJECT_ROOT / ".gitignore"
        self.assertTrue(gitignore_path.exists(), ".gitignore must exist")
        content = gitignore_path.read_text(encoding="utf-8")
        self.assertIn("memory.md", content, ".gitignore must exclude memory.md")
        self.assertIn("builds/", content, ".gitignore must exclude builds/")


class TestWpsImplementation(unittest.TestCase):
    """Tier 1 & 2: R1 - WPS PBC Listening & Credential Capture"""

    def setUp(self):
        self.main_cpp = (PROJECT_ROOT / "src" / "main.cpp").read_text(encoding="utf-8")
        self.wps_ino = (PROJECT_ROOT / "wps.ino").read_text(encoding="utf-8")

    def test_wps_pbc_api_initialization(self):
        for code in [self.main_cpp, self.wps_ino]:
            self.assertIn("esp_wps.h", code, "Must include esp_wps.h")
            self.assertIn("esp_wifi.h", code, "Must include esp_wifi.h")
            self.assertIn("WPS_CONFIG_INIT_DEFAULT(WPS_TYPE_PBC)", code, "Must configure WPS PBC")
            self.assertIn("esp_wifi_wps_enable", code, "Must call esp_wifi_wps_enable")
            self.assertIn("esp_wifi_wps_start", code, "Must call esp_wifi_wps_start")

    def test_genuine_credential_extraction_without_hardcoded_dummy(self):
        for code in [self.main_cpp, self.wps_ino]:
            # Must extract from esp_wifi_get_config on success
            self.assertIn("ARDUINO_EVENT_WPS_ER_SUCCESS", code)
            self.assertIn("esp_wifi_get_config(WIFI_IF_STA, &conf)", code)
            self.assertIn("conf.sta.ssid", code)
            self.assertIn("conf.sta.password", code)
            # Integrity check: No hardcoded fake credentials
            self.assertNotIn('strcpy(credentials.ssid, "MyHomeWifi")', code)
            self.assertNotIn('strcpy(credentials.password, "SecretPassword123")', code)

    def test_wps_error_handling_and_recovery(self):
        for code in [self.main_cpp, self.wps_ino]:
            self.assertIn("ARDUINO_EVENT_WPS_ER_FAILED", code)
            self.assertIn("ARDUINO_EVENT_WPS_ER_TIMEOUT", code)
            self.assertIn("ARDUINO_EVENT_WPS_ER_PBC_OVERLAP", code)
            self.assertIn("esp_wifi_wps_disable()", code)


class TestStandaloneConstraint(unittest.TestCase):
    """Integrity: User Constraint - Standalone ESP32-S3 (NO SD card or external modules)"""

    def test_no_sd_card_or_spi_storage_dependencies(self):
        for path in [PROJECT_ROOT / "src" / "main.cpp", PROJECT_ROOT / "wps.ino"]:
            content = path.read_text(encoding="utf-8")
            self.assertNotIn("#include <SD.h>", content, "SD.h must NOT be included (standalone constraint)")
            self.assertNotIn('#include "SD.h"', content, "SD.h must NOT be included (standalone constraint)")
            self.assertNotIn("SD_CS_PIN", content, "SD_CS_PIN must NOT exist")
            self.assertNotIn("fs::FS", content, "External filesystem must NOT be used")


class TestHostModeAndWebPortal(unittest.TestCase):
    """Tier 1, 2, 3: R2 - SoftAP & Offline Captive Web Portal"""

    def setUp(self):
        self.main_cpp = (PROJECT_ROOT / "src" / "main.cpp").read_text(encoding="utf-8")
        self.portal_html = (PROJECT_ROOT / "include" / "portal_html.h").read_text(encoding="utf-8")

    def test_softap_parameters(self):
        self.assertIn("ESP32-WPS-Portal", self.main_cpp, "SoftAP SSID must be ESP32-WPS-Portal")
        self.assertIn("192, 168, 4, 1", self.main_cpp, "SoftAP IP must be 192.168.4.1")
        self.assertIn("dnsServer.start(DNS_PORT, \"*\", AP_IP)", self.main_cpp, "Captive DNS must direct * to AP_IP")

    def test_web_routes_and_json_endpoint(self):
        self.assertIn('server.on("/", HTTP_GET', self.main_cpp, "Root portal route required")
        self.assertIn('server.on("/api/credentials", HTTP_GET', self.main_cpp, "REST API route required")
        self.assertIn('server.on("/generate_204"', self.main_cpp, "Android captive portal probe required")
        self.assertIn('server.on("/hotspot-detect.html"', self.main_cpp, "Apple captive portal probe required")
        self.assertIn('server.onNotFound(', self.main_cpp, "Catch-all 302 redirect required")

    def test_portal_html_offline_self_containment(self):
        # Strict offline check: No external http(s) links in CSS, JS, or images
        external_urls = re.findall(r'(?:href|src)=["\']https?://[^"\']+["\']', self.portal_html)
        self.assertEqual(len(external_urls), 0, f"Portal HTML must have ZERO external CDN links, found: {external_urls}")

    def test_portal_ui_features(self):
        self.assertIn("WPS Credential Portal", self.portal_html)
        self.assertIn("%SSID%", self.portal_html)
        self.assertIn("%PASSWORD%", self.portal_html)
        self.assertIn("togglePassword()", self.portal_html, "Must have show/hide toggle function")
        self.assertIn("copyText(", self.portal_html, "Must have copy to clipboard function")
        self.assertIn("qr-canvas", self.portal_html, "Must have QR code canvas element")
        self.assertIn("WIFI:T:WPA;S:", self.portal_html, "Must generate standard Wi-Fi QR string")


class TestStatusLedIndication(unittest.TestCase):
    """Tier 1: R3 - WS2812 RGB LED Signaling"""

    def setUp(self):
        self.main_cpp = (PROJECT_ROOT / "src" / "main.cpp").read_text(encoding="utf-8")

    def test_ws2812_gpio48_pin_and_driver(self):
        self.assertIn("RGB_LED_PIN 48", self.main_cpp, "WS2812 RGB LED must be mapped to GPIO 48")
        self.assertTrue(
            "rgbLedWrite(RGB_LED_PIN" in self.main_cpp or "neopixelWrite(RGB_LED_PIN" in self.main_cpp,
            "Must use native hardware RMT driver"
        )

    def test_non_blocking_led_states(self):
        self.assertIn("LED_WPS_PULSE", self.main_cpp)
        self.assertIn("LED_DOUBLE_FLASH_GREEN", self.main_cpp)
        self.assertIn("LED_SOLID_GREEN", self.main_cpp)
        self.assertIn("LED_FLASH_RED", self.main_cpp)
        self.assertIn("millis()", self.main_cpp, "Must use non-blocking millis() timer")


class TestFlashingScriptAndArchival(unittest.TestCase):
    """Tier 1 & 4: R4 - Auto-Detection & Build Archival"""

    def test_scripts_exist_and_executable(self):
        ps1 = PROJECT_ROOT / "scripts" / "detect_and_flash.ps1"
        py = PROJECT_ROOT / "scripts" / "detect_and_flash.py"
        self.assertTrue(ps1.exists(), "scripts/detect_and_flash.ps1 must exist")
        self.assertTrue(py.exists(), "scripts/detect_and_flash.py must exist")

    def test_ps1_contains_com_detection_and_archival(self):
        content = (PROJECT_ROOT / "scripts" / "detect_and_flash.ps1").read_text(encoding="utf-8")
        self.assertIn("Win32_PnPEntity", content)
        self.assertIn("pio run", content)
        self.assertIn("upload-port", content)
        self.assertIn("builds\\firmware", content)


if __name__ == "__main__":
    unittest.main()
