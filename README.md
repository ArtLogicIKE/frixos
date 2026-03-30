# Frixos

**Daylight Projection Clock with Smart Home Integration and Biometric Monitoring**

**[Frixos Store → buyfrixos.com](https://buyfrixos.com)**

Frixos is an open-source firmware for the Frixos daylight projection clock—a WiFi-enabled device that projects time, weather, and biometric data onto any surface. Built on the ESP-IDF framework for ESP32, it features a web-based provisioning portal, multilingual settings UI, and integrations with popular smart home and health platforms.

---

## Project Overview

Frixos firmware powers projection clocks that combine:

- **Time display** — Large projected digits with optional 12/24-hour format, timezone support, and NTP synchronization
- **Weather** — OpenWeatherMap integration for current conditions and forecasts
- **Biometric monitoring** — Continuous glucose monitoring via Dexcom, Freestyle Libre, or Nightscout
- **Smart home** — Home Assistant integration for custom entity tokens in scroll messages; [custom HA component](https://github.com/ArtLogicIKE/frixos-ha-integration) for full device control
- **Stock quotes** — Finnhub.io integration for real-time market data

The device uses an ST7735S LCD (128×128) as the projection source, an LTR303 ambient light sensor for automatic day/night dimming, and LVGL for the display engine. Settings are stored in NVS and served via a captive-portal web UI when the device is in provisioning mode.

*Frixos projection clocks — [Obelisk](https://buyfrixos.com/product/obelisk-stylish-daylight-projection-clock/), [Monolith](https://buyfrixos.com/product/monolith-its-full-of-time/), [HaloTime](https://buyfrixos.com/shop/) — available at [buyfrixos.com](https://buyfrixos.com)*

> **Tip:** To add product images to this README, right-click images on [buyfrixos.com](https://buyfrixos.com) → "Copy image address" and use: `![Description](image-url)`

---

## Provisioning Portal

When the device has no stored WiFi credentials or cannot connect, it starts a **softAP provisioning portal**:

1. **SoftAP mode** — Creates an open access point named `frixos` (no password)
2. **Captive portal** — A DNS server redirects all HTTP requests to the device’s IP (typically 192.168.4.1)
3. **QR code** — The LCD shows a QR code; scanning it opens the provisioning page on a phone
4. **Web UI** — Users enter WiFi SSID, password, hostname, and other connection settings
5. **Timeout** — After 10 minutes without a successful connection, the device restarts

The provisioning flow is implemented in `main/f-provisioning.c`:

- Tries stored credentials first
- Falls back to a manufacturer network if `ARTLOGIC_SSID` and `ARTLOGIC_PASSWORD` are set in `.env`
- Starts softAP + DNS server + HTTP server for the captive portal
- Uses `main/f-settings.c` for the HTTP server and settings endpoints

Connection settings (SSID, password, hostname) are saved to NVS and trigger a device restart on change.

---

## Multilingual Settings UI

The settings interface is a **single-page web app** served from SPIFFS (`spiffs/`) with full internationalization:

| Language | Code |
|----------|------|
| English  | `en` |
| German   | `de` |
| French   | `fr` |
| Italian  | `it` |
| Portuguese | `pt` |
| Swedish  | `sv` |
| Danish   | `da` |
| Polish   | `pl` |
| Spanish  | `es` |

- **Translation files** — `spiffs/language_*.json` for each language
- **UI toggle** — Language selector in the header; preference is stored in `eeprom_language`
- **Dark/light theme** — Theme toggle for the web UI
- **Sections** — Settings, Advanced, Integrations, Status, Update, Restart

The UI is built with vanilla HTML/CSS/JS, uses `data-i18n` attributes for translation keys, and loads the appropriate JSON based on the selected language.

---

## Display Engine & Font Caching

### Display Architecture

The display engine (`main/f-display.c`) uses:

- **LVGL** — Light and Versatile Graphics Library for rendering
- **ST7735S** — 128×128 SPI LCD with double buffering (8-line height) to reduce tearing
- **Digit sprites** — Time digits are rendered from a sprite sheet (18×36 px per digit) via `lv_image_set_offset_x()` for fast updates
- **Theme support** — Day/night fonts, weather icons, moon phases, and color filters are theme-specific (e.g. `default-`, `nixie-`, `ficasso-`)

### Font Caching

Two caching mechanisms improve text rendering performance:

1. **LVGL glyph cache** (`lv_font_fmt_txt_glyph_cache_t`)  
   Used by custom bitmap fonts (e.g. `frixos_8`, `frixos_9`, `frixos_11` in `main/frixos_*.c`). LVGL’s format-text font driver caches glyph bitmaps to avoid repeated lookups when drawing labels.

2. **Character width cache**  
   A 512-entry hash-style cache in `f-display.c` stores character widths for Unicode code points. It pre-fills ASCII (0–127) and common symbols (e.g. °) and caches additional code points on demand. Used for substring rendering and scroll positioning. Invalidated when the message font changes.

### Message Scrolling

- Messages shorter than 88 px are **centered**; longer messages **scroll** horizontally
- Scroll speed and delay are configurable
- Supports **tokens** (e.g. `[HA:sensor.temp:state]`, `[GLUCOSE]`, `[STOCK:AAPL]`) that are replaced with live data from integrations

---

## Integrations

Frixos supports six integration types, configured via the Integrations section of the web UI.

### Home Assistant (built-in)

- **Source** — Your [Home Assistant](https://www.home-assistant.io/) instance REST API
- **Data** — Entity states fetched and displayed in the scrolling message via tokens
- **Config** — Home Assistant URL, long-lived access token, refresh interval (minutes)
- **Token format** — `[HA:entity_id:path]` (e.g. `[HA:sensor.temperature:state]`, `[HA:light.living:attributes.brightness]`)
- **Files** — `main/f-integrations.c` (HA entity fetch, `parse_HA_entities`)

The device polls Home Assistant at the configured interval and replaces tokens in the scroll message with live entity values. Use the JSON path to extract nested attributes (default `state` for the main value).

### Freestyle Libre (LibreLinkUp)

- **Source** — Abbott LibreLinkUp API
- **Data** — Continuous glucose readings and trend arrows
- **Config** — Patient ID, region (US/Japan/RoW), refresh interval
- **Files** — `main/f-freestyle.c`, `main/include/f-freestyle.h`

### Dexcom

- **Source** — Dexcom Share API (`share2.dexcom.com`, `share2.dexcom.jp`, `shareous1.dexcom.com`)
- **Data** — Glucose values and trend arrows
- **Config** — Username, password, region
- **Files** — `main/f-dexcom.c`, `main/include/f-dexcom.h`

### Nightscout

- **Source** — Self-hosted Nightscout instance
- **Data** — Glucose and trend from `/api/v1/entries/current`
- **Config** — Nightscout URL, API secret
- **Files** — `main/f-nightscout.c`, `main/include/f-nightscout.h`

*Note: Dexcom, Freestyle, and Nightscout are mutually exclusive; only one CGM source is active at a time.*

### Open Weather

- **Source** — [OpenWeatherMap](https://openweathermap.org/) (`api.openweathermap.org`)
- **Data** — Current weather, forecast high/low, icons, sunrise/sunset
- **Config** — Lat/lon from location settings; API key in firmware
- **Files** — `main/f-wifi.c` (weather fetch logic)

### Finnhub.io

- **Source** — [Finnhub](https://finnhub.io/) stock API
- **Data** — Real-time stock quotes for symbols in scroll message tokens
- **Config** — Finnhub API key in Integrations
- **Files** — `main/f-stocks.c`, `main/include/f-stocks.h`

Token format: `[STOCK:AAPL]` for symbol `AAPL`.

---

## Home Assistant Integration

A [custom Home Assistant integration](https://github.com/ArtLogicIKE/frixos-ha-integration) lets you control and monitor Frixos devices directly from Home Assistant. **Requires Firmware 62 or later.**

### Features

- **Full settings control** — Access and modify device settings through Home Assistant entities (switches, numbers, selects, text inputs, color pickers)
- **Real-time monitoring** — Sensors for light level (lux), uptime, free heap memory
- **Automatic updates** — Settings and sensor data refreshed every 60 seconds
- **Automations** — Use Frixos entities in automations (e.g. adjust brightness based on light level)

### Installation

1. Install the custom component: [ArtLogicIKE/frixos-ha-integration](https://github.com/ArtLogicIKE/frixos-ha-integration)
2. Add the integration in Home Assistant: **Settings → Devices & Services → Add Integration** → search for "Frixos"
3. Enter your device hostname (e.g. `frixos.local`) or IP address

The integration uses the device's `/api/settings` and `/api/status` endpoints. For manual installation, HACS support, and full entity documentation, see the [integration repository](https://github.com/ArtLogicIKE/frixos-ha-integration).

---

## Build & Flash

See **[docs/BUILD.md](docs/BUILD.md)** for local, Docker, and Podman build instructions.

See **[docs/FLASH.md](docs/FLASH.md)** for flashing instructions and a description of build artifacts.

See **[docs/PROGRAMMING.md](docs/PROGRAMMING.md)** for the P1 UART programming header pinout, connector parts, and board photo.

**Before building:** Copy `env.example` to `.env` and configure. The `.env` file is gitignored and will not be published.

| Variable | Required | Description |
|----------|----------|--------------|
| `WEATHER_API_KEY` | Yes (for weather) | OpenWeatherMap API key from [openweathermap.org/api](https://openweathermap.org/api) |
| `ARTLOGIC_SSID` | No | Manufacturer fallback WiFi SSID (leave empty to skip) |
| `ARTLOGIC_PASSWORD` | No | Manufacturer fallback WiFi password |

### Key Configuration (menuconfig)

- **Flash size** — 8 MB
- **Partition table** — Custom
- **SPIFFS** — Mount point `S:` (drive letter 83), working dir `/spiffs`
- **Bluetooth** — NimBLE host only
- **WiFi** — Software coexistence disabled

---

## Project Structure

```
frixos/
├── env.example          # Template for .env (copy to .env; API keys, manufacturer fallback WiFi)
├── main/
│   ├── f-display.c       # Display engine, fonts, scrolling
│   ├── f-provisioning.c  # WiFi provisioning & captive portal
│   ├── f-settings.c      # HTTP server, settings API
│   ├── f-integrations.c  # Integration orchestration
│   ├── f-dexcom.c        # Dexcom Share API
│   ├── f-freestyle.c     # Freestyle Libre API
│   ├── f-nightscout.c    # Nightscout API
│   ├── f-stocks.c        # Finnhub stock API
│   ├── f-wifi.c          # WiFi, weather, NTP
│   └── frixos_*.c       # Custom bitmap fonts
├── spiffs/
│   ├── index.html        # Settings UI
│   ├── index.js          # UI logic, i18n
│   ├── language_*.json   # Translations
│   └── *.jpg             # Themes, logos, icons
├── docs/
│   ├── BUILD.md          # Local, Docker, and Podman build instructions
│   ├── FLASH.md          # Flashing instructions and build artifact reference
│   └── PROGRAMMING.md    # P1 UART header pinout and board programming guide
└── README.md
```

---

## Links

- **Store** — [buyfrixos.com](https://buyfrixos.com)
- **Shop** — [buyfrixos.com/shop](https://buyfrixos.com/shop/)
- **Products** — [Obelisk](https://buyfrixos.com/product/obelisk-stylish-daylight-projection-clock/), [Monolith](https://buyfrixos.com/product/monolith-its-full-of-time/)
- **Home Assistant integration** — [ArtLogicIKE/frixos-ha-integration](https://github.com/ArtLogicIKE/frixos-ha-integration)

---

## License

This project is **free to use and modify for personal use**. Forks are allowed.

**Commercial use requires a license from ArtLogic.** Any use intended to generate revenue is considered commercial use, including but not limited to:

- Selling the software
- Selling devices that include or run this software
- Selling installation, configuration, or support services for this software

Interested parties should contact [licensing@artlogic.gr](mailto:licensing@artlogic.gr) to obtain a commercial license.

See [license.free.txt](license.free.txt) and [license.commercial.txt](license.commercial.txt) for full details.
