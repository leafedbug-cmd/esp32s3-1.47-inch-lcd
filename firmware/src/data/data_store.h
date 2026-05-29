#pragma once
// =============================================================================
// Blip Terminal - Shared Data Store
// Holds latest parsed API payload. Written by api_client, read by screens.
// =============================================================================

#include <Arduino.h>
#include "../config.h"

// Number of history samples stored for sparklines
#define HISTORY_LEN 40

struct SystemData {
    int   cpu_pct      = 0;
    float cpu_temp     = 0.0f;
    int   ram_pct      = 0;
    int   gpu_pct      = 0;
    float gpu_temp     = 0.0f;
    int   vram_pct     = 0;
    int   disk_pct     = 0;
    float net_down     = 0.0f;   // Mbps
    float net_up       = 0.0f;   // Mbps

    // Sparkline history
    float cpu_hist[HISTORY_LEN]  = {};
    float ram_hist[HISTORY_LEN]  = {};
    float gpu_hist[HISTORY_LEN]  = {};
    int   hist_idx               = 0;

    void push_history() {
        cpu_hist[hist_idx] = (float)cpu_pct;
        ram_hist[hist_idx] = (float)ram_pct;
        gpu_hist[hist_idx] = (float)gpu_pct;
        hist_idx = (hist_idx + 1) % HISTORY_LEN;
    }

    // Return pointer to ordered history starting from oldest
    void get_ordered(float* src, float* dst) {
        for (int i = 0; i < HISTORY_LEN; i++) {
            dst[i] = src[(hist_idx + i) % HISTORY_LEN];
        }
    }
};

struct BlipData {
    char  model[48]       = "---";
    int   tpm             = 0;     // tokens per minute
    int   conversations   = 0;
    int   latency_ms      = 0;
    char  uptime[24]      = "---";
    int   mem_pct         = 0;
    char  agent_status[32] = "OFFLINE";
    bool  online          = false;
};

struct InternetData {
    char  weather[48]     = "---";
    char  weather_icon[8] = "?";
    char  external_ip[24] = "---";
    int   latency_ms      = 0;
    float dl_mbps         = 0.0f;
    float ul_mbps         = 0.0f;
};

struct DiscordData {
    int   members         = 0;
    int   online          = 0;
    int   msgs_today      = 0;
    int   voice_users     = 0;
    bool  connected       = false;
};

struct OpenClawData {
    int   agents          = 0;
    int   tasks           = 0;
    char  current_tool[48] = "---";
    int   queue_len       = 0;
    char  last_task[80]   = "---";
    char  log_lines[4][80] = {};   // 4 most recent log lines
    bool  running         = false;
};

struct NetworkScanData {
    struct WifiNet {
        char ssid[33] = {};
        int  rssi     = -100;
    } wifi[8];
    int wifi_count = 0;

    struct BleDevice {
        char name[24] = {};
        int  rssi     = -100;
    } ble[8];
    int ble_count = 0;

    int lan_devices = 0;
    int ping_ms     = 0;
};

// Singleton data store
struct DataStore {
    SystemData   system;
    BlipData     blip;
    InternetData internet;
    DiscordData  discord;
    OpenClawData openclaw;
    NetworkScanData network;

    bool     api_ok        = false;
    bool     wifi_ok       = false;
    uint32_t last_update   = 0;   // millis() of last successful fetch
    uint32_t uptime_ms     = 0;
};

extern DataStore g_data;
