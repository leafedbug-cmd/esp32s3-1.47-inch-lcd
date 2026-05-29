// =============================================================================
// Blip Terminal - Main Entry Point
// Waveshare ESP32-S3-LCD-1.47 | 320x172 Landscape
// Cyberpunk dashboard for Blip AI / BuggedLab ecosystem
// =============================================================================

#include <Arduino.h>
#include <TFT_eSPI.h>

#include "config.h"
#include "data/data_store.h"
#include "display/display_mgr.h"
#include "net/wifi_mgr.h"
#include "net/api_client.h"
#include "screens/screen_base.h"
#include "screens/screen_blip.h"
#include "screens/screen_pc.h"
#include "screens/screen_internet.h"
#include "screens/screen_discord.h"
#include "screens/screen_openclaw.h"
#include "screens/screen_network.h"
#include "input/touch_mgr.h"

// -----------------------------------------------------------------------------
// Globals
// -----------------------------------------------------------------------------
TFT_eSPI  tft;
TFT_eSprite spr = TFT_eSprite(&tft);

DataStore g_data;

static WiFiMgr    wifi_mgr;
static ApiClient  api_client;
static TouchMgr   touch_mgr;

// Screen instances
static ScreenBlip     scr_blip;
static ScreenPC       scr_pc;
static ScreenInternet scr_internet;
static ScreenDiscord  scr_discord;
static ScreenOpenClaw scr_openclaw;
static ScreenNetwork  scr_network;

static ScreenBase* screens[SCREEN_COUNT] = {
    &scr_blip,
    &scr_pc,
    &scr_internet,
    &scr_discord,
    &scr_openclaw,
    &scr_network,
};

static int     current_screen  = 0;
static bool    auto_rotate     = true;
static uint32_t last_rotate    = 0;
static uint32_t last_fetch     = 0;
static uint32_t last_clock     = 0;

// -----------------------------------------------------------------------------
// Boot splash screen
// -----------------------------------------------------------------------------
static void boot_splash() {
    display_clear();

    spr.setTextFont(FONT_MEDIUM);
    spr.setTextColor(COLOR_GREEN, COLOR_BG);
    spr.setTextDatum(MC_DATUM);
    spr.drawString("BLIP TERMINAL", SCREEN_WIDTH / 2, 50);

    spr.setTextFont(FONT_TINY);
    spr.setTextColor(COLOR_GREEN_DIM, COLOR_BG);
    spr.drawString("v" BLIP_TERMINAL_VERSION " | BUGGEDLAB.COM", SCREEN_WIDTH / 2, 70);

    spr.setTextColor(COLOR_GREY_DARK, COLOR_BG);
    spr.drawString("ESP32-S3 | 320x172 | TFT_eSPI", SCREEN_WIDTH / 2, 82);

    // Animated progress bar
    for (int i = 0; i <= 100; i += 2) {
        int bw = (SCREEN_WIDTH - 60) * i / 100;
        spr.fillRect(30, 100, SCREEN_WIDTH - 60, 10, COLOR_BG_PANEL);
        spr.drawRect(30, 100, SCREEN_WIDTH - 60, 10, COLOR_GREEN_DARK);
        spr.fillRect(31, 101, bw, 8, COLOR_GREEN);

        spr.setTextFont(FONT_TINY);
        spr.setTextColor(COLOR_GREEN_DIM, COLOR_BG);
        spr.setTextDatum(MC_DATUM);

        char pct_buf[8];
        snprintf(pct_buf, sizeof(pct_buf), "%d%%", i);
        spr.drawString(pct_buf, SCREEN_WIDTH / 2, 116);

        // Status messages
        if (i < 30) {
            spr.setTextColor(COLOR_GREY_DIM, COLOR_BG);
            spr.drawString("INIT DISPLAY...", SCREEN_WIDTH / 2, 130);
        } else if (i < 60) {
            spr.drawString("CONNECTING WIFI...", SCREEN_WIDTH / 2, 130);
        } else if (i < 90) {
            spr.drawString("FETCHING STATUS...", SCREEN_WIDTH / 2, 130);
        } else {
            spr.setTextColor(COLOR_GREEN, COLOR_BG);
            spr.drawString("READY", SCREEN_WIDTH / 2, 130);
        }

        display_flush();
        delay(20);
    }
    delay(400);
}

// -----------------------------------------------------------------------------
// Switch to a screen index
// -----------------------------------------------------------------------------
static void switch_screen(int idx) {
    screens[current_screen]->on_exit();
    current_screen = (idx + SCREEN_COUNT) % SCREEN_COUNT;
    screens[current_screen]->on_enter();
    last_rotate = millis();
}

// -----------------------------------------------------------------------------
// setup
// -----------------------------------------------------------------------------
void setup() {
    Serial.begin(115200);
    Serial.println("[BLIP] Boot start");

    // Display
    display_init();
    boot_splash();

    // WiFi
    wifi_mgr.begin();

    // Initial data fetch
    api_client.fetch();

    // Touch controller (no-op when TOUCH_ENABLED=0)
    touch_mgr.begin();

    // Enter first screen
    screens[current_screen]->on_enter();
    last_rotate = millis();
    last_fetch  = millis();

    Serial.println("[BLIP] Boot complete");
}

// -----------------------------------------------------------------------------
// loop
// -----------------------------------------------------------------------------
void loop() {
    uint32_t now = millis();
    g_data.uptime_ms = now;

    // WiFi health check
    wifi_mgr.tick();

    // Touch input
    {
        TouchEvent te;
        if (touch_mgr.poll(te)) {
            switch (te.gesture) {
                case GESTURE_SINGLE_TAP:
                case GESTURE_SWIPE_LEFT:
                    // Advance to next screen; pause auto-rotate for one cycle
                    switch_screen(current_screen + 1);
                    last_rotate = millis();
                    break;
                case GESTURE_SWIPE_RIGHT:
                    // Go to previous screen
                    switch_screen(current_screen - 1);
                    last_rotate = millis();
                    break;
                case GESTURE_LONG_PRESS:
                    // Toggle auto-rotate
                    auto_rotate = !auto_rotate;
                    Serial.printf("[TOUCH] auto-rotate %s\n", auto_rotate ? "ON" : "OFF");
                    break;
                case GESTURE_DOUBLE_TAP:
                    // Return to first screen
                    switch_screen(0);
                    break;
                default:
                    break;
            }
        }
    }

    // Periodic API fetch
    if (now - last_fetch >= FETCH_INTERVAL) {
        last_fetch = now;
        api_client.fetch();
    }

    // Auto screen rotation
    if (auto_rotate && now - last_rotate >= AUTO_ROTATE_INTERVAL) {
        switch_screen(current_screen + 1);
    }

    // Draw current screen
    screens[current_screen]->draw();

    // Small yield to prevent watchdog trips during heavy drawing
    yield();
}
