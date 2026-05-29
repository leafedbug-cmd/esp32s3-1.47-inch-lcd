#pragma once
// =============================================================================
// Blip Terminal - WiFi Manager
// Handles connection, automatic reconnection, status reporting
// =============================================================================

#include <WiFi.h>
#include "../config.h"
#include "../data/data_store.h"

class WiFiMgr {
public:
    void begin() {
        WiFi.mode(WIFI_STA);
        WiFi.setAutoReconnect(true);
        WiFi.persistent(false);
        _connect();
    }

    // Call every loop - handles reconnection
    void tick() {
        if (WiFi.status() != WL_CONNECTED) {
            g_data.wifi_ok = false;
            uint32_t now = millis();
            if (now - _last_retry > _retry_interval) {
                _last_retry = now;
                _retry_interval = min(_retry_interval * 2, (uint32_t)30000);
                WiFi.reconnect();
            }
        } else {
            g_data.wifi_ok = true;
            _retry_interval = 5000;  // reset backoff on success
        }
    }

    bool is_connected() { return WiFi.status() == WL_CONNECTED; }

private:
    uint32_t _last_retry      = 0;
    uint32_t _retry_interval  = 5000;

    void _connect() {
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        uint32_t start = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
            delay(200);
        }
        g_data.wifi_ok = (WiFi.status() == WL_CONNECTED);
    }
};
