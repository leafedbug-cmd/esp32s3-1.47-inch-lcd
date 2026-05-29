#pragma once
// =============================================================================
// Blip Terminal - Configuration
// Board: Waveshare ESP32-S3-LCD-1.47
// Display: ST7789 172x320 (used in LANDSCAPE = 320w x 172h)
// =============================================================================

// WiFi credentials and other secrets live in secrets.h (gitignored)
#include "secrets.h"

// -----------------------------------------------------------------------------
// Backend API
// IP of the PC running the Python backend service
// -----------------------------------------------------------------------------
#define API_HOST        "192.168.1.100"
#define API_PORT        8765
#define API_ENDPOINT    "/api/status"
#define API_TIMEOUT_MS  5000

// -----------------------------------------------------------------------------
// Display hardware
// Pins are injected via build_flags in platformio.ini (-DTFT_MOSI=11 etc.)
// These are documented here for reference only.
// -----------------------------------------------------------------------------
// TFT_MOSI  = GP11  (GPIO 11)
// TFT_SCLK  = GP10  (GPIO 10)
// TFT_CS    = GP9   (GPIO 9)
// TFT_DC    = GP8   (GPIO 8)
// TFT_RST   = GP12  (GPIO 12)
// TFT_BL    = GPIO 46 (internal, not on header)
//
// TF Card (shared SPI bus):
// SD_MOSI   = GP11  (shared)
// SD_SCLK   = GP10  (shared)
// SD_MISO   = GP13  (GPIO 13)
// SD_CS     = GP7   (GPIO 7)  -- unused in this project

// -----------------------------------------------------------------------------
// RGB LED (internal WS2812 or similar)
// -----------------------------------------------------------------------------
#define RGB_LED_PIN     48
#define RGB_LED_COUNT   1

// -----------------------------------------------------------------------------
// Touch controller (CST816S via I2C)
// Set TOUCH_ENABLED to 1 when using the Type-B / touch-capable board variant.
// The standard ESP32-S3-LCD-1.47 has NO touch hardware — keep this 0.
//
// Waveshare Type-B pin mapping (I2C):
//   Touch SDA  = GP1  (GPIO 1)
//   Touch SCL  = GP2  (GPIO 2)
//   Touch INT  = GP3  (GPIO 3)  -- active-low interrupt
//   Touch RST  = GP4  (GPIO 4)
// -----------------------------------------------------------------------------
#define TOUCH_ENABLED       0       // 0 = no touch; 1 = CST816S present

#define TOUCH_SDA_PIN       1
#define TOUCH_SCL_PIN       2
#define TOUCH_INT_PIN       3
#define TOUCH_RST_PIN       4
#define TOUCH_I2C_ADDR      0x15    // CST816S default address
#define TOUCH_I2C_FREQ      400000  // 400 kHz

// Tap behaviour
#define TOUCH_LONG_PRESS_MS     500   // hold duration for long-press action
#define TOUCH_SWIPE_MIN_PX      40    // minimum swipe distance (display-relative px)

// -----------------------------------------------------------------------------
// Display dimensions (landscape orientation)
// -----------------------------------------------------------------------------
#define SCREEN_WIDTH    320
#define SCREEN_HEIGHT   172
#define SCREEN_ROTATION 1       // 0=portrait, 1=landscape

// -----------------------------------------------------------------------------
// Screen timing
// -----------------------------------------------------------------------------
#define SCREEN_COUNT          6
#define AUTO_ROTATE_INTERVAL  6000   // ms between auto-rotation
#define FETCH_INTERVAL        5000   // ms between API polls
#define CLOCK_TICK_INTERVAL   1000   // ms for clock/animation tick

// -----------------------------------------------------------------------------
// Display backlight
// -----------------------------------------------------------------------------
#define BACKLIGHT_PIN       46
#define BACKLIGHT_CHANNEL   0
#define BACKLIGHT_FREQ      5000
#define BACKLIGHT_RESOLUTION 8
#define BACKLIGHT_DEFAULT   200   // 0-255

// -----------------------------------------------------------------------------
// API data keys (must match backend JSON)
// -----------------------------------------------------------------------------
// System
#define KEY_CPU_PCT       "cpu"
#define KEY_CPU_TEMP      "cpu_temp"
#define KEY_RAM_PCT       "ram"
#define KEY_GPU_PCT       "gpu"
#define KEY_GPU_TEMP      "gpu_temp"
#define KEY_VRAM_PCT      "vram"
#define KEY_DISK_PCT      "disk"
#define KEY_NET_DOWN      "net_down_mbps"
#define KEY_NET_UP        "net_up_mbps"
// Blip AI
#define KEY_AI_MODEL      "ai_model"
#define KEY_AI_TPM        "ai_tpm"
#define KEY_AI_CONVO      "ai_conversations"
#define KEY_AI_LATENCY    "ai_latency_ms"
#define KEY_AI_UPTIME     "ai_uptime"
#define KEY_AI_MEM        "ai_mem_pct"
#define KEY_AI_STATUS     "ai_status"
// Internet
#define KEY_WEATHER       "weather"
#define KEY_WEATHER_ICON  "weather_icon"
#define KEY_EXTERNAL_IP   "external_ip"
#define KEY_LATENCY       "latency_ms"
#define KEY_DL_SPEED      "dl_mbps"
#define KEY_UL_SPEED      "ul_mbps"
// Discord
#define KEY_DC_MEMBERS    "dc_members"
#define KEY_DC_ONLINE     "dc_online"
#define KEY_DC_MSGS       "dc_msgs_today"
#define KEY_DC_VOICE      "dc_voice_users"
// OpenClaw
#define KEY_OC_AGENTS     "oc_agents"
#define KEY_OC_TASKS      "oc_tasks"
#define KEY_OC_TOOL       "oc_current_tool"
#define KEY_OC_QUEUE      "oc_queue_len"
#define KEY_OC_LAST_TASK  "oc_last_task"
#define KEY_OC_LOG        "oc_log"
// Network scan
#define KEY_WIFI_NETS     "wifi_networks"
#define KEY_BLE_DEVICES   "ble_devices"
#define KEY_LAN_DEVICES   "lan_devices"
#define KEY_PING          "ping_ms"
