#pragma once
// =============================================================================
// Blip Terminal - Display Manager
// Wraps TFT_eSPI, handles init, backlight PWM, and shared drawing helpers
// =============================================================================

#include <TFT_eSPI.h>
#include "theme.h"
#include "../config.h"

extern TFT_eSPI tft;
extern TFT_eSprite spr;     // Full-screen sprite for flicker-free rendering

// -----------------------------------------------------------------------------
// Initialise display + backlight
// -----------------------------------------------------------------------------
inline void display_init() {
    tft.init();
    tft.setRotation(SCREEN_ROTATION);
    tft.fillScreen(COLOR_BG);
    tft.setTextColor(COLOR_GREEN, COLOR_BG);
    tft.setTextDatum(TL_DATUM);

    // Backlight — drive HIGH first as a guaranteed fallback, then apply PWM
    pinMode(BACKLIGHT_PIN, OUTPUT);
    digitalWrite(BACKLIGHT_PIN, HIGH);
    ledcSetup(BACKLIGHT_CHANNEL, BACKLIGHT_FREQ, BACKLIGHT_RESOLUTION);
    ledcAttachPin(BACKLIGHT_PIN, BACKLIGHT_CHANNEL);
    ledcWrite(BACKLIGHT_CHANNEL, BACKLIGHT_DEFAULT);

    // Full-screen sprite (uses PSRAM)
    spr.setColorDepth(16);
    spr.createSprite(SCREEN_WIDTH, SCREEN_HEIGHT);
}

inline void display_set_brightness(uint8_t brightness) {
    ledcWrite(BACKLIGHT_CHANNEL, brightness);
}

// Push the sprite to the display (double-buffer flip)
inline void display_flush() {
    spr.pushSprite(0, 0);
}

// Clear sprite to background
inline void display_clear() {
    spr.fillSprite(COLOR_BG);
}

// -----------------------------------------------------------------------------
// Common drawing helpers (operate on sprite)
// -----------------------------------------------------------------------------

// Draw top header bar
inline void draw_header(const char* title, const char* screen_num,
                         bool wifi_ok, bool api_ok) {
    spr.fillRect(0, 0, SCREEN_WIDTH, HEADER_H, COLOR_BG_HEADER);
    spr.drawFastHLine(0, HEADER_H, SCREEN_WIDTH, COLOR_GREEN_DARK);

    // Title
    spr.setTextColor(COLOR_GREEN, COLOR_BG_HEADER);
    spr.setTextDatum(ML_DATUM);
    spr.setTextFont(FONT_TINY);
    spr.drawString(title, 4, HEADER_H / 2);

    // Screen number / nav indicator
    spr.setTextColor(COLOR_GREY_DIM, COLOR_BG_HEADER);
    spr.setTextDatum(MR_DATUM);
    spr.drawString(screen_num, SCREEN_WIDTH - 30, HEADER_H / 2);

    // WiFi dot
    uint16_t wifi_col = wifi_ok ? COLOR_STATUS_OK : COLOR_STATUS_ERR;
    spr.fillCircle(SCREEN_WIDTH - 20, HEADER_H / 2, 3, wifi_col);
    // API dot
    uint16_t api_col = api_ok ? COLOR_STATUS_OK : COLOR_STATUS_WARN;
    spr.fillCircle(SCREEN_WIDTH - 10, HEADER_H / 2, 3, api_col);
}

// Draw bottom footer bar with timestamp / mode
inline void draw_footer(const char* status_text, bool auto_mode) {
    int fy = SCREEN_HEIGHT - FOOTER_H;
    spr.fillRect(0, fy, SCREEN_WIDTH, FOOTER_H, COLOR_BG_HEADER);
    spr.drawFastHLine(0, fy, SCREEN_WIDTH, COLOR_GREEN_DARK);

    spr.setTextFont(FONT_TINY);
    spr.setTextColor(COLOR_GREY_DIM, COLOR_BG_HEADER);
    spr.setTextDatum(ML_DATUM);
    spr.drawString(status_text, 4, fy + FOOTER_H / 2);

    spr.setTextDatum(MR_DATUM);
    spr.setTextColor(auto_mode ? COLOR_GREEN_DIM : COLOR_ORANGE_DIM, COLOR_BG_HEADER);
    spr.drawString(auto_mode ? "AUTO" : "MANUAL", SCREEN_WIDTH - 4, fy + FOOTER_H / 2);
}

// Draw a labelled horizontal bar graph
// x,y = top-left of label area, w = bar width
inline void draw_bar(int x, int y, int w, const char* label, int value_pct,
                     uint16_t bar_color = COLOR_GREEN) {
    // Clamp
    if (value_pct < 0) value_pct = 0;
    if (value_pct > 100) value_pct = 100;

    // Warn color override
    if (value_pct >= 90) bar_color = COLOR_RED;
    else if (value_pct >= 75) bar_color = COLOR_ORANGE;

    // Label
    spr.setTextFont(FONT_TINY);
    spr.setTextColor(COLOR_GREY, COLOR_BG);
    spr.setTextDatum(ML_DATUM);
    spr.drawString(label, x, y + BAR_H / 2);

    int label_w = 36;
    int bx = x + label_w;
    int bw = w - label_w - 24;

    // Bar background
    spr.fillRect(bx, y, bw, BAR_H, COLOR_BG_PANEL);
    spr.drawRect(bx, y, bw, BAR_H, COLOR_GREY_DARK);

    // Bar fill
    int fill_w = (bw - 2) * value_pct / 100;
    if (fill_w > 0) {
        spr.fillRect(bx + 1, y + 1, fill_w, BAR_H - 2, bar_color);
    }

    // Percentage text
    char buf[8];
    snprintf(buf, sizeof(buf), "%d%%", value_pct);
    spr.setTextColor(COLOR_WHITE, COLOR_BG);
    spr.setTextDatum(MR_DATUM);
    spr.drawString(buf, x + w, y + BAR_H / 2);
}

// Draw a value label pair: "LABEL  VALUE"
inline void draw_kv(int x, int y, const char* key, const char* val,
                    uint16_t val_color = COLOR_CYAN) {
    spr.setTextFont(FONT_TINY);
    spr.setTextColor(COLOR_GREY_DIM, COLOR_BG);
    spr.setTextDatum(TL_DATUM);
    spr.drawString(key, x, y);

    spr.setTextColor(val_color, COLOR_BG);
    spr.drawString(val, x, y + 9);
}

// Draw a status dot with label
inline void draw_status_dot(int x, int y, const char* label, bool online) {
    uint16_t c = online ? COLOR_STATUS_OK : COLOR_STATUS_ERR;
    spr.fillCircle(x + 4, y + 4, 4, c);
    spr.setTextFont(FONT_TINY);
    spr.setTextColor(online ? COLOR_GREEN : COLOR_RED, COLOR_BG);
    spr.setTextDatum(ML_DATUM);
    spr.drawString(label, x + 12, y + 4);
}

// Draw a thin divider line
inline void draw_divider(int y) {
    spr.drawFastHLine(CONTENT_X, y, CONTENT_W, COLOR_GREEN_DARK);
}

// Draw a panel box (border only)
inline void draw_panel(int x, int y, int w, int h, uint16_t border_color = COLOR_GREEN_DARK) {
    spr.drawRect(x, y, w, h, border_color);
}

// Draw sparkline from circular buffer of floats (0..100)
// data[0] is oldest, data[len-1] is newest
inline void draw_sparkline(int x, int y, int w, int h,
                            float* data, int len, uint16_t line_color = COLOR_GREEN) {
    if (len < 2) return;
    spr.fillRect(x, y, w, h, COLOR_BG_PANEL);
    spr.drawRect(x, y, w, h, COLOR_GREY_DARK);

    for (int i = 1; i < len; i++) {
        int x0 = x + 1 + (i - 1) * (w - 2) / (len - 1);
        int x1 = x + 1 + i       * (w - 2) / (len - 1);
        int y0 = y + h - 1 - (int)(data[i - 1] * (h - 2) / 100.0f);
        int y1 = y + h - 1 - (int)(data[i]     * (h - 2) / 100.0f);
        y0 = constrain(y0, y + 1, y + h - 1);
        y1 = constrain(y1, y + 1, y + h - 1);
        spr.drawLine(x0, y0, x1, y1, line_color);
    }
}

// Animated "blinking" dot helper — returns current blink state based on millis
inline bool blink_state() {
    return (millis() % BLINK_PERIOD_MS) < (BLINK_PERIOD_MS / 2);
}

// Draw screen-transition wipe (vertical bar sweep left)
inline void draw_wipe(int progress_pct) {
    int x = SCREEN_WIDTH * progress_pct / 100;
    spr.fillRect(0, 0, x, SCREEN_HEIGHT, COLOR_BG);
}
