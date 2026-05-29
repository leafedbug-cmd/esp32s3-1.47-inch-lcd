#pragma once
// =============================================================================
// Blip Terminal - Cyberpunk / BuggedLab Theme
// 320x172 landscape display
// =============================================================================

// -----------------------------------------------------------------------------
// Color palette (RGB565)
// -----------------------------------------------------------------------------
#define COLOR_BG            0x0000  // True black background
#define COLOR_BG_PANEL      0x0821  // Very dark green-tinted panel
#define COLOR_BG_HEADER     0x0841  // Dark header stripe

#define COLOR_GREEN         0x07E0  // Bright green - primary accent
#define COLOR_GREEN_DIM     0x0380  // Dim green - secondary text
#define COLOR_GREEN_DARK    0x01A0  // Dark green - inactive/borders

#define COLOR_ORANGE        0xFD20  // Orange - warning / highlight
#define COLOR_ORANGE_DIM    0xA980  // Dim orange

#define COLOR_CYAN          0x07FF  // Cyan - links / values
#define COLOR_CYAN_DIM      0x0398  // Dim cyan

#define COLOR_WHITE         0xFFFF
#define COLOR_GREY          0x7BEF  // Light grey
#define COLOR_GREY_DIM      0x39E7  // Mid grey
#define COLOR_GREY_DARK     0x18C3  // Dark grey - borders

#define COLOR_RED           0xF800  // Error / critical
#define COLOR_RED_DIM       0x7800  // Warning

#define COLOR_YELLOW        0xFFE0  // Caution
#define COLOR_PURPLE        0x781F  // Blip accent

// Status indicator colors
#define COLOR_STATUS_OK     COLOR_GREEN
#define COLOR_STATUS_WARN   COLOR_ORANGE
#define COLOR_STATUS_ERR    COLOR_RED
#define COLOR_STATUS_OFF    COLOR_GREY_DIM

// -----------------------------------------------------------------------------
// Layout constants (320x172 landscape)
// -----------------------------------------------------------------------------
#define HEADER_H            18      // Top header bar height
#define FOOTER_H            14      // Bottom status bar height
#define CONTENT_Y           (HEADER_H + 1)
#define CONTENT_H           (SCREEN_HEIGHT - HEADER_H - FOOTER_H - 2)
#define CONTENT_X           2
#define CONTENT_W           (SCREEN_WIDTH - 4)

// Column layouts
#define COL1_X              2
#define COL2_X              108
#define COL3_X              214
#define COL_W               100
#define COL_W_HALF          48

// Bar graph dimensions
#define BAR_H               8
#define BAR_FULL_W          90
#define SPARKLINE_H         24
#define SPARKLINE_W         80
#define SPARKLINE_POINTS    40

// -----------------------------------------------------------------------------
// Font sizes (using TFT_eSPI built-in fonts)
// Font 1 = 8px, Font 2 = 16px, Font 4 = 26px, Font 6 = 48px, Font 7 = 7-seg
// -----------------------------------------------------------------------------
#define FONT_TINY           1
#define FONT_SMALL          2
#define FONT_MEDIUM         4
#define FONT_LARGE          6
#define FONT_7SEG           7

// -----------------------------------------------------------------------------
// Animation
// -----------------------------------------------------------------------------
#define BLINK_PERIOD_MS     600
#define PULSE_PERIOD_MS     1200
#define SCROLL_SPEED_PX     1       // pixels per tick for scrolling text
