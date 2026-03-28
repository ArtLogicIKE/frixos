# Frixos — Build, Flash & OTA Guide

## Prerequisites

- **ESP-IDF v6.0** installed and activated:
  ```bash
  source ~/.espressif/tools/activate_idf_v6.0.sh
  ```
- **`.env` file** in the project root (copy from `env.example`):
  ```bash
  cp env.example .env
  ```
  Edit `.env` and set:
  | Variable | Required | Description |
  |----------|----------|-------------|
  | `WEATHER_API_KEY` | Yes | [OpenWeatherMap](https://openweathermap.org/api) API key |
  | `ARTLOGIC_SSID` | No | Fallback WiFi SSID (used before provisioning portal) |
  | `ARTLOGIC_PASSWORD` | No | Fallback WiFi password |

  `CMakeLists.txt` reads `.env` and generates `main/include/config.h` automatically at build time.

---

## Build

```bash
idf.py build
```

Outputs:
- `build/frixos.bin` — firmware binary
- `build/spiffs.bin` — SPIFFS filesystem image (web UI + assets)
- `build/partition_table/partition-table.bin` — partition table

---

## Wired Flash (macOS)

### 1. Install the USB-to-UART driver

The ESP32 development board uses one of two USB bridge chips. Install the matching driver:

- **CP210x** (Silicon Labs) — [Download from silabs.com](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers)
- **CH340** — [Download from wch-ic.com](http://www.wch-ic.com/downloads/CH341SER_MAC_ZIP.html)

After installing, macOS may require approval in **System Settings → Privacy & Security**.

### 2. Find the serial port

Plug in the device via USB, then:

```bash
ls /dev/tty.usbserial-* /dev/cu.usbserial-*
```

Typical output: `/dev/tty.usbserial-0001` or `/dev/cu.SLAB_USBtoUART`

### 3. Flash and monitor

```bash
idf.py -p /dev/tty.usbserial-XXXX flash monitor
```

This single command:
1. Flashes the **firmware** (`build/frixos.bin`)
2. Flashes the **SPIFFS image** (`build/spiffs.bin`) — included automatically via `FLASH_IN_PROJECT`
3. Flashes the **partition table**
4. Opens a serial monitor (baud 115200); press `Ctrl-]` to exit

> **macOS permissions**: If the port is denied, go to **System Settings → Privacy & Security → Files and Folders** and grant your terminal app access, or run with `sudo`.

### Flash without monitor

```bash
idf.py -p /dev/tty.usbserial-XXXX flash
```

### Monitor only (after flashing)

```bash
idf.py -p /dev/tty.usbserial-XXXX monitor
```

---

## SPIFFS Assets

The `spiffs/` directory is packed into a 1.5 MB filesystem image and flashed to the `spiffs` partition (offset `0x670000`).

| Contents | Description |
|----------|-------------|
| `index.html`, `index.js`, `index.css` | Settings web portal (single-page app) |
| `language_*.json` | Translations — `en`, `de`, `fr`, `it`, `pt`, `sv`, `da`, `pl`, `es` |
| `timezone.txt` | Timezone database (12 KB) |
| `*.jpg` | Clock-face theme preview images (shown in web UI) |
| `files.txt` | Manifest used by the OTA server for incremental SPIFFS updates |
| `favicon.ico`, `logo.jpg`, `wifi-qr.jpg` | UI assets |

The SPIFFS image is built by this line in `CMakeLists.txt`:
```cmake
spiffs_create_partition_image(spiffs ./spiffs FLASH_IN_PROJECT)
```

`FLASH_IN_PROJECT` means the image is always included in `idf.py flash` — no separate step needed.

---

## OTA: Automatic Updates

The firmware checks for updates **every 8 hours** after WiFi connects.

**How it works:**
1. Sends a version-check GET request to `http://update.artlogic.gr:8080/latest` with device identity (MAC, firmware version, hostname, etc.)
2. If a newer version is available, downloads the SPIFFS file manifest and any changed asset files first (progress: 0–50%)
3. Downloads the firmware binary `revE{version}.bin` and writes it to the inactive OTA partition (progress: 50–100%)
4. Switches the boot partition and reboots

**Kill switch:** Automatic updates are controlled by the **"Firmware Updates"** toggle in the web UI settings (stored as `eeprom_update_firmware`). Disable it to prevent automatic updates.

**Status reporting:** After each attempt, the device reports success or failure back to the update server.

---

## OTA: Manual Upload

Upload a new firmware or SPIFFS file directly to the device over WiFi.

### Via web UI

1. Open `http://frixos.local` (or the device IP) in a browser
2. Go to the **Update** tab
3. Upload `build/frixos.bin`

The device flashes it to the inactive OTA slot and reboots after 5 seconds.

### Via curl

```bash
curl -X POST http://frixos.local/api/ota \
  -H "X-Filename: frixos.bin" \
  --data-binary @build/frixos.bin
```

**File routing:**
- Files with `.bin` extension → flashed as firmware to the inactive OTA slot → device reboots
- Any other file → written to `/spiffs/{filename}` (use for updating individual web assets)

Example to update a language file:
```bash
curl -X POST http://frixos.local/api/ota \
  -H "X-Filename: language_en.json" \
  --data-binary @spiffs/language_en.json
```

---

## Rollback

The firmware uses ESP-IDF's automatic rollback mechanism:

- After flashing and rebooting, the new firmware must successfully boot and call `esp_ota_mark_app_valid_cancel_rollback()` (done internally in `main/f-ota.c`)
- If the device crashes before reaching that call, the bootloader automatically reverts to the previous firmware slot on the next boot
- No user action is required — rollback is invisible and automatic

---

## Partition Layout

| Partition | Offset | Size | Description |
|-----------|--------|------|-------------|
| `nvs` | 0x9000 | 20 KB | Non-volatile settings storage |
| `otadata` | 0xE000 | 8 KB | Active OTA slot metadata |
| `app0` | 0x10000 | 3.19 MB | Primary firmware slot |
| `app1` | 0x340000 | 3.19 MB | Secondary firmware slot (OTA) |
| `spiffs` | 0x670000 | 1.5 MB | Web UI, assets, language files |
| `coredump` | 0x7F0000 | 64 KB | Crash dump storage |

Total: 8 MB flash.
