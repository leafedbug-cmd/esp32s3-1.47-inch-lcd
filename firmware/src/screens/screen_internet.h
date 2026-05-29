#pragma once
// =============================================================================
// Screen 3: Internet Status
// Weather, IP, latency, download/upload speeds
// =============================================================================

#include "screen_base.h"

class ScreenInternet : public ScreenBase {
public:
    const char* name()      const override { return ">_ INTERNET"; }
    const char* index_str() const override { return "3/6"; }

    void draw() override {
        display_clear();
        draw_header(name(), index_str(), g_data.wifi_ok, g_data.api_ok);

        InternetData& d = g_data.internet;
        char buf[48];

        // ---- Weather (big top section) ----
        int wy = CONTENT_Y + 2;
        spr.setTextFont(FONT_SMALL);
        spr.setTextColor(COLOR_ORANGE, COLOR_BG);
        spr.setTextDatum(TL_DATUM);
        spr.drawString(d.weather, COL1_X, wy);

        // Weather icon (emoji-style char)
        spr.setTextFont(FONT_SMALL);
        spr.setTextColor(COLOR_YELLOW, COLOR_BG);
        spr.setTextDatum(TR_DATUM);
        spr.drawString(d.weather_icon, SCREEN_WIDTH - 4, wy);

        draw_divider(wy + 18);

        // ---- Grid of stats ----
        int gy = wy + 22;

        // External IP
        spr.setTextFont(FONT_TINY);
        spr.setTextColor(COLOR_GREY_DIM, COLOR_BG);
        spr.setTextDatum(TL_DATUM);
        spr.drawString("EXT IP", COL1_X, gy);
        spr.setTextColor(COLOR_CYAN, COLOR_BG);
        spr.drawString(d.external_ip, COL1_X + 42, gy);

        gy += 14;

        // Latency
        spr.setTextColor(COLOR_GREY_DIM, COLOR_BG);
        spr.drawString("PING", COL1_X, gy);
        uint16_t ping_col = d.latency_ms > 200 ? COLOR_RED
                          : d.latency_ms > 80  ? COLOR_ORANGE
                          : COLOR_GREEN;
        snprintf(buf, sizeof(buf), "%d ms", d.latency_ms);
        spr.setTextColor(ping_col, COLOR_BG);
        spr.drawString(buf, COL1_X + 42, gy);

        // Download speed
        spr.setTextColor(COLOR_GREY_DIM, COLOR_BG);
        spr.drawString("DOWN", COL2_X, gy);
        snprintf(buf, sizeof(buf), "%.1f Mb/s", d.dl_mbps);
        spr.setTextColor(COLOR_GREEN, COLOR_BG);
        spr.drawString(buf, COL2_X + 34, gy);

        // Upload speed
        spr.setTextColor(COLOR_GREY_DIM, COLOR_BG);
        spr.drawString("UP", COL3_X, gy);
        snprintf(buf, sizeof(buf), "%.1f Mb/s", d.ul_mbps);
        spr.setTextColor(COLOR_CYAN_DIM, COLOR_BG);
        spr.drawString(buf, COL3_X + 20, gy);

        gy += 16;
        draw_divider(gy);
        gy += 4;

        // ---- Speed bars ----
        float max_speed = 1000.0f;  // 1Gbps reference
        int dl_pct = (int)(d.dl_mbps / max_speed * 100.0f);
        int ul_pct = (int)(d.ul_mbps / max_speed * 100.0f);
        dl_pct = constrain(dl_pct, 0, 100);
        ul_pct = constrain(ul_pct, 0, 100);

        draw_bar(COL1_X, gy, CONTENT_W / 2 - 4, "DL", dl_pct, COLOR_GREEN);
        draw_bar(COL1_X + CONTENT_W / 2, gy, CONTENT_W / 2 - 4, "UL", ul_pct, COLOR_CYAN);

        draw_footer("NETWORK STATUS", true);
        display_flush();
    }
};
