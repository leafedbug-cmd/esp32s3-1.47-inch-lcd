# Blip Terminal

A cyberpunk dashboard for the **Waveshare ESP32-S3-LCD-1.47** that displays live data from your PC, Blip AI (OpenClaw), Discord server, and the internet.

---

## Hardware

| Item | Spec |
|------|------|
| Board | Waveshare ESP32-S3-LCD-1.47 |
| Display | ST7789 1.47" 172×320 (landscape 320×172) |
| MCU | ESP32-S3 LX7 dual-core 240 MHz |
| Flash | 16 MB QIO |
| PSRAM | 8 MB OPI |

### Pin Reference

| Function | GPIO | Header |
|----------|------|--------|
| LCD MOSI | 11 | GP11 |
| LCD SCLK | 10 | GP10 |
| LCD CS | 9 | GP9 |
| LCD DC | 8 | GP8 |
| LCD RST | 12 | GP12 |
| LCD BL (backlight) | 46 | internal |
| SD MISO | 13 | GP13 |
| RGB LED | 48 | internal |
| Free GPIOs | 1–7 | GP1–GP7 |

---

## Project Structure

```
esp32s3-blip-terminal/
├── firmware/               ESP32-S3 PlatformIO project
│   ├── platformio.ini
│   ├── partitions_16MB.csv
│   └── src/
│       ├── main.cpp        Entry point, boot splash, screen loop
│       ├── config.h        All pins, WiFi, API URL, timing constants
│       ├── display/
│       │   ├── display_mgr.h   TFT_eSPI wrapper, draw helpers
│       │   └── theme.h         Color palette, layout constants
│       ├── data/
│       │   └── data_store.h    Shared state structs (updated by API client)
│       ├── screens/
│       │   ├── screen_base.h
│       │   ├── screen_blip.h       Screen 1 – Blip AI status
│       │   ├── screen_pc.h         Screen 2 – PC metrics
│       │   ├── screen_internet.h   Screen 3 – Internet / weather
│       │   ├── screen_discord.h    Screen 4 – Discord server
│       │   ├── screen_openclaw.h   Screen 5 – OpenClaw ops log
│       │   └── screen_network.h    Screen 6 – WiFi / BLE scan
│       └── net/
│           ├── wifi_mgr.h      WiFi + auto-reconnect
│           └── api_client.h    HTTP GET + ArduinoJson parser
└── backend/                Python 3.11+ service (runs on your PC)
    ├── main.py
    ├── requirements.txt
    ├── config.example.yaml
    ├── api/
    │   ├── server.py       FastAPI — GET /api/status
    │   └── state.py        AppState dataclass
    └── collectors/
        ├── system.py       psutil CPU/RAM/GPU/disk/network
        ├── weather.py      OpenWeatherMap
        ├── network.py      ping, external IP, LAN scan, speedtest
        ├── discord_collector.py  discord.py bot
        └── openclaw.py     OpenClaw / OpenWebUI REST API
```

---

## Quick Start

### 1 — Backend (PC)

```bash
cd backend
pip install -r requirements.txt

# Copy and edit the config
cp config.example.yaml config.yaml
# Edit config.yaml: set your WiFi SSID is not needed here,
# but set API_HOST in firmware/src/config.h to your PC's IP.
# Fill in Discord bot token, OpenWeatherMap key, etc.

python main.py
```

The API will be available at `http://<YOUR_PC_IP>:8765/api/status`.

### 2 — Firmware (ESP32)

1. Open `firmware/` in VS Code with PlatformIO.
2. Edit `src/config.h`:
   - Set `WIFI_SSID` and `WIFI_PASSWORD`
   - Set `API_HOST` to your PC's local IP address
3. Build and upload:
   ```
   pio run --target upload
   ```
4. If upload fails, hold **BOOT**, tap **RST**, release **BOOT**, then retry.

---

## Screens

| # | Screen | Key Data |
|---|--------|----------|
| 1 | Blip Status | AI model, TPM, latency, uptime, memory, online indicator |
| 2 | PC Status | CPU/RAM/GPU bars + sparklines + temps |
| 3 | Internet | Weather, external IP, ping, download/upload |
| 4 | Discord | Members, online, voice, messages today |
| 5 | OpenClaw Ops | Agents, tasks, current tool, scrolling log |
| 6 | Network Tools | WiFi scan (from ESP32), BLE, LAN hosts, ping |

Screens auto-rotate every **6 seconds** by default (`AUTO_ROTATE_INTERVAL` in `config.h`).

---

## Configuration

### Firmware `src/config.h`

| Constant | Default | Description |
|----------|---------|-------------|
| `WIFI_SSID` | `YOUR_SSID` | Your WiFi network name |
| `WIFI_PASSWORD` | `YOUR_PASSWORD` | Your WiFi password |
| `API_HOST` | `192.168.1.100` | PC running the backend |
| `API_PORT` | `8765` | Backend port |
| `AUTO_ROTATE_INTERVAL` | `6000` | ms between screen switches |
| `FETCH_INTERVAL` | `5000` | ms between API polls |
| `BACKLIGHT_DEFAULT` | `200` | Brightness 0–255 |

### Backend `config.yaml`

See `config.example.yaml` for all options. Key items:

```yaml
discord:
  token: "your-bot-token"
  guild_id: 123456789

weather:
  api_key: "your-openweathermap-key"
  city: "Your City"
  units: "imperial"   # or metric

openclaw:
  base_url: "http://localhost:8000"
```

---

## Discord Bot Setup

1. Create a bot at https://discord.com/developers/applications
2. Under **Bot** → enable **Server Members Intent**, **Presence Intent**, **Message Content Intent**
3. Copy the token into `config.yaml`
4. Invite the bot to your server with `bot` + `applications.commands` scopes and `Read Message History` permission

---

## Future Expansion (planned)

- Touch/swipe navigation (capacitive overlay mod)
- nRF24L01 diagnostics screen
- BLE active scanning from ESP32
- Home Assistant integration
- Push notification overlay
- AI-generated daily summary screensaver
- Voice activation via Blip

---

## GPIO Expansion

Free GPIOs on the header for future peripherals:

| Header Pin | GPIO | Suggested Use |
|-----------|------|--------------|
| GP1 | 1 | I2C SDA (sensors) |
| GP2 | 2 | I2C SCL (sensors) |
| GP3 | 3 | nRF24 CE |
| GP4 | 4 | nRF24 CSN |
| GP5 | 5 | nRF24 MOSI (SPI2) |
| GP6 | 6 | nRF24 MISO (SPI2) |
| GP7 | 7 | nRF24 SCK (SPI2) |
