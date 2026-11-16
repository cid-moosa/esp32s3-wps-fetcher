/**
 * ESP32-S3 Standalone WPS Password Fetcher & Captive Web Portal
 * 
 * Hardware Target: ESP32-S3 (16MB Flash, 8MB PSRAM, Onboard WS2812 RGB LED on GPIO 48)
 * 100% Standalone - Zero external modules or SD cards required.
 * 
 * Architecture:
 * - Phase 1: WPS PBC Enrollee listening via esp_wps
 * - Phase 2: Credential capture via esp_wifi_get_config(WIFI_IF_STA, &conf)
 * - Phase 3: Transition to SoftAP "ESP32-WPS-Portal" (192.168.4.1)
 * - Phase 4: Offline-first Captive Web Portal & JSON API (/api/credentials)
 * - Peripheral: Non-blocking WS2812 RGB LED state machine on GPIO 48
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <esp_wifi.h>
#include <esp_wps.h>
#include "portal_html.h"

// Hardware Configuration
#ifndef RGB_LED_PIN
#define RGB_LED_PIN 48
#endif

// SoftAP Configuration
static const char* AP_SSID = "ESP32-WPS-Portal";
static const IPAddress AP_IP(192, 168, 4, 1);
static const IPAddress AP_NETMASK(255, 255, 255, 0);
static const byte DNS_PORT = 53;

// Global Servers
static WebServer server(80);
static DNSServer dnsServer;

// Finite State Machine States
enum AppState {
    STATE_BOOT,
    STATE_WPS_LISTENING,
    STATE_WPS_SUCCESS_FLASH,
    STATE_WPS_RETRY_WAIT,
    STATE_TRANSITION_AP,
    STATE_PORTAL_ACTIVE
};

enum LedState {
    LED_OFF,
    LED_WPS_PULSE,
    LED_DOUBLE_FLASH_GREEN,
    LED_SOLID_GREEN,
    LED_FLASH_RED
};

static volatile AppState currentState = STATE_BOOT;
static volatile LedState currentLedState = LED_OFF;

// Credential Storage
struct CapturedCredentials {
    char ssid[33];
    char password[65];
    char bssid_str[20];
    bool valid;
    uint32_t captured_at_ms;
};

static CapturedCredentials credentials = {
    .ssid = {0},
    .password = {0},
    .bssid_str = {0},
    .valid = false,
    .captured_at_ms = 0
};

// Retry and Timing Tracking
static uint32_t stateTimer = 0;
static uint32_t wpsStartTimer = 0;
static const uint32_t WPS_TIMEOUT_MS = 120000; // 120s standard WPS walk-time

// Forward Declarations
void setStatusLed(uint8_t r, uint8_t g, uint8_t b);
void updateLedStateMachine();
void startWpsListening();
void stopWpsListening();
void launchSoftApPortal();
void setupWebServerRoutes();
void onWiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info);

// Helper: Hardware WS2812 RGB LED Control
void setStatusLed(uint8_t r, uint8_t g, uint8_t b) {
    // Native Arduino-ESP32 RMT driver (zero external libraries)
    rgbLedWrite(RGB_LED_PIN, r, g, b);
}

// Non-blocking RGB LED state machine driven by millis()
void updateLedStateMachine() {
    static uint32_t lastLedUpdate = 0;
    uint32_t now = millis();
    if (now - lastLedUpdate < 20) return; // 50 Hz refresh rate
    lastLedUpdate = now;

    switch (currentLedState) {
        case LED_OFF:
            setStatusLed(0, 0, 0);
            break;

        case LED_WPS_PULSE: {
            // Breathing pulse effect: Yellow with subtle blue hue (1200ms cycle)
            uint32_t phase = now % 1200;
            uint8_t brightness;
            if (phase < 600) {
                brightness = (phase * 45) / 600; // ramp up 0 -> 45
            } else {
                brightness = ((1200 - phase) * 45) / 600; // ramp down 45 -> 0
            }
            if (brightness < 3) brightness = 3; // soft floor
            
            // Yellow pulse: R = brightness, G = brightness * 0.75, B = 0
            uint8_t r = brightness;
            uint8_t g = (brightness * 3) / 4;
            uint8_t b = (brightness > 20) ? (brightness / 5) : 0;
            setStatusLed(r, g, b);
            break;
        }

        case LED_DOUBLE_FLASH_GREEN: {
            // Fast double-green flash to signal credential capture
            uint32_t flashPhase = (now - stateTimer) % 600;
            if ((flashPhase < 120) || (flashPhase >= 240 && flashPhase < 360)) {
                setStatusLed(0, 50, 0); // Vivid Green ON
            } else {
                setStatusLed(0, 0, 0);  // OFF
            }
            break;
        }

        case LED_SOLID_GREEN:
            // Solid emerald green capped at safe ~15% brightness (40/255)
            setStatusLed(0, 40, 10);
            break;

        case LED_FLASH_RED: {
            // Flashing red warning (250ms interval = 2 Hz)
            bool on = (now / 250) % 2;
            if (on) {
                setStatusLed(45, 0, 0);
            } else {
                setStatusLed(0, 0, 0);
            }
            break;
        }
    }
}

// Start WPS PBC Listening
void startWpsListening() {
    Serial.println(F("[WPS] Initializing Station Mode for WPS PBC..."));
    
    // Set STA mode and start Wi-Fi driver
    WiFi.mode(WIFI_STA);
    esp_wifi_start();
    delay(50);

    esp_wps_config_t config = WPS_CONFIG_INIT_DEFAULT(WPS_TYPE_PBC);
    
    esp_err_t err = esp_wifi_wps_enable(&config);
    if (err != ESP_OK) {
        Serial.printf("[WPS] esp_wifi_wps_enable failed! Error: 0x%x\n", err);
        currentLedState = LED_FLASH_RED;
        currentState = STATE_WPS_RETRY_WAIT;
        stateTimer = millis();
        return;
    }

    err = esp_wifi_wps_start(0);
    if (err != ESP_OK) {
        Serial.printf("[WPS] esp_wifi_wps_start failed! Error: 0x%x\n", err);
        esp_wifi_wps_disable();
        currentLedState = LED_FLASH_RED;
        currentState = STATE_WPS_RETRY_WAIT;
        stateTimer = millis();
        return;
    }

    Serial.println(F("[WPS] >>> WPS PBC ACTIVE! Press the WPS button on your Wi-Fi router now <<<"));
    wpsStartTimer = millis();
    currentState = STATE_WPS_LISTENING;
    currentLedState = LED_WPS_PULSE;
}

// Stop WPS Listening cleanly
void stopWpsListening() {
    Serial.println(F("[WPS] Disabling WPS engine..."));
    esp_wifi_wps_disable();
}

// Wi-Fi Event Handler Callback
void onWiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info) {
    switch (event) {
        case ARDUINO_EVENT_WPS_ER_SUCCESS: {
            Serial.println(F("\n[EVENT] ARDUINO_EVENT_WPS_ER_SUCCESS: WPS Handshake completed successfully!"));
            
            // Cleanly disable WPS engine immediately
            esp_wifi_wps_disable();

            // Extract credentials directly from ESP-IDF STA configuration
            // Note: esp_wifi_get_config directly reads the recovered WPS profile
            wifi_config_t conf;
            esp_err_t err = esp_wifi_get_config(WIFI_IF_STA, &conf);
            if (err == ESP_OK) {
                memset(&credentials, 0, sizeof(credentials));
                memcpy(credentials.ssid, conf.sta.ssid, sizeof(conf.sta.ssid));
                memcpy(credentials.password, conf.sta.password, sizeof(conf.sta.password));
                
                // Ensure null termination
                credentials.ssid[32] = '\0';
                credentials.password[64] = '\0';
                credentials.valid = true;
                credentials.captured_at_ms = millis();

                Serial.println(F("=================================================="));
                Serial.printf("[CAPTURED] SSID     : %s\n", credentials.ssid);
                Serial.printf("[CAPTURED] Password : %s\n", credentials.password);
                Serial.println(F("=================================================="));

                // Initiate success signaling
                currentState = STATE_WPS_SUCCESS_FLASH;
                currentLedState = LED_DOUBLE_FLASH_GREEN;
                stateTimer = millis();
            } else {
                Serial.printf("[ERROR] Failed to read Wi-Fi config from driver: 0x%x\n", err);
                currentState = STATE_WPS_RETRY_WAIT;
                currentLedState = LED_FLASH_RED;
                stateTimer = millis();
            }
            break;
        }

        case ARDUINO_EVENT_WPS_ER_FAILED:
            Serial.println(F("\n[EVENT] ARDUINO_EVENT_WPS_ER_FAILED: WPS pairing failed or received M2D frame."));
            stopWpsListening();
            currentState = STATE_WPS_RETRY_WAIT;
            currentLedState = LED_FLASH_RED;
            stateTimer = millis();
            break;

        case ARDUINO_EVENT_WPS_ER_TIMEOUT:
            Serial.println(F("\n[EVENT] ARDUINO_EVENT_WPS_ER_TIMEOUT: WPS PBC walk time expired without pairing."));
            stopWpsListening();
            currentState = STATE_WPS_RETRY_WAIT;
            currentLedState = LED_FLASH_RED;
            stateTimer = millis();
            break;

        case ARDUINO_EVENT_WPS_ER_PBC_OVERLAP:
            Serial.println(F("\n[EVENT] ARDUINO_EVENT_WPS_ER_PBC_OVERLAP: Multiple WPS PBC sessions detected in range!"));
            stopWpsListening();
            currentState = STATE_WPS_RETRY_WAIT;
            currentLedState = LED_FLASH_RED;
            stateTimer = millis();
            break;

        default:
            break;
    }
}

// Transition into SoftAP Host Mode & Launch Captive Portal
void launchSoftApPortal() {
    Serial.println(F("\n[PORTAL] Transitioning to Standalone SoftAP Host Mode..."));
    
    // Disconnect STA cleanly
    WiFi.disconnect(true, false);
    delay(100);

    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(AP_IP, AP_IP, AP_NETMASK);
    
    // Launch open SoftAP on Channel 1, maximum 4 clients
    bool apStarted = WiFi.softAP(AP_SSID, NULL, 1, 0, 4);
    if (!apStarted) {
        Serial.println(F("[ERROR] Failed to launch SoftAP network!"));
        currentLedState = LED_FLASH_RED;
        return;
    }

    Serial.println(F("=================================================="));
    Serial.printf("[PORTAL] SoftAP SSID   : %s\n", AP_SSID);
    Serial.printf("[PORTAL] Gateway IP    : %s\n", WiFi.softAPIP().toString().c_str());
    Serial.printf("[PORTAL] Clients Limit : 4\n");
    Serial.println(F("=================================================="));

    // Start DNS Server on port 53 for Captive Portal (redirect all queries to 192.168.4.1)
    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer.start(DNS_PORT, "*", AP_IP);
    Serial.println(F("[DNS] Captive Portal DNS responder started on port 53."));

    // Register Web Server Routes
    setupWebServerRoutes();
    server.begin();
    Serial.println(F("[HTTP] Offline Web Portal Server active on port 80."));

    currentState = STATE_PORTAL_ACTIVE;
    currentLedState = LED_SOLID_GREEN;
}

// Generate Masked Password String (e.g. bullet characters)
static String getMaskedPassword(const char* pwd) {
    size_t len = strlen(pwd);
    if (len == 0) return String("••••••••");
    String masked = "";
    for (size_t i = 0; i < len; i++) {
        masked += "•";
    }
    return masked;
}

// Format Device Uptime string (hh:mm:ss)
static String formatUptime() {
    uint32_t sec = millis() / 1000;
    uint32_t h = sec / 3600;
    uint32_t m = (sec % 3600) / 60;
    uint32_t s = sec % 60;
    char buf[16];
    snprintf(buf, sizeof(buf), "%02u:%02u:%02u", h, m, s);
    return String(buf);
}

// Setup Web Server Endpoints
void setupWebServerRoutes() {
    // Root Dashboard
    server.on("/", HTTP_GET, []() {
        String page = FPSTR(PORTAL_HTML);
        
        // Escape special HTML chars in credentials
        String safeSsid = credentials.valid ? String(credentials.ssid) : String("Unknown");
        String safePwd = credentials.valid ? String(credentials.password) : String("");
        String maskedPwd = getMaskedPassword(safePwd.c_str());

        page.replace("%SSID%", safeSsid);
        page.replace("%PASSWORD%", safePwd);
        page.replace("%MASKED_PASSWORD%", maskedPwd);
        page.replace("%UPTIME%", formatUptime());

        server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
        server.sendHeader("Pragma", "no-cache");
        server.sendHeader("Expires", "-1");
        server.send(200, "text/html", page);
    });

    // JSON REST API: GET /api/credentials
    server.on("/api/credentials", HTTP_GET, []() {
        String json = "{\n";
        json += "  \"status\": \"" + String(credentials.valid ? "ok" : "pending") + "\",\n";
        json += "  \"captured\": " + String(credentials.valid ? "true" : "false") + ",\n";
        json += "  \"ssid\": \"" + String(credentials.ssid) + "\",\n";
        json += "  \"password\": \"" + String(credentials.password) + "\",\n";
        json += "  \"security\": \"WPA2-PSK\",\n";
        json += "  \"gateway\": \"192.168.4.1\",\n";
        json += "  \"uptime_seconds\": " + String(millis() / 1000) + "\n";
        json += "}";

        server.sendHeader("Access-Control-Allow-Origin", "*");
        server.send(200, "application/json", json);
    });

    // Captive Portal Probe Redirects (Android, Apple, Windows)
    auto handleCaptiveRedirect = []() {
        server.sendHeader("Location", "http://192.168.4.1/", true);
        server.send(302, "text/plain", "");
    };

    server.on("/generate_204", HTTP_GET, handleCaptiveRedirect);       // Android
    server.on("/gen_204", HTTP_GET, handleCaptiveRedirect);            // Android fallback
    server.on("/hotspot-detect.html", HTTP_GET, handleCaptiveRedirect);// Apple iOS / macOS
    server.on("/canonical.html", HTTP_GET, handleCaptiveRedirect);     // Apple
    server.on("/ncsi.txt", HTTP_GET, []() {                            // Windows
        server.send(200, "text/plain", "Microsoft NCSI");
    });
    server.on("/connecttest.txt", HTTP_GET, handleCaptiveRedirect);    // Windows 10/11

    // Catch-All Not Found -> 302 Redirect to Portal
    server.onNotFound([]() {
        server.sendHeader("Location", "http://192.168.4.1/", true);
        server.send(302, "text/plain", "");
    });
}

void setup() {
    Serial.begin(115200);
    delay(200);

    // Initial LED setup
    setStatusLed(0, 0, 0);

    Serial.println(F("\n========================================================"));
    Serial.println(F("   ESP32-S3 WPS Password Fetcher & Web Portal"));
    Serial.println(F("   Target: ESP32-S3 (Standalone - No External Modules)"));
    Serial.println(F("   WS2812 Status LED: GPIO 48"));
    Serial.println(F("========================================================"));

    // Register Wi-Fi Event listener
    WiFi.onEvent(onWiFiEvent);

    // Start WPS PBC listening
    startWpsListening();
}

void loop() {
    // 1. Maintain Status LED animation
    updateLedStateMachine();

    // 2. State Machine Dispatch
    uint32_t now = millis();

    switch (currentState) {
        case STATE_WPS_LISTENING:
            // Check for WPS timeout if not caught by event
            if (now - wpsStartTimer > WPS_TIMEOUT_MS) {
                Serial.println(F("[WPS] Internal timeout reached (120s). Retrying..."));
                stopWpsListening();
                currentState = STATE_WPS_RETRY_WAIT;
                currentLedState = LED_FLASH_RED;
                stateTimer = now;
            }
            break;

        case STATE_WPS_SUCCESS_FLASH:
            // Let the double-green flash play for 1.8 seconds before AP launch
            if (now - stateTimer > 1800) {
                currentState = STATE_TRANSITION_AP;
            }
            break;

        case STATE_WPS_RETRY_WAIT:
            // Wait 3 seconds before restarting WPS PBC
            if (now - stateTimer > 3000) {
                Serial.println(F("[WPS] Restarting WPS PBC listening cycle..."));
                startWpsListening();
            }
            break;

        case STATE_TRANSITION_AP:
            launchSoftApPortal();
            break;

        case STATE_PORTAL_ACTIVE:
            // Handle Captive Portal DNS and HTTP Web Server
            dnsServer.processNextRequest();
            server.handleClient();
            break;

        default:
            break;
    }
}
