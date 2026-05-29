#pragma once
// =============================================================================
// Screen 2: PC Status
// CPU, RAM, GPU bars + sparklines + temp warnings
// =============================================================================

#include "screen_base.h"

class ScreenPC : public ScreenBase {
public:
    const char* name()      const override { return ">_ PC STATUS"; }
    const char* index_str() const override { return "2/6"; }

    void draw() override {
        display_clear();
        draw_header(name(), index_str(), g_data.wifi_ok, g_data.api_ok);

        SystemData& s = g_data.system;

        // Content area starts at CONTENT_Y
        // Layout: 3 columns, each with bar + sparkline
        // Col widths: ~100px each

        // ---- Row 1: CPU ----
        int ry = CONTENT_Y + 2;
        _draw_stat_row(ry, "CPU", s.cpu_pct, s.cpu_temp,
                       s.cpu_hist, s.hist_idx, COLOR_GREEN);

        // ---- Row 2: RAM ----
        ry += 38;
        _draw_stat_row(ry, "RAM", s.ram_pct, -1,
                       s.ram_hist, s.hist_idx, COLOR_CYAN);

        // ---- Row 3: GPU ----
        ry += 38;
        _draw_stat_row(ry, "GPU", s.gpu_pct, s.gpu_temp,
                       s.gpu_hist, s.hist_idx, COLOR_ORANGE);

        // ---- Row 4: mini stats ----
        ry += 38;
        char buf[24];

        spr.setTextFont(FONT_TINY);
        spr.setTextDatum(TL_DATUM);

        // VRAM
        spr.setTextColor(COLOR_GREY_DIM, COLOR_BG);
        spr.drawString("VRAM", COL1_X, ry);
        snprintf(buf, sizeof(buf), "%d%%", s.vram_pct);
        spr.setTextColor(s.vram_pct > 85 ? COLOR_ORANGE : COLOR_CYAN, COLOR_BG);
        spr.drawString(buf, COL1_X + 28, ry);

        // DISK
        spr.setTextColor(COLOR_GREY_DIM, COLOR_BG);
        spr.drawString("DISK", COL1_X + 72, ry);
        snprintf(buf, sizeof(buf), "%d%%", s.disk_pct);
        spr.setTextColor(s.disk_pct > 85 ? COLOR_ORANGE : COLOR_CYAN, COLOR_BG);
        spr.drawString(buf, COL1_X + 100, ry);

        // NET
        spr.setTextColor(COLOR_GREY_DIM, COLOR_BG);
        spr.drawString("NET", COL1_X + 152, ry);
        snprintf(buf, sizeof(buf), "%.1f/%.1f", s.net_down, s.net_up);
        spr.setTextColor(COLOR_GREEN_DIM, COLOR_BG);
        spr.drawString(buf, COL1_X + 178, ry);
        spr.setTextColor(COLOR_GREY_DIM, COLOR_BG);
        spr.drawString("Mb", COL1_X + 232, ry);

        draw_footer("SYSTEM TELEMETRY", true);
        display_flush();
    }

private:
    void _draw_stat_row(int y, const char* label, int pct, float temp,
                        float* hist, int hist_idx, uint16_t bar_color) {
        char buf[24];

        // Label + percentage (left)
        spr.setTextFont(FONT_TINY);
        spr.setTextColor(COLOR_GREY, COLOR_BG);
        spr.setTextDatum(TL_DATUM);
        spr.drawString(label, COL1_X, y);

        snprintf(buf, sizeof(buf), "%3d%%", pct);
        uint16_t pct_color = pct >= 90 ? COLOR_RED
                           : pct >= 75 ? COLOR_ORANGE
                           : bar_color;
        spr.setTextColor(pct_color, COLOR_BG);
        spr.drawString(buf, COL1_X + 22, y);

        // Temp (if provided)
        if (temp >= 0) {
            uint16_t tc = temp >= 85 ? COLOR_RED
                        : temp >= 70 ? COLOR_ORANGE
                        : COLOR_CYAN_DIM;
            snprintf(buf, sizeof(buf), "%.0fC", temp);
            spr.setTextColor(tc, COLOR_BG);
            spr.drawString(buf, COL1_X + 58, y);
        }

        // Bar (middle)
        draw_bar(COL1_X + 88, y, 148, "", pct, bar_color);

        // Sparkline (right)
        float ordered[HISTORY_LEN];
        g_data.system.get_ordered(hist, ordered);
        draw_sparkline(COL1_X + 240, y - 2, SPARKLINE_W - 4, SPARKLINE_H - 4,
                       ordered, HISTORY_LEN, bar_color);
    }
};
