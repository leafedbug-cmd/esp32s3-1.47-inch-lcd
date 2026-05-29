#pragma once
// =============================================================================
// Screen 1: Blip AI Status
// Makes Blip feel alive — model info, latency, status, uptime, memory
// =============================================================================

#include "screen_base.h"

class ScreenBlip : public ScreenBase {
public:
    const char* name()      const override { return ">_ BLIP STATUS"; }
    const char* index_str() const override { return "1/6"; }

    void on_enter() override {
        _pulse_t = millis();
    }

    void draw() override {
        display_clear();

        BlipData& b = g_data.blip;
        bool online = b.online;

        // ---- Header ----
        draw_header(name(), index_str(), g_data.wifi_ok, g_data.api_ok);

        // ---- Online / Offline hero indicator ----
        int cy = CONTENT_Y + 6;

        // Pulsing status circle
        uint32_t elapsed = millis() - _pulse_t;
        int pulse_r = 6 + (int)(2.0f * sinf((float)elapsed / (PULSE_PERIOD_MS / 6.2832f)));
        uint16_t pulse_col = online ? COLOR_GREEN : COLOR_RED;
        spr.fillCircle(14, cy + pulse_r + 2, pulse_r, pulse_col);
        spr.drawCircle(14, cy + pulse_r + 2, pulse_r + 3, pulse_col);

        // Status text
        spr.setTextFont(FONT_SMALL);
        spr.setTextColor(online ? COLOR_GREEN : COLOR_RED, COLOR_BG);
        spr.setTextDatum(ML_DATUM);
        spr.drawString(online ? "ONLINE" : "OFFLINE", 30, cy + 8);

        // Agent status
        spr.setTextFont(FONT_TINY);
        spr.setTextColor(COLOR_CYAN_DIM, COLOR_BG);
        spr.drawString(b.agent_status, 30, cy + 19);

        // Uptime (top right)
        spr.setTextDatum(TR_DATUM);
        spr.setTextColor(COLOR_GREY_DIM, COLOR_BG);
        spr.drawString("UPTIME", SCREEN_WIDTH - 4, cy);
        spr.setTextColor(COLOR_CYAN, COLOR_BG);
        spr.drawString(b.uptime, SCREEN_WIDTH - 4, cy + 10);

        // ---- Divider ----
        draw_divider(cy + 30);

        // ---- Model row ----
        int ry = cy + 34;
        spr.setTextFont(FONT_TINY);
        spr.setTextColor(COLOR_GREY_DIM, COLOR_BG);
        spr.setTextDatum(TL_DATUM);
        spr.drawString("MODEL", COL1_X, ry);
        spr.setTextColor(COLOR_ORANGE, COLOR_BG);
        spr.drawString(b.model, COL1_X + 38, ry);

        // ---- Metrics grid ----
        int my = ry + 14;

        // Col 1: TPM
        _draw_metric(COL1_X,  my, "TPM",     b.tpm,        COLOR_GREEN, " tok/m");
        // Col 2: Latency
        _draw_metric(COL2_X,  my, "LATENCY", b.latency_ms, COLOR_CYAN,  " ms");
        // Col 3: Convos
        _draw_metric(COL3_X,  my, "CONVOS",  b.conversations, COLOR_ORANGE, "");

        // ---- Memory bar ----
        int by = my + 30;
        draw_bar(COL1_X, by, CONTENT_W - 4, "MEM", b.mem_pct, COLOR_PURPLE);

        // ---- Blinking cursor at bottom-left ----
        if (blink_state()) {
            spr.setTextFont(FONT_TINY);
            spr.setTextColor(COLOR_GREEN, COLOR_BG);
            spr.setTextDatum(TL_DATUM);
            spr.drawString("_", COL1_X, by + 18);
        }

        // ---- Footer ----
        char footer[40];
        snprintf(footer, sizeof(footer), "BLIP v1 | LAST UPD: %lus AGO",
                 (millis() - g_data.last_update) / 1000);
        draw_footer(footer, true);

        display_flush();
    }

private:
    uint32_t _pulse_t = 0;

    void _draw_metric(int x, int y, const char* label, int value,
                      uint16_t vc, const char* unit) {
        char val_buf[24];
        snprintf(val_buf, sizeof(val_buf), "%d%s", value, unit);

        spr.setTextFont(FONT_TINY);
        spr.setTextColor(COLOR_GREY_DIM, COLOR_BG);
        spr.setTextDatum(TL_DATUM);
        spr.drawString(label, x, y);

        spr.setTextColor(vc, COLOR_BG);
        spr.drawString(val_buf, x, y + 10);
    }
};
