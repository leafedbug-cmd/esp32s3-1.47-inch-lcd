#pragma once
// =============================================================================
// Blip Terminal - API Client
// Polls backend JSON endpoint, parses into DataStore
// =============================================================================

#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "../config.h"
#include "../data/data_store.h"

class ApiClient {
public:
    // Returns true if fetch+parse succeeded
    bool fetch() {
        if (WiFi.status() != WL_CONNECTED) return false;

        char url[80];
        snprintf(url, sizeof(url), "http://%s:%d%s",
                 API_HOST, API_PORT, API_ENDPOINT);

        HTTPClient http;
        http.begin(url);
        http.setTimeout(API_TIMEOUT_MS);
        http.addHeader("Accept", "application/json");

        int code = http.GET();
        if (code != 200) {
            http.end();
            g_data.api_ok = false;
            return false;
        }

        // Parse JSON into stack-allocated document (PSRAM backed on ESP32-S3)
        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, http.getStream());
        http.end();

        if (err) {
            g_data.api_ok = false;
            return false;
        }

        _parse(doc);
        g_data.api_ok     = true;
        g_data.last_update = millis();
        return true;
    }

private:
    void _parse(JsonDocument& doc) {
        // ---- System ----
        SystemData& s = g_data.system;
        s.cpu_pct  = doc[KEY_CPU_PCT]  | s.cpu_pct;
        s.cpu_temp = doc[KEY_CPU_TEMP] | s.cpu_temp;
        s.ram_pct  = doc[KEY_RAM_PCT]  | s.ram_pct;
        s.gpu_pct  = doc[KEY_GPU_PCT]  | s.gpu_pct;
        s.gpu_temp = doc[KEY_GPU_TEMP] | s.gpu_temp;
        s.vram_pct = doc[KEY_VRAM_PCT] | s.vram_pct;
        s.disk_pct = doc[KEY_DISK_PCT] | s.disk_pct;
        s.net_down = doc[KEY_NET_DOWN] | s.net_down;
        s.net_up   = doc[KEY_NET_UP]   | s.net_up;
        s.push_history();

        // ---- Blip AI ----
        BlipData& b = g_data.blip;
        _copy_str(b.model,        doc[KEY_AI_MODEL]   | "---");
        b.tpm           = doc[KEY_AI_TPM]     | b.tpm;
        b.conversations = doc[KEY_AI_CONVO]   | b.conversations;
        b.latency_ms    = doc[KEY_AI_LATENCY] | b.latency_ms;
        _copy_str(b.uptime,       doc[KEY_AI_UPTIME]  | "---");
        b.mem_pct       = doc[KEY_AI_MEM]     | b.mem_pct;
        _copy_str(b.agent_status, doc[KEY_AI_STATUS]  | "OFFLINE");
        b.online        = (strcmp(b.agent_status, "OFFLINE") != 0);

        // ---- Internet ----
        InternetData& inet = g_data.internet;
        _copy_str(inet.weather,     doc[KEY_WEATHER]     | "---");
        _copy_str(inet.weather_icon, doc[KEY_WEATHER_ICON] | "?");
        _copy_str(inet.external_ip, doc[KEY_EXTERNAL_IP] | "---");
        inet.latency_ms = doc[KEY_LATENCY] | inet.latency_ms;
        inet.dl_mbps    = doc[KEY_DL_SPEED] | inet.dl_mbps;
        inet.ul_mbps    = doc[KEY_UL_SPEED] | inet.ul_mbps;

        // ---- Discord ----
        DiscordData& dc = g_data.discord;
        dc.members    = doc[KEY_DC_MEMBERS] | dc.members;
        dc.online     = doc[KEY_DC_ONLINE]  | dc.online;
        dc.msgs_today = doc[KEY_DC_MSGS]    | dc.msgs_today;
        dc.voice_users = doc[KEY_DC_VOICE]  | dc.voice_users;
        dc.connected  = (dc.members > 0);

        // ---- OpenClaw ----
        OpenClawData& oc = g_data.openclaw;
        oc.agents    = doc[KEY_OC_AGENTS] | oc.agents;
        oc.tasks     = doc[KEY_OC_TASKS]  | oc.tasks;
        _copy_str(oc.current_tool, doc[KEY_OC_TOOL]      | "---");
        oc.queue_len = doc[KEY_OC_QUEUE]  | oc.queue_len;
        _copy_str(oc.last_task,    doc[KEY_OC_LAST_TASK] | "---");
        oc.running   = (oc.agents > 0 || oc.tasks > 0);

        // Log lines array
        JsonArray log_arr = doc[KEY_OC_LOG].as<JsonArray>();
        if (log_arr) {
            int i = 0;
            for (JsonVariant v : log_arr) {
                if (i >= 4) break;
                _copy_str(oc.log_lines[i], v.as<const char*>() ? v.as<const char*>() : "");
                i++;
            }
        }

        // ---- Network scan ----
        NetworkScanData& ns = g_data.network;
        ns.ping_ms    = doc[KEY_PING]        | ns.ping_ms;
        ns.lan_devices = doc[KEY_LAN_DEVICES] | ns.lan_devices;

        JsonArray wifi_arr = doc[KEY_WIFI_NETS].as<JsonArray>();
        if (wifi_arr) {
            ns.wifi_count = 0;
            for (JsonObject net : wifi_arr) {
                if (ns.wifi_count >= 8) break;
                _copy_str(ns.wifi[ns.wifi_count].ssid, net["ssid"] | "");
                ns.wifi[ns.wifi_count].rssi = net["rssi"] | -100;
                ns.wifi_count++;
            }
        }

        JsonArray ble_arr = doc[KEY_BLE_DEVICES].as<JsonArray>();
        if (ble_arr) {
            ns.ble_count = 0;
            for (JsonObject dev : ble_arr) {
                if (ns.ble_count >= 8) break;
                _copy_str(ns.ble[ns.ble_count].name, dev["name"] | "");
                ns.ble[ns.ble_count].rssi = dev["rssi"] | -100;
                ns.ble_count++;
            }
        }
    }

    // Safe string copy (null-safe, bounded)
    template<size_t N>
    void _copy_str(char (&dst)[N], const char* src) {
        if (src) strncpy(dst, src, N - 1);
        dst[N - 1] = '\0';
    }
};
