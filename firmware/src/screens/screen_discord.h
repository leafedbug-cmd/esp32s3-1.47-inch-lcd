#pragma once
// =============================================================================
// Screen 4: Discord Status
// Members, online, messages, voice activity
// =============================================================================

#include "screen_base.h"

class ScreenDiscord : public ScreenBase {
public:
    const char* name()      const override { return ">_ DISCORD"; }
    const char* index_str() const override { return "4/6"; }

    void draw() override {
        display_clear();
        draw_header(name(), index_str(), g_data.wifi_ok, g_data.api_ok);

        DiscordData& d = g_data.discord;
        char buf[32];

        // ---- Connection status ----
        int sy = CONTENT_Y + 2;
        draw_status_dot(COL1_X, sy, d.connected ? "CONNECTED" : "DISCONNECTED", d.connected);

        // ---- Big number grid ----
        int gy = sy + 18;
        draw_divider(gy - 2);
        gy += 4;

        // Total members
        _draw_big_stat(COL1_X, gy, 90, "MEMBERS", d.members, COLOR_CYAN);

        // Online members
        _draw_big_stat(COL2_X - 10, gy, 90, "ONLINE",
                       d.online, COLOR_GREEN);

        // Messages today
        _draw_big_stat(COL3_X - 4, gy, 90, "MSG TODAY", d.msgs_today, COLOR_ORANGE);

        gy += 50;
        draw_divider(gy);
        gy += 4;

        // ---- Voice activity ----
        spr.setTextFont(FONT_TINY);
        spr.setTextColor(COLOR_GREY_DIM, COLOR_BG);
        spr.setTextDatum(TL_DATUM);
        spr.drawString("VOICE", COL1_X, gy);

        if (d.voice_users > 0) {
            snprintf(buf, sizeof(buf), "%d USER%s IN VC",
                     d.voice_users, d.voice_users == 1 ? "" : "S");
            spr.setTextColor(COLOR_GREEN, COLOR_BG);

            // Animated waveform indicator
            if (blink_state()) {
                spr.fillRect(COL1_X + 44, gy + 1, 4, 6, COLOR_GREEN);
                spr.fillRect(COL1_X + 50, gy - 1, 4, 10, COLOR_GREEN);
                spr.fillRect(COL1_X + 56, gy + 2, 4, 4, COLOR_GREEN);
                spr.fillRect(COL1_X + 62, gy, 4, 8, COLOR_GREEN);
            }
            spr.drawString(buf, COL1_X + 72, gy);
        } else {
            spr.setTextColor(COLOR_GREY_DARK, COLOR_BG);
            spr.drawString("NO ACTIVE VOICE", COL1_X + 44, gy);
        }

        // Online percentage mini-bar
        gy += 14;
        int online_pct = d.members > 0 ? (d.online * 100 / d.members) : 0;
        draw_bar(COL1_X, gy, CONTENT_W - 4, "ONLINE%", online_pct, COLOR_GREEN);

        draw_footer("DISCORD MONITOR", true);
        display_flush();
    }

private:
    void _draw_big_stat(int x, int y, int w,
                        const char* label, int value, uint16_t color) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%d", value);

        // Big value
        spr.setTextFont(FONT_MEDIUM);
        spr.setTextColor(color, COLOR_BG);
        spr.setTextDatum(TC_DATUM);
        spr.drawString(buf, x + w / 2, y + 6);

        // Label below
        spr.setTextFont(FONT_TINY);
        spr.setTextColor(COLOR_GREY_DIM, COLOR_BG);
        spr.drawString(label, x + w / 2, y + 32);
    }
};
