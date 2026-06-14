# Frixos — Developer Guide

This guide is intended for developers who are **new to the project**. It covers the repository layout, how to set up your environment, how to build and flash the firmware, how to work on the web UI, and how to use the Home Assistant integration.

---

## Table of contents

1. [Project overview](#1-project-overview)
2. [Repository layout](#2-repository-layout)
3. [Prerequisites](#3-prerequisites)
4. [Environment setup](#4-environment-setup)
5. [Building the firmware](#5-building-the-firmware)
6. [Flashing the device](#6-flashing-the-device)
7. [Deploying the web UI (SPIFFS)](#7-deploying-the-web-ui-spiffs)
8. [Working on the web UI locally](#8-working-on-the-web-ui-locally)
9. [Home Assistant integration](#9-home-assistant-integration)
10. [Key configuration files](#10-key-configuration-files)
11. [Partition layout](#11-partition-layout)
12. [Tools & verification scripts](#12-tools--verification-scripts)
13. [Further reading](#13-further-reading)

---

## 1. Project overview

**Frixos** is an open-source **ESP32 firmware** for a WiFi-enabled daylight projection clock. It displays:

- Time (NTP-synced, timezone-aware, 12/24-hour, projected digits)
- Weather forecasts (met.no primary, OpenWeatherMap optional)
- Biometric data: blood glucose from Dexcom, Freestyle Libre, or Nightscout
- Stock quotes (Finnhub.io)
- Custom scrolling messages with live token replacement

The device is configured through an embedded web UI and can be monitored/controlled from **Home Assistant** via a custom integration.

**Hardware:**
- SoC: ESP32
- Display: ST7735S 128×128 LCD
- Light sensor: LTR303 (ambient light / auto-brightness)
- Flash: 8 MB

**Build system:** ESP-IDF 5.x (CMake-based)

---

## 2. Repository layout

```
frixos/
├── main/                   # Firmware source code (C)
│   ├── main.c              # Entry point — boot detection, subsystem init
│   ├── f-display.c/h       # LVGL display engine, digit sprites, font cache
│   ├── f-wifi.c/h          # WiFi connection, NTP, mDNS, OTA timer
│   ├── f-provisioning.c/h  # Captive portal (SoftAP), QR code
│   ├── f-settings.c/h      # HTTP server (/api/settings, /api/status, OTA)
│   ├── f-integrations.c/h  # Token replacement, Home Assistant REST fetch
│   ├── f-dexcom.c/h        # Dexcom Share API
│   ├── f-freestyle.c/h     # LibreLinkUp (Freestyle Libre)
│   ├── f-nightscout.c/h    # Nightscout self-hosted
│   ├── f-stocks.c/h        # Finnhub.io stock quotes
│   ├── f-weather.c/h       # Weather cache
│   ├── f-time.c/h          # NTP, timezone, formatting
│   ├── f-pwm.c/h           # LED brightness / PWM
│   ├── f-ota.c/h           # OTA firmware update
│   ├── f-json.c/h          # JSON parsing helpers
│   ├── f-dns.c/h           # DNS redirect (captive portal)
│   ├── f-membuffer.c/h     # Memory-efficient buffering
│   ├── ltr303.c/h          # Ambient light sensor driver
│   ├── moon.c/h            # Moon phase calculations
│   ├── frixos_*.c          # Bitmap fonts (8–14 px)
│   ├── include/frixos.h    # NVS parameter declarations (eeprom_* vars)
│   ├── include/config.h.in # Config template (generated → config.h)
│   └── CMakeLists.txt
│
├── components/
│   └── teriyakigod__esp_lcd_st7735/   # ST7735 LCD driver (vendored)
│
├── custom_components/
│   └── frixos/             # Home Assistant custom integration (Python)
│       ├── __init__.py     # Platform setup, config entry handler
│       ├── config_flow.py  # Add device dialog in HA
│       ├── coordinator.py  # Data coordinator (polls /api/* every 60s)
│       ├── const.py        # API parameter mappings (p00–p43)
│       ├── sensor.py       # HA sensors (light level, uptime, WiFi…)
│       ├── switch.py       # HA switches (12h mode, dark theme…)
│       ├── number.py       # HA number inputs (brightness, offsets…)
│       ├── select.py       # HA selectors (font, filter, timezone…)
│       └── text.py         # HA text inputs (scroll message, tokens…)
│
├── spiffs/                 # Embedded web UI (served from flash)
│   ├── index.html          # Single-page app markup
│   ├── index.js            # Core JS: settings, i18n, API calls
│   ├── index.css           # Responsive styling
│   ├── language_*.json     # Translations (EN, DE, FR, IT, PT, SV, DA, PL, ES)
│   └── timezone.txt        # IANA timezone list for the dropdown
│
├── tools/                  # Developer utilities (Python)
│   ├── mock_server.py      # FastAPI mock of /api/settings and /api/status
│   ├── verify_ui.py        # Playwright UI smoke tests
│   └── verify_final.py     # Final integration verification
│
├── scripts/
│   └── build.ps1           # PowerShell build/flash helper (Windows)
│
├── docs/                   # Knowledge base
│   ├── developer-guide.md  # This file
│   ├── kb-flashing-esp32-p1-pgm.md   # UART flashing via P1 PGM header
│   └── kb-user-font-guide.md         # Custom font creation guide
│
├── artwork/                # Paint.NET source files for UI graphics
│
├── CMakeLists.txt          # Root CMake: loads .env, generates config.h, registers SPIFFS
├── partitions.csv          # Custom flash partition table (8 MB)
├── env.example             # Template for the required .env file
└── requirements.txt        # Python dependencies for tools/
```

---

## 3. Prerequisites

| Tool | Version | Notes |
|------|---------|-------|
| **ESP-IDF** | 5.x | Must match the project's `idf_component.yml` |
| **Python** | 3.9+ | Bundled with ESP-IDF; also needed for tools/ |
| **CMake** | 3.16+ | Bundled with ESP-IDF |
| **USB-to-serial adapter** | — | For UART flashing (see [section 6](#6-flashing-the-device)) |

For the tools/ scripts:
```bash
pip install -r requirements.txt
```

---

## 4. Environment setup

### 4.1 Install ESP-IDF

Follow the [ESP-IDF Getting Started guide](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/index.html) for your OS. After installation, export the environment in every terminal session:

```bash
# Linux / macOS
. $HOME/esp/esp-idf/export.sh

# Windows (PowerShell)
$env:IDF_PATH = "C:\Espressif\frameworks\esp-idf"
. $env:IDF_PATH\export.ps1
```

### 4.2 Create the `.env` file

```bash
cp env.example .env
```

Edit `.env` and fill in the required values:

```bash
# Required for met.no weather (unique User-Agent is mandatory)
WEATHER_USER_AGENT=Frixos/1.0 <your-github-or-email>

# Optional — OpenWeatherMap API key (legacy; met.no is the primary source)
WEATHER_API_KEY=

# Optional — manufacturer fallback WiFi (leave empty if not needed)
ARTLOGIC_SSID=
ARTLOGIC_PASSWORD=
```

> The build system reads `.env` at CMake configure time and injects the values into a generated `main/config.h` header. **Never commit `.env`** — it is gitignored.

---

## 5. Building the firmware

### Using the PowerShell script (Windows)

```powershell
# Build only
.\scripts\build.ps1

# Build + flash
.\scripts\build.ps1 -Flash -Port COM3

# Build + flash + open serial monitor
.\scripts\build.ps1 -Flash -Monitor -Port COM3
```

### Using idf.py directly

```bash
# Configure the target (first time only)
idf.py set-target esp32

# Build
idf.py build
```

Build output is placed in `build/`. The relevant artefacts are:

| File | Purpose |
|------|---------|
| `build/frixos.bin` | Application binary |
| `build/bootloader/bootloader.bin` | Bootloader |
| `build/partition_table/partition-table.bin` | Custom partition table |
| `build/spiffs.bin` | SPIFFS image containing the web UI |

---

## 6. Flashing the device

### 6.1 Over-the-air (OTA) — recommended for updates

1. Open the device's web UI (`http://<hostname>.local` or its IP).
2. Navigate to the **Update** tab.
3. Upload `build/frixos.bin`.

The device reboots into the new firmware automatically. The SPIFFS partition is **not** overwritten by OTA; update the web UI separately if needed (see [section 7](#7-deploying-the-web-ui-spiffs)).

### 6.2 UART flashing — for factory/recovery flashing

Full first-time flashing writes the bootloader, partition table, application, and SPIFFS in a single command:

```bash
idf.py -p /dev/ttyUSB0 flash
```

On Windows:

```bash
idf.py -p COM3 flash
```

To also open the serial monitor immediately after:

```bash
idf.py -p /dev/ttyUSB0 flash monitor
```

**Boot mode:** The ESP32 must be placed in UART download mode before flashing. See [kb-flashing-esp32-p1-pgm.md](kb-flashing-esp32-p1-pgm.md) for the full wiring guide and boot mode procedure using the P1 PGM header.

### 6.3 Flashing individual partitions

```bash
# Application only
esptool.py -p /dev/ttyUSB0 write_flash 0x10000 build/frixos.bin

# SPIFFS only
esptool.py -p /dev/ttyUSB0 write_flash 0x670000 build/spiffs.bin
```

---

## 7. Deploying the web UI (SPIFFS)

The web UI lives in `spiffs/` and is packaged into a SPIFFS image during `idf.py build`. Changes to any file in `spiffs/` require rebuilding and reflashing the SPIFFS partition.

```bash
# Build (regenerates build/spiffs.bin)
idf.py build

# Flash SPIFFS partition only
idf.py -p /dev/ttyUSB0 spiffs-flash
# or manually:
esptool.py -p /dev/ttyUSB0 write_flash 0x670000 build/spiffs.bin
```

---

## 8. Working on the web UI locally

You can develop and test the web UI without hardware using the mock server:

```bash
# Install dependencies
pip install -r requirements.txt

# Start the mock API server (mimics /api/settings and /api/status)
python tools/mock_server.py
# → runs on http://localhost:8080

# Open spiffs/index.html in your browser pointed at the mock server
# (the UI reads the MOCK_API_URL from localStorage or defaults to the current host)
```

To run the automated UI smoke tests (requires Playwright):

```bash
python tools/verify_ui.py
```

---

## 9. Home Assistant integration

The `custom_components/frixos/` folder contains a **Home Assistant custom integration** that exposes all device settings as HA entities.

### Installation

1. Copy the `custom_components/frixos/` folder to your HA `config/custom_components/` directory.
2. Restart Home Assistant.
3. Go to **Settings → Devices & Services → Add Integration** and search for **Frixos**.
4. Enter the device hostname/IP, port (default `80`), and a display name.

### What it exposes

| HA Platform | Examples |
|------------|---------|
| **Sensor** | Light level (lux), uptime, WiFi RSSI, free heap |
| **Switch** | 12-hour format, dark theme, dim disabled, Fahrenheit |
| **Number** | Brightness (day/night), X/Y display offset, scroll delay, refresh interval |
| **Select** | Font (day/night), color filter, CGM source, timezone |
| **Text** | Scroll message, HA URL, HA token, hostname |

The coordinator polls `/api/settings` and `/api/status` on the device every 60 seconds.

---

## 10. Key configuration files

| File | Purpose |
|------|---------|
| `.env` | Secrets injected at build time (copy from `env.example`) |
| `main/config.h.in` | CMake template that becomes `main/config.h` |
| `main/include/frixos.h` | Declares all NVS-backed settings variables (`eeprom_*`) |
| `sdkconfig` | ESP-IDF Kconfig options (flash size, log level, etc.) |
| `partitions.csv` | Custom partition table |
| `main/idf_component.yml` | Component dependencies (LVGL, mDNS, esp_lvgl_port) |

---

## 11. Partition layout

The device uses a custom 8 MB partition table:

| Name | Type | Offset | Size | Description |
|------|------|--------|------|-------------|
| `nvs` | data/nvs | 0x9000 | 20 KB | Non-volatile storage (user settings) |
| `otadata` | data/ota | 0xE000 | 8 KB | OTA slot metadata |
| `app0` | app/ota_0 | 0x10000 | 3.2 MB | OTA slot 0 (active firmware) |
| `app1` | app/ota_1 | 0x340000 | 3.2 MB | OTA slot 1 (pending firmware) |
| `spiffs` | data/spiffs | 0x670000 | 1.5 MB | Web UI and static assets |
| `coredump` | data/coredump | 0x7F0000 | 64 KB | Crash dump storage |

---

## 12. Tools & verification scripts

| Script | Usage |
|--------|-------|
| `tools/mock_server.py` | `python tools/mock_server.py` — starts a FastAPI mock of the device API on port 8080 |
| `tools/verify_ui.py` | `python tools/verify_ui.py` — Playwright smoke tests for the web UI |
| `tools/verify_final.py` | `python tools/verify_final.py` — final integration verification |
| `scripts/build.ps1` | `.\scripts\build.ps1 [-Flash] [-Monitor] [-Port COMx]` — Windows build helper |

---

## 13. Further reading

- [kb-flashing-esp32-p1-pgm.md](kb-flashing-esp32-p1-pgm.md) — Detailed UART flashing guide (P1 PGM connector, wiring, boot mode)
- [kb-user-font-guide.md](kb-user-font-guide.md) — How to create and install custom fonts
- [ESP-IDF documentation](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/)
- [LVGL documentation](https://docs.lvgl.io/master/)
