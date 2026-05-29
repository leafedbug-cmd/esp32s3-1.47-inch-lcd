#pragma once
// =============================================================================
// Screen 6: BuggedLab Network Tools
// WiFi scan, BLE devices, LAN device count, ping monitor
// =============================================================================

#include "screen_base.h"

class ScreenNetwork : public ScreenBase {
public:
    const char* name()      const override { return ">_ BUGGEDLAB NET"; }
    const char* index_str() const override { return "6/6"; }

    void draw() override {
        display_clear();
        draw_header(name(), index_str(), g_data.wifi_ok, g_data.api_ok);

        NetworkScanData& n = g_data.network;
        char buf[48];

        // ---- Top stats row ----
        int ry = CONTENT_Y + 2;
        spr.setTextFont(FONT_TINY);
        spr.setTextDatum(TL_DATUM);

        // Ping
        uint16_t ping_col = n.ping_ms > 100 ? COLOR_RED
                          : n.ping_ms > 40  ? COLOR_ORANGE
                          : COLOR_GREEN;
        spr.setTextColor(COLOR_GREY_DIM, COLOR_BG);
        spr.drawString("PING", COL1_X, ry);
        snprintf(buf, sizeof(buf), "%d ms", n.ping_ms);
        spr.setTextColor(ping_col, COLOR_BG);
        spr.drawString(buf, COL1_X + 28, ry);

        // LAN devices
        spr.setTextColor(COLOR_GREY_DIM, COLOR_BG);
        spr.drawString("LAN", COL2_X, ry);
        snprintf(buf, sizeof(buf), "%d hosts", n.lan_devices);
        spr.setTextColor(COLOR_CYAN, COLOR_BG);
        spr.drawString(buf, COL2_X + 22, ry);

        // BLE count
        spr.setTextColor(COLOR_GREY_DIM, COLOR_BG);
        spr.drawString("BLE", COL3_X, ry);
        snprintf(buf, sizeof(buf), "%d dev", n.ble_count);
        spr.setTextColor(COLOR_PURPLE, COLOR_BG);
        spr.drawString(buf, COL3_X + 22, ry);

        draw_divider(ry + 12);

        // ---- Two column layout: WiFi | BLE ----
        int col_y = ry + 16;
        int half  = CONTENT_W / 2;

        // WiFi header
        spr.setTextColor(COLOR_GREEN, COLOR_BG);
        spr.drawString("WIFI NETWORKS", COL1_X, col_y);
        // BLE header
        spr.setTextColor(COLOR_PURPLE, COLOR_BG);
        spr.drawString("BLE DEVICES", COL1_X + half + 2, col_y);

        // Vertical divider
        spr.drawFastVLine(COL1_X + half, col_y, CONTENT_H - 24, COLOR_GREEN_DARK);

        col_y += 10;

        // WiFi list (up to 5)
        int max_rows = 5;
        for (int i = 0; i < min(n.wifi_count, max_rows); i++) {
            int bar_w = _rssi_to_bars(n.wifi[i].rssi);
            uint16_t sig_col = bar_w >= 3 ? COLOR_GREEN
                             : bar_w >= 2 ? COLOR_ORANGE
                             : COLOR_RED;

            // SSID (truncated to fit)
            char ssid_buf[18];
            strncpy(ssid_buf, n.wifi[i].ssid, 17);
            ssid_buf[17] = '\0';

            spr.setTextColor(COLOR_GREY, COLOR_BG);
            spr.drawString(ssid_buf, COL1_X, col_y + i * 10);

            // Signal bars (drawn as tiny rects)
            int bx = COL1_X + half - 16;
            for (int b = 0; b < 4; b++) {
                uint16_t bc = b < bar_w ? sig_col : COLOR_GREY_DARK;
                spr.fillRect(bx + b * 4, col_y + i * 10 + (3 - b),
                             3, (b + 1) * 2, bc);
            }
        }
        if (n.wifi_count == 0) {
            spr.setTextColor(COLOR_GREY_DARK, COLOR_BG);
            spr.drawString("SCANNING...", COL1_X, col_y);
        }

        // BLE list (up to 5)
        int bx2 = COL1_X + half + 4;
        for (int i = 0; i < min(n.ble_count, max_rows); i++) {
            spr.setTextColor(COLOR_GREY, COLOR_BG);
            char ble_buf[14];
            strncpy(ble_buf, n.ble[i].name[0] ? n.ble[i].name : "Unknown", 13);
            ble_buf[13] = '\0';
            spr.drawString(ble_buf, bx2, col_y + i * 10);

            snprintf(buf, sizeof(buf), "%d", n.ble[i].rssi);
            spr.setTextColor(COLOR_PURPLE, COLOR_BG);
            spr.setTextDatum(TR_DATUM);
            spr.drawString(buf, COL1_X + half + half - 2, col_y + i * 10);
            spr.setTextDatum(TL_DATUM);
        }
        if (n.ble_count == 0) {
            spr.setTextColor(COLOR_GREY_DARK, COLOR_BG);
            spr.drawString("SCANNING...", bx2, col_y);
        }

        draw_footer("BUGGEDLAB NETWORK TOOLS", true);
        display_flush();
    }

private:
    // Convert RSSI to 1-4 bars
    int _rssi_to_bars(int rssi) {
        if (rssi >= -55) return 4;
        if (rssi >= -67) return 3;
        if (rssi >= -78) return 2;
        if (rssi >= -89) return 1;
        return 0;
    }
};
