#pragma once
// =============================================================================
// Screen 5: OpenClaw Operations
// Active agents, task queue, current tool, scrolling activity log
// =============================================================================

#include "screen_base.h"

class ScreenOpenClaw : public ScreenBase {
public:
    const char* name()      const override { return ">_ OPENCLAW OPS"; }
    const char* index_str() const override { return "5/6"; }

    void on_enter() override {
        _scroll_y   = 0;
        _last_tick  = millis();
    }

    void draw() override {
        display_clear();
        draw_header(name(), index_str(), g_data.wifi_ok, g_data.api_ok);

        OpenClawData& oc = g_data.openclaw;
        char buf[64];

        // ---- Status row ----
        int ry = CONTENT_Y + 2;
        draw_status_dot(COL1_X, ry, oc.running ? "RUNNING" : "IDLE", oc.running);

        // Agents + Tasks
        spr.setTextFont(FONT_TINY);
        spr.setTextColor(COLOR_GREY_DIM, COLOR_BG);
        spr.setTextDatum(TL_DATUM);

        spr.drawString("AGENTS", COL2_X, ry);
        snprintf(buf, sizeof(buf), "%d", oc.agents);
        spr.setTextColor(COLOR_GREEN, COLOR_BG);
        spr.drawString(buf, COL2_X + 48, ry);

        spr.setTextColor(COLOR_GREY_DIM, COLOR_BG);
        spr.drawString("TASKS", COL2_X + 72, ry);
        snprintf(buf, sizeof(buf), "%d", oc.tasks);
        spr.setTextColor(COLOR_ORANGE, COLOR_BG);
        spr.drawString(buf, COL2_X + 110, ry);

        // Queue length
        spr.setTextColor(COLOR_GREY_DIM, COLOR_BG);
        spr.drawString("Q:", SCREEN_WIDTH - 44, ry);
        snprintf(buf, sizeof(buf), "%d", oc.queue_len);
        spr.setTextColor(oc.queue_len > 5 ? COLOR_ORANGE : COLOR_CYAN, COLOR_BG);
        spr.drawString(buf, SCREEN_WIDTH - 28, ry);

        // ---- Current tool ----
        int ty = ry + 12;
        spr.setTextFont(FONT_TINY);
        spr.setTextColor(COLOR_GREY_DIM, COLOR_BG);
        spr.drawString("TOOL:", COL1_X, ty);
        spr.setTextColor(COLOR_CYAN, COLOR_BG);
        spr.drawString(oc.current_tool, COL1_X + 36, ty);

        // Blinking cursor after tool name when running
        if (oc.running && blink_state()) {
            int tw = spr.textWidth(oc.current_tool);
            spr.setTextColor(COLOR_GREEN, COLOR_BG);
            spr.drawString("|", COL1_X + 36 + tw + 2, ty);
        }

        draw_divider(ty + 12);

        // ---- Scrolling log terminal ----
        // 4 log lines in a box, scroll one pixel per tick
        int lx = COL1_X;
        int ly = ty + 15;
        int lw = CONTENT_W - 2;
        int lh = SCREEN_HEIGHT - FOOTER_H - ly - 4;

        spr.fillRect(lx, ly, lw, lh, COLOR_BG_PANEL);
        spr.drawRect(lx, ly, lw, lh, COLOR_GREEN_DARK);

        // Terminal prompt line
        spr.setTextFont(FONT_TINY);
        spr.setTextColor(COLOR_GREEN_DIM, COLOR_BG_PANEL);
        spr.setTextDatum(TL_DATUM);

        // Calculate scroll
        uint32_t now = millis();
        if (now - _last_tick >= 60) {
            _scroll_y = (_scroll_y + SCROLL_SPEED_PX) % 9;  // 9px per line
            _last_tick = now;
        }

        // Draw up to 4 log lines — manually bounds-check each line
        int clip_top    = ly + 1;
        int clip_bottom = ly + lh - 1;
        for (int i = 0; i < 4; i++) {
            int line_y = ly + 2 + i * 9 - _scroll_y;
            // Only draw if the 8px-tall text row is fully inside the panel
            if (line_y >= clip_top && line_y + 8 <= clip_bottom) {
                spr.setTextColor(i == 3 ? COLOR_GREEN : COLOR_GREEN_DARK, COLOR_BG_PANEL);
                snprintf(buf, sizeof(buf), "> %s", oc.log_lines[i]);
                spr.drawString(buf, lx + 2, line_y);
            }
        }
        // Prompt line
        {
            int prompt_y = ly + 2 + 4 * 9 - _scroll_y;
            if (prompt_y >= clip_top && prompt_y + 8 <= clip_bottom) {
                spr.setTextColor(COLOR_GREEN, COLOR_BG_PANEL);
                spr.drawString(blink_state() ? "> _" : ">", lx + 2, prompt_y);
            }
        }

        draw_footer("OPENCLAW AI ENGINE", true);
        display_flush();
    }

private:
    int      _scroll_y  = 0;
    uint32_t _last_tick = 0;
};
