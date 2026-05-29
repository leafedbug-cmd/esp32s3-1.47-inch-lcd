#pragma once
// =============================================================================
// Blip Terminal - Touch Manager
// Driver for CST816S capacitive touch controller (I2C)
// Used on: Waveshare ESP32-S3-LCD-1.47 TYPE-B (touch variant)
//
// To enable: set TOUCH_ENABLED 1 in config.h
// Board pins: SDA=GP1, SCL=GP2, INT=GP3, RST=GP4
//
// Actions dispatched:
//   tap / swipe-left  -> next screen, pause auto-rotate briefly
//   swipe-right       -> previous screen, pause auto-rotate briefly
//   long-press        -> toggle auto-rotate on/off
// =============================================================================

#include "config.h"

// Gesture codes -- always defined so main.cpp compiles regardless of TOUCH_ENABLED
enum TouchGesture : uint8_t {
    GESTURE_NONE        = 0x00,
    GESTURE_SWIPE_UP    = 0x01,
    GESTURE_SWIPE_DOWN  = 0x02,
    GESTURE_SWIPE_LEFT  = 0x03,
    GESTURE_SWIPE_RIGHT = 0x04,
    GESTURE_SINGLE_TAP  = 0x05,
    GESTURE_DOUBLE_TAP  = 0x0B,
    GESTURE_LONG_PRESS  = 0x0C,
};

struct TouchEvent {
    TouchGesture gesture = GESTURE_NONE;
    uint16_t     x       = 0;
    uint16_t     y       = 0;
    uint8_t      fingers = 0;
    bool         valid   = false;
};

#if TOUCH_ENABLED
#include <Wire.h>

// CST816S registers
static constexpr uint8_t CST_REG_GESTURE   = 0x01;
static constexpr uint8_t CST_REG_FINGERS   = 0x02;
static constexpr uint8_t CST_REG_XPOS_H    = 0x03;
static constexpr uint8_t CST_REG_XPOS_L    = 0x04;
static constexpr uint8_t CST_REG_YPOS_H    = 0x05;
static constexpr uint8_t CST_REG_YPOS_L    = 0x06;
static constexpr uint8_t CST_REG_CHIP_ID   = 0xA7;
static constexpr uint8_t CST_REG_IRQ_CTL   = 0xFA;

// -----------------------------------------------------------------------------
class TouchMgr {
public:
    bool begin() {
        pinMode(TOUCH_RST_PIN, OUTPUT);
        pinMode(TOUCH_INT_PIN, INPUT_PULLUP);
        digitalWrite(TOUCH_RST_PIN, LOW);
        delay(10);
        digitalWrite(TOUCH_RST_PIN, HIGH);
        delay(50);
        Wire.begin(TOUCH_SDA_PIN, TOUCH_SCL_PIN, TOUCH_I2C_FREQ);
        uint8_t chip_id = _read_reg(CST_REG_CHIP_ID);
        if (chip_id == 0x00 || chip_id == 0xFF) {
            Serial.printf("[TOUCH] CST816S not found (id=0x%02X)\n", chip_id);
            _present = false;
            return false;
        }
        Serial.printf("[TOUCH] CST816S detected (id=0x%02X)\n", chip_id);
        _write_reg(CST_REG_IRQ_CTL, 0x71);
        _present = true;
        return true;
    }

    bool poll(TouchEvent& evt) {
        if (!_present) return false;
        if (digitalRead(TOUCH_INT_PIN) == HIGH) return false;
        Wire.beginTransmission(TOUCH_I2C_ADDR);
        Wire.write(CST_REG_GESTURE);
        if (Wire.endTransmission(false) != 0) return false;
        if (Wire.requestFrom((uint8_t)TOUCH_I2C_ADDR, (uint8_t)6) != 6) return false;
        uint8_t gesture = Wire.read();
        uint8_t fingers = Wire.read();
        uint8_t xh      = Wire.read();
        uint8_t xl      = Wire.read();
        uint8_t yh      = Wire.read();
        uint8_t yl      = Wire.read();
        if (fingers == 0) return false;
        // CST816S portrait coords (172w x 320h) -> landscape (320w x 172h)
        uint16_t raw_x = ((xh & 0x0F) << 8) | xl;
        uint16_t raw_y = ((yh & 0x0F) << 8) | yl;
        evt.x       = raw_y;
        evt.y       = (SCREEN_HEIGHT - 1) - raw_x;
        evt.fingers = fingers;
        evt.gesture = static_cast<TouchGesture>(gesture);
        evt.valid   = true;
        return true;
    }

    bool is_present() const { return _present; }

private:
    bool _present = false;

    uint8_t _read_reg(uint8_t reg) {
        Wire.beginTransmission(TOUCH_I2C_ADDR);
        Wire.write(reg);
        if (Wire.endTransmission(false) != 0) return 0;
        Wire.requestFrom((uint8_t)TOUCH_I2C_ADDR, (uint8_t)1);
        return Wire.available() ? Wire.read() : 0;
    }

    void _write_reg(uint8_t reg, uint8_t val) {
        Wire.beginTransmission(TOUCH_I2C_ADDR);
        Wire.write(reg);
        Wire.write(val);
        Wire.endTransmission();
    }
};

#else  // !TOUCH_ENABLED

class TouchMgr {
public:
    bool begin()              { return false; }
    bool poll(TouchEvent&)    { return false; }
    bool is_present() const   { return false; }
};

#endif  // TOUCH_ENABLED
