# Frixos HTTP API — settings and screen layout

This guide is for developers who want to **set every firmware parameter with a direct HTTP call**, without opening the web UI. It also covers the **binary screen-layout blob**: why it exists, how to patch it, and how POST applies it.

There is **no authentication**. Anyone on the same network can read and change settings.

Source of truth is firmware (`main/f-settings.c`, `main/include/frixos.h`, `main/include/f-screen-layout-bin.h`). The Home Assistant mapping in `custom_components/frixos/const.py` stops around `p43` and is incomplete.

---

## Table of contents

1. [Host and curl](#1-host-and-curl)
2. [Why the screen layout is a binary blob](#2-why-the-screen-layout-is-a-binary-blob)
3. [Endpoints at a glance](#3-endpoints-at-a-glance)
4. [`GET` / `POST /api/settings`](#4-get--post-apisettings)
5. [Parameter reference](#5-parameter-reference)
6. [Screen layout binary (`/api/screen`)](#6-screen-layout-binary-apiscreen)
7. [Apply flow](#7-apply-flow)
8. [Python: patch scroll text and a text widget](#8-python-patch-scroll-text-and-the-first-text-widget)
9. [Other HTTP APIs](#9-other-http-apis)
10. [Reboot, apply, and gotchas](#10-reboot-apply-and-gotchas)

---

## 1. Host and curl

Default hostname is `frixos`. mDNS is typically:

```text
http://frixos.local
```

On a LAN you can also use the device IP (lab/test devices have used `192.168.2.129`).

On **Windows**, use `curl.exe`. PowerShell aliases `curl` to `Invoke-WebRequest`, which is not compatible with these examples.

```powershell
# Windows
$HOST = "http://frixos.local"   # or http://192.168.2.129
curl.exe -s "$HOST/api/status"
```

```bash
# POSIX (Linux / macOS)
HOST=http://frixos.local
curl -s "$HOST/api/status"
```

The rest of this document writes `curl.exe`. Drop `.exe` on POSIX.

---

## 2. Why the screen layout is a binary blob

`/api/settings` is JSON (`p00`…`p66` and `tz_iana`). That path is small enough for the ESP32 HTTP stack, cJSON, and the shared 4096-byte receive buffer (`HTTP_BUFFER_SIZE`).

The **screen layout is not**. A JSON document for ~15 widgets and 8 text widgets × 2 profiles, 512-byte scroll strings, text widgets, and graph config is too large for the device to receive and parse on `/api/screen` (heap, parse buffers, `httpd` recv). Firmware therefore speaks a **packed little-endian blob**.

JSON exists **only in the browser**. `spiffs/js/screen-editor.js` decodes the blob to an editor object and encodes it back. Firmware encode/decode is `main/f-screen-layout-bin.c`. Comment in `f-settings.c`:

```text
/api/screen (compact binary wire format; JSON is used only in the browser)
```

Wire MIME type:

```text
application/vnd.frixos.screen-layout+1
```

POST also accepts `application/octet-stream`. Do **not** POST JSON to `/api/screen`.

The text of the text widgets lives **only** in this blob. It is not an `/api/settings` key.

---

## 3. Endpoints at a glance

| Method | Path | Body | Notes |
|--------|------|------|--------|
| GET | `/api/settings` | — | JSON. Optional `?group=` / `?params=` |
| POST | `/api/settings` | JSON | Partial update. `Content-Type: application/json` |
| GET | `/api/screen` | — | **5634-byte** binary blob |
| POST | `/api/screen` | binary, exactly 5634 bytes (or 1654 bytes: layout without the text widgets) | Persists and applies layout |
| GET | `/api/status` | — | Device status JSON. `?logs=1` adds logs |
| GET | `/api/locate` | — | IP geolocation (`lat`, `lon`, `city`, `iana`, `posix`) |
| GET | `/api/timezone` | — | `?location=Europe/Athens` or `?lat=&lon=` |
| POST | `/api/ota` | firmware or SPIFFS file | Multipart or raw + `X-Filename` |
| POST | `/api/ota/reinstall` | empty | Reinstall from stored firmware |
| POST | `/api/reset` | empty | Reboot |
| GET | `/api/wifi/scan` | — | Start scan |
| GET | `/api/wifi/status` | — | Scan results |
| GET | `/api/files` | — | List SPIFFS files |
| POST | `/api/files/delete` | JSON `{ "files": ["…"] }` | Delete |
| POST | `/api/files/rename` | JSON `{ "oldName", "newName" }` | Rename |

Handlers may return **503** with JSON if the HTTP mutex is held (OTA or an integration fetch), wait ~15s and retry.

---

## 4. `GET` / `POST /api/settings`

### GET

Full dump:

```powershell
curl.exe -s "$HOST/api/settings"
```

Subset by keys (`tz_iana` is included when `p19` is in the mask):

```powershell
curl.exe -s "$HOST/api/settings?params=p00,p16,p23"
```

Named groups (OR’d with `params` if both are set):

| `group` | Keys |
|---------|------|
| `theme` | `p40`, `p41` |
| `settings` | `p00`, `p03`, `p09`, `p16`, `p34`–`p37`, `p39`, `p60`–`p63` |
| `advanced` | `p01`–`p24`, `p42`, `p43`, `p46`, `p47`, `p50`, `p55`, `p56` |
| `integrations` | `p25`–`p33`, `p44`, `p45`, `p51`–`p54`, `p64`–`p66` |

```powershell
curl.exe -s "$HOST/api/settings?group=integrations"
curl.exe -s "$HOST/api/settings?group=settings&params=p03,p09,p16"
```

### POST

- Body is a JSON **object**. Send only the keys you want to change.
- Header: `Content-Type: application/json` (firmware parses the body as JSON; send this anyway).
- Max body ≈ **4095 bytes** (shared HTTP buffer).
- Success: `{"status":"ok","message":"Settings saved"}`  
  or, if a network setting changed: `{"status":"ok","message":"Settings saved, rebooting..."}` then reboot after ~5s.
- Validation failure: HTTP 400, `{"status":"error","message":"Invalid …"}`.

```powershell
curl.exe -s -X POST "$HOST/api/settings" `
  -H "Content-Type: application/json" `
  --data-binary '{"p16":"Hello from HTTP"}'
```

```bash
curl -s -X POST "$HOST/api/settings" \
  -H "Content-Type: application/json" \
  --data-binary '{"p16":"Hello from HTTP"}'
```

`p16` max **511** characters (`SCROLL_MSG_LENGTH` 512 including NUL). It writes `eeprom_message` and calls `screen_layout_apply_legacy_message()`, which copies the string into **both** day and night profile `scroll_text`. That is the legacy JSON path for the scrolling message. The text widgets have no JSON path.

---

## 5. Parameter reference

Unless noted, a change is stored to NVS immediately and the display refreshes (`settings_updated`). **Reboot** means firmware starts `restart_device` after the response (hostname, Wi‑Fi, static IP only).

Curl bodies below are the `--data-binary` argument. Prefix with the POST command from [section 4](#4-get--post-apisettings).

### Device / network

| Key | Meaning | Type / range | Reboot | Example body |
|-----|---------|--------------|--------|--------------|
| `p00` | Hostname (mDNS label, no `.local`) | string, 1–32 chars, `[A-Za-z0-9-]`, no leading/trailing `-`. Default `frixos` | **yes** | `{"p00":"frixos"}` |
| `p34` | Wi‑Fi SSID | string, max 32 | **yes** | `{"p34":"MyNetwork"}` |
| `p35` | Wi‑Fi password | string, max 63 | **yes** | `{"p35":"secret"}` |
| `p60` | Static IPv4; empty = DHCP | string, max 15, valid IPv4 if non-empty | **yes** | `{"p60":"192.168.2.50"}` |
| `p61` | Default gateway | string, max 15, IPv4 if non-empty | **yes** | `{"p61":"192.168.2.1"}` |
| `p62` | Subnet mask | string, max 15. Default `255.255.255.0` | **yes** | `{"p62":"255.255.255.0"}` |
| `p63` | DNS, comma-separated | string, max 39, each part IPv4 | **yes** | `{"p63":"8.8.8.8,8.8.4.4"}` |
| `p46` | Wi‑Fi active-hours start | number, minutes from midnight | no | see note |
| `p47` | Wi‑Fi active-hours end | number, minutes from midnight | no | see note |

`p46`/`p47`: minutes from midnight (`0`–`1439`). Both `0` = Wi‑Fi always on. Validation and apply both store that range. Runtime compares the stored number to `hour*60+minute`.

```powershell
curl.exe -s -X POST "$HOST/api/settings" -H "Content-Type: application/json" --data-binary '{"p00":"frixos"}'
curl.exe -s -X POST "$HOST/api/settings" -H "Content-Type: application/json" --data-binary '{"p34":"MyNetwork","p35":"secret"}'
curl.exe -s -X POST "$HOST/api/settings" -H "Content-Type: application/json" --data-binary '{"p46":480,"p47":1320}'
curl.exe -s -X POST "$HOST/api/settings" -H "Content-Type: application/json" --data-binary '{"p60":"","p61":"","p62":"255.255.255.0","p63":""}'
```

Clearing `p60` (empty string) returns the device to DHCP (reboot).

### Location / time

| Key | Meaning | Type / range | Reboot | Example body |
|-----|---------|--------------|--------|--------------|
| `p17` | Latitude | string, max 11, or empty. Decimal degrees | no* | `{"p17":"40.7128000"}` |
| `p18` | Longitude | string, max 11, or empty | no* | `{"p18":"-74.0060000"}` |
| `p19` | POSIX TZ | string, max 127. e.g. `EST5EDT,M3.2.0,M11.1.0`. Empty allowed | no* | `{"p19":"GMT0BST,M3.5.0/1,M10.5.0"}` |
| `tz_iana` | IANA zone (display / fallback) | string, max 63, no `;` or control chars | no* | `{"tz_iana":"America/New_York"}` |
| `p36` | Fahrenheit | `0` = °C, `1` = °F | no | `{"p36":1}` |
| `p37` | 12-hour clock | `0` = 24h, `1` = 12h | no | `{"p37":1}` |

\* Firmware does **not** auto-reboot for lat/lon/tz (unlike hostname/Wi‑Fi). Values are saved to NVS. Timezone is applied on the Wi‑Fi/NTP path at boot (`setenv("TZ", …)`), so a **manual reboot** (`POST /api/reset`) is the reliable way to make `p19` / `tz_iana` live. `GET /api/timezone` can resolve POSIX from IANA or coordinates.

```powershell
curl.exe -s "$HOST/api/timezone?location=Europe/Athens"
curl.exe -s -X POST "$HOST/api/settings" -H "Content-Type: application/json" --data-binary '{"p17":"37.9838100","p18":"23.7275400","p19":"EET-2EEST,M3.5.0/3,M10.5.0/4","tz_iana":"Europe/Athens"}'
```

### Display geometry / clock

| Key | Meaning | Type / range | Example body |
|-----|---------|--------------|--------------|
| `p01` | X offset | `0`–`160` | `{"p01":22}` |
| `p02` | Y offset | `0`–`160` | `{"p02":22}` |
| `p03` | Rotation | `0`=0°, `1`=90°, `2`=180°, `3`=270° | `{"p03":2}` |
| `p09` | Mirror | `0`/`1` | `{"p09":1}` |
| `p08` | Show grid | `0`/`1` | `{"p08":1}` |
| `p24` | Leading zero on hour | `0`/`1` | `{"p24":1}` |
| `p50` | Disable breathing time-dots | `0` = breathe, `1` = disable | `{"p50":1}` |

```powershell
curl.exe -s -X POST "$HOST/api/settings" -H "Content-Type: application/json" --data-binary '{"p03":3,"p09":0}'
```

### Fonts / color filter (JSON)

Fonts and color filters also exist in the **screen blob header**. POST `/api/screen` overwrites fonts, aux fonts, and color filters from the blob. POST `/api/settings` can still set the JSON keys below. Text-widget color and font size are configured on the widget in the screen editor, not through legacy message settings.

| Key | Meaning | Type / range | Example body |
|-----|---------|--------------|--------------|
| `p04` | Day digit font | string, max 11. See font list | `{"p04":"bold"}` |
| `p05` | Night digit font | same | `{"p05":"light"}` |
| `p10` | Day color filter | `0`–`4`: none, red, green, blue, B&W | `{"p10":0}` |
| `p11` | Night color filter | `0`–`4` | `{"p11":4}` |
| `p06` | Show scrolling widget | `0`/`1` (compatibility mirror of the scroll-enabled text widget) | `{"p06":1}` |
| `p07` | Show weather icon | `0`/`1` (mirrors weather widget enabled) | `{"p07":1}` |

Valid font names: `bold`, `light`, `lcd`, `nixie`, `robrito`, `ficasso`, `lichten`, `kablame`, `kablamo`, `kaboom`, `kabboom`, `user1`, `user2`.

**Aux digit fonts** (`day_aux_font` / `night_aux_font`) are **not** JSON settings. Set them in the `/api/screen` header (offsets 36 and 48).

```powershell
curl.exe -s -X POST "$HOST/api/settings" -H "Content-Type: application/json" --data-binary '{"p04":"bold","p05":"light","p10":0,"p11":0}'
```

### Scrolling text (compatibility JSON)

| Key | Meaning | Type / range | Example body |
|-----|---------|--------------|--------------|
| `p16` | Scroll text | string, max **511**. Tokens e.g. `[temp]` | `{"p16":"Hello [temp]"}` |
| `p14` | Scroll delay | `30`–`255` ms (`uint8_t`) | `{"p14":60}` |

`p16` is retained for compatibility: it copies into **both** layout profiles’ `scroll_text` and persists NVS. Scrolling is enabled and styled on an ordinary text widget in the screen layout. For per-profile text or widget configuration, patch the blob ([section 6](#6-screen-layout-binary-apiscreen)).

```powershell
curl.exe -s -X POST "$HOST/api/settings" -H "Content-Type: application/json" --data-binary '{"p16":"UI test message [temp]","p14":60}'
```

### Brightness / light sensor / PWM

| Key | Meaning | Type / range | Example body |
|-----|---------|--------------|--------------|
| `p23` | LED brightness **array** `[day, night]` | JSON array of 1–2 numbers, each **1–100** | `{"p23":[80,40]}` |
| `p20` | Lux sensitivity | float `0`–`50` | `{"p20":8.0}` |
| `p21` | Lux threshold (day/night switch) | float `0`–`500` | `{"p21":20.0}` |
| `p22` | Dim mode | `0`=brightness, `1`=full, `2`=time-of-day | `{"p22":0}` |
| `p55` | Dim window start | minutes from midnight, `0`–`1439` | `{"p55":450}` |
| `p56` | Dim window end | minutes from midnight, `0`–`1439` | `{"p56":1365}` |
| `p42` | PWM frequency Hz | `60`–`50000`. Applied immediately; failed reconfigure rolls back | `{"p42":250}` |
| `p43` | Max power | `1`–`1023` (runtime may cap by board rev) | `{"p43":900}` |

`p23` **must be a JSON array**, not a single number:

```powershell
curl.exe -s -X POST "$HOST/api/settings" -H "Content-Type: application/json" --data-binary '{"p23":[80,40]}'
```

`p55`/`p56` are minutes from midnight (`0`–`1439`), same as `p46`/`p47`. `450` is 07:30.

```powershell
curl.exe -s -X POST "$HOST/api/settings" -H "Content-Type: application/json" --data-binary '{"p55":450,"p56":1365}'
```

### Theme / language / updates

| Key | Meaning | Type / range | Example body |
|-----|---------|--------------|--------------|
| `p40` | Dark web-UI theme | `0`/`1` | `{"p40":1}` |
| `p41` | Language index | `0`–`8`: en, de, fr, it, pt, sv, da, pl, es | `{"p41":0}` |
| `p39` | Auto firmware update | `0`/`1` | `{"p39":1}` |

```powershell
curl.exe -s -X POST "$HOST/api/settings" -H "Content-Type: application/json" --data-binary '{"p40":1,"p41":0}'
```

### Home Assistant / stocks

| Key | Meaning | Type / range | Example body |
|-----|---------|--------------|--------------|
| `p25` | HA base URL | string, max 199. Trailing `/` stripped | `{"p25":"http://homeassistant.local:8123"}` |
| `p26` | HA long-lived token | string, max 254 | `{"p26":"eyJ..."}` |
| `p27` | HA refresh | **minutes**, `1`–`7200` | `{"p27":2}` |
| `p28` | Finnhub API key | string, max 63 | `{"p28":"api_key"}` |
| `p29` | Stock refresh | **minutes**, `1`–`1440` | `{"p29":10}` |
| `p64` | Weather Underground station ID | string, max 16, letters and digits. Any public station, not only one on the account | `{"p64":"KCASANFR123"}` |
| `p65` | Weather Underground API key | string, max 64. From wunderground.com/member/api-keys after a device is registered | `{"p65":"api_key"}` |
| `p66` | Weather Underground refresh | **minutes**, `5`–`180`. Default 15 | `{"p66":15}` |

Firmware multiplies `p27`/`p29`/`p66` by 60 when scheduling fetches. `p64`–`p66` are past the 64-bit settings mask; they are included in the unfiltered GET and in `group=integrations`.

Both `p64` and `p65` must be set. The device then fills `[wu:temp]`, `[wu:hum]`, `[wu:dew]`, `[wu:wind]`, `[wu:gust]`, `[wu:pressure]`, `[wu:rain]`, and `[wu:uv]` from the station’s current observation. These do not replace the met.no tokens (`[temp]`, `[high]`, `[low]`, and the rest).

```powershell
curl.exe -s -X POST "$HOST/api/settings" -H "Content-Type: application/json" --data-binary '{"p27":2,"p29":10,"p66":15}'
```

### CGM (Dexcom / Libre / Nightscout)

Shared credentials: `p31`/`p32`/`p33` (username, password, refresh). Dexcom region `p30`; Libre region `p44`; Nightscout URL `p54`.

| Key | Meaning | Type / range | Example body |
|-----|---------|--------------|--------------|
| `p30` | Dexcom region | `0`=off, `1`=US, `2`=Japan, `3`=RoW | `{"p30":0}` |
| `p44` | Libre region | `0`=off, `1`=US, `2`=EU, `3`=DE, `4`=FR, `5`=JP, `6`=AU, `7`=global | `{"p44":0}` |
| `p31` | Glucose username | string, max 63 | `{"p31":"user"}` |
| `p32` | Glucose password | string, max 63 | `{"p32":"pass"}` |
| `p33` | Glucose refresh | minutes `1`–`60` | `{"p33":5}` |
| `p45` | Glucose validity | minutes `10`–`360` | `{"p45":45}` |
| `p51` | High threshold | mg/dL `1`–`400` | `{"p51":180}` |
| `p52` | Low threshold | mg/dL `1`–`400` | `{"p52":75}` |
| `p53` | Glucose unit | `0`=mg/dL, `1`=mmol/L | `{"p53":0}` |
| `p54` | Nightscout URL | string, max 100 | `{"p54":"https://ns.example.com"}` |

```powershell
curl.exe -s -X POST "$HOST/api/settings" -H "Content-Type: application/json" --data-binary '{"p30":0,"p44":0,"p33":5,"p53":0}'
```

### Text widget rotation

The old digit-display schedule (`p48`, `p49`, `p57`, `p58`, `p59`) is gone. A text widget carries up to **3** contents that it shows in turn, each with its own duration in seconds (`1`–`3600`). A content that has nothing to show right now (a value token without data renders as `-`) is skipped. They are part of the `/api/screen` blob ([section 6](#6-screen-layout-binary-apiscreen)).

A layout written by an older firmware (layout version 10) is converted at the first boot of the new firmware: the schedule that was kept in NVS becomes the contents of the clock widget, and the stored schedule keys are erased.

---

## 6. Screen layout binary (`/api/screen`)

### Sizes (current firmware — do not use `tools/set_graph.py`)

`tools/set_graph.py` still has **stale** constants. Use these, matching `screen-editor.js` and the `_Static_assert` lines in `f-screen-layout-bin.c`:

| Constant | Value |
|----------|-------|
| Magic | `"FSXL"` little-endian `0x4653584C` |
| Format | `1` |
| Layout version | `11` (`FRIXOS_SCREEN_LAYOUT_VERSION`) |
| Header | **64** bytes |
| Widget | 13 bytes × **15** widgets = 195 |
| `scroll_text` | 512 |
| Graph cfg | **88** bytes |
| Profile | **795** bytes |
| Profiles | 2 (day = 0, night = 1) |
| Base wire | `64 + 795*2` = **1654** bytes (`FRIXOS_SCREEN_LAYOUT_WIRE_SIZE`) |
| Text widget section | 8 + 2 × 8 × 216 + 516 = **3980** bytes, appended after the base wire |
| Widget meta (end of the text widget section) | **516** bytes: `uint16 instanced[2]` (bit *i* = slot exists: text 0–7, icons 8–15) + `char name[2][16][16]` |
| Full wire | **5634** bytes (`FRIXOS_SCREEN_LAYOUT_FULL_WIRE_SIZE`) |

The widget meta is editor data (which text/icon slots exist, enabled or not, and their names). The display never reads it. Ext version 3 carries it; version 2 (text widgets only, 5118 bytes) is still read from layout files and by the editor, and every enabled widget then counts as existing.

GET returns the full 5634 bytes. POST accepts 5634 bytes, or the 1654-byte base wire (the text widgets are then left as they are). Wrong `Content-Type` → `{"status":"error","error":"bad_content_type",…}`. Wrong length → `invalid_length` with `got` / `expected`. Bad magic/format/version → `bad_wire`. Success → `{"status":"ok"}`.

```powershell
curl.exe -s -o layout.bin "$HOST/api/screen"
# check size: should be 5634
curl.exe -s -X POST "$HOST/api/screen" `
  -H "Content-Type: application/vnd.frixos.screen-layout+1" `
  --data-binary "@layout.bin"
```

```bash
curl -s -o layout.bin "$HOST/api/screen"
curl -s -X POST "$HOST/api/screen" \
  -H "Content-Type: application/vnd.frixos.screen-layout+1" \
  --data-binary @layout.bin
```

### Header (64 bytes, little-endian)

| Offset | Size | Field |
|--------|------|--------|
| 0 | 4 | magic `FSXL` |
| 4 | 1 | format `1` |
| 5 | 1 | layout version |
| 6 | 1 | `scroll_delay` (decode clamps 30–255) |
| 7 | 1 | day color filter `0`–`4` |
| 8 | 1 | night color filter `0`–`4` |
| 9 | 3 | reserved |
| 12 | 12 | day font, NUL-terminated |
| 24 | 12 | night font |
| 36 | 12 | day **aux** font |
| 48 | 12 | night **aux** font |
| 60 | 2 | width (`128`) |
| 62 | 2 | height (`128`) |

Day profile starts at byte **64**. Night profile at **859** (`64+795`). The text widget section starts at **1654**.

### Offsets within a profile

| Offset | Size | Field |
|--------|------|--------|
| 0 | 195 | `widget[15]` |
| 195 | 512 | `scroll_text` |
| 707 | 88 | graph cfg |

Strings are fixed-length **NUL-terminated**; unused bytes should be `0`. Absolute file offsets of `scroll_text`: day **259**, night **1054**.

### Widget bytes (13)

Index order (`screen_element_id_t`):

`glucose_level`, `glucose_trend`, `wifi_off`, `weather`, `moon`, `message`, `graph`, `icon_1`…`icon_8`.

| +0 | +1 | +2 | +3 | +4 | +5 | +6 | +7–9 | +10–12 |
|----|----|----|----|----|----|----|------|--------|
| enabled | x | y | z | font | width | align | RGB fg | RGB bg |

- `x`,`y`: 0–127. `z`: 0–4 (higher on top).
- `font` (message only): 0–4 = 8–16pt; firmware also allows 5 = 5pt tiny.
- `align`: `0` left, `1` center, `2` right.
- Graph line color = widget fg; graph background = widget bg.
- Icon slots map to SPIFFS `icon1.jpg`…`icon8.jpg`.

### Graph cfg (88 bytes, little-endian)

| Offset | Type | Field |
|--------|------|--------|
| 0 | char[64] | token, e.g. `[temp]` or `[HA:sensor.x:state]` |
| 64 | u16 | interval minutes `1`–`1440` |
| 66 | u8 | points `2`–`100` |
| 67 | u8 | width `60`–`80` |
| 68 | u8 | height `28`–`36` |
| 69 | u8 | flags (see below) |
| 70 | i16 | band_low (`-32768` = unset) |
| 72 | i16 | band_high |
| 74 | i16 | y_min |
| 76 | i16 | y_max |
| 78 | 3×u8 | band RGB |
| 81 | 3×u8 | warn RGB |
| 84 | 3×u8 | axis RGB |
| 87 | u8 | reserved |

Flags: `0x01` autoscale, `0x02` Y axis, `0x04` band, `0x08` backfill (compat), `0x10` value readout, `0x20` boolean, `0x40` thick line, `0x80` X axis.

### Text widgets (section after the base wire)

Header (8 bytes): magic `0x54585346` (`"FSXT"` little-endian), version `2`, widget count `8`, `u16` size of one widget (`216`). Then `8` widgets for the day profile and `8` for the night profile, each `text_widget_cfg_t`:

| Offset | Size | Field |
|--------|------|--------|
| 0 | 1 | enabled |
| 1–3 | 3 | x, y, z (z `0`–`4`) |
| 4 | 1 | width in pixels, `0` = up to the right edge |
| 5 | 1 | align: `0` left, `1` center, `2` right |
| 6 | 1 | family: `0` sans, `1` bold, `2` profile digit theme, `3` aux digit theme, `4`+ = digit theme by index (bold, light, lcd, …) |
| 7 | 1 | size in pixels, `8`–`36` |
| 8 | 1 | flags: `0x01` show units, `0x02` tabular digits, `0x04` transparent background |
| 9 | 1 | content count, `1`–`3` |
| 10–12 | 3 | RGB text colour |
| 13–15 | 3 | RGB background colour |
| 16 | 6 | `u16` duration in seconds of each content |
| 22 | 2 | reserved |
| 24 | 192 | `char text[3][64]` |

A text is a template: `"[temp]"`, `"Now [temp] [high]"`, `"[CGM:reading]"`. Besides the integration tokens it understands `[clock]` (time, honours the 12 h setting and the leading zero setting), `[ampm12]` (`AM`/`PM` in 12 h mode, empty in 24 h mode) and `[colon]` (an animated colon, only meaningful with a digit theme). Default layout: widget 1 = `[clock]` in the profile digit theme at size 36, widget 2 = `[ampm12]`.

---

## 7. Apply flow

There is no separate “apply” endpoint. **POST `/api/screen` is apply.**

1. `GET /api/screen` → 5634-byte `layout.bin` (current live layout, encoded from `eeprom_screen_layout` + fonts/filters).
2. Patch fixed-length NUL-terminated strings and/or widget bytes. Keep length **5634**. Do not change magic/format unless you know the decoder (`format` must stay `1`; `layout_version` must be `≤ 12`; version 11 message widgets are migrated, see above).
3. `POST /api/screen` with `Content-Type: application/vnd.frixos.screen-layout+1` and `--data-binary @layout.bin`.
4. Firmware: decode → copy to `eeprom_screen_layout` → `screen_layout_sync_legacy_eeprom` (mirrors day `scroll_text` into `p16` / `eeprom_message` and `p06`/`p07` from widgets) → `write_nvs_parameters` → `schedule_parse_integrations`. Display uses `eeprom_screen_layout`. Response `{"status":"ok"}`. **No reboot.**

Recommended workflow: always GET, patch, POST (do not synthesize a blob from scratch unless you copy a known-good GET).

`p16` via `/api/settings` is the inverse for the scroll string only: it writes NVS message and copies into **both** profiles’ `scroll_text`, then schedules integration parse. It does not move widgets or text widgets.

---

## 8. Python: patch scroll text and the first text widget

```python
#!/usr/bin/env python3
"""GET /api/screen, patch day+night scroll_text and the text of text widget 1, POST back."""
from urllib.request import Request, urlopen

HOST = "http://frixos.local"  # or http://192.168.2.129
WIRE, HEADER, PROFILE = 5634, 64, 795
BASE_WIRE = HEADER + PROFILE * 2          # 1654: the text widget section follows
SCROLL_OFF, SCROLL_LEN = 195, 512
EXT_HEADER, TW_SIZE, TW_COUNT = 8, 216, 8
TW_TEXT_OFF, TW_TEXT_LEN = 24, 64
MIME = "application/vnd.frixos.screen-layout+1"

def put_cstr(buf: bytearray, off: int, maxlen: int, text: str) -> None:
    data = text.encode("utf-8")[: maxlen - 1]
    buf[off : off + maxlen] = b"\x00" * maxlen
    buf[off : off + len(data)] = data

with urlopen(Request(HOST + "/api/screen")) as resp:
    blob = bytearray(resp.read())
if len(blob) != WIRE:
    raise SystemExit(f"unexpected wire size {len(blob)}, expected {WIRE}")

scroll = "Hello from HTTP [temp]"
text_1 = "Kitchen [temp]"
for profile in (0, 1):  # day, night
    base = HEADER + PROFILE * profile
    put_cstr(blob, base + SCROLL_OFF, SCROLL_LEN, scroll)
    # first content of text widget 1 of this profile
    tw = BASE_WIRE + EXT_HEADER + TW_SIZE * TW_COUNT * profile
    put_cstr(blob, tw + TW_TEXT_OFF, TW_TEXT_LEN, text_1)

req = Request(
    HOST + "/api/screen",
    data=bytes(blob),
    method="POST",
    headers={"Content-Type": MIME},
)
with urlopen(req) as resp:
    print(resp.read().decode())
```

Text widget `N` (1-based) of a profile is at `BASE_WIRE + EXT_HEADER + TW_SIZE * (TW_COUNT * profile + N - 1)`.

---

## 9. Other HTTP APIs

### Status

```powershell
curl.exe -s "$HOST/api/status"
curl.exe -s "$HOST/api/status?logs=1"
```

Useful fields: `wifi_connected`, `app`, `version`, `fwversion`, `ip_address`, `mac_address`, `free_heap`, `uptime`, `lux`, `latitude`, `longitude`, `timezone`, `poh`. `logs=1` appends `system_logs` and integration token dumps (`ha_tokens`, stocks, CGM, Weather Underground). A fresh station observation looks like `Weather Underground: KCASANFR123, obs 4m, temp 22°C`. `stale` or `no observation` means the integration dot stays amber.

### Locate / timezone

```powershell
curl.exe -s "$HOST/api/locate"
curl.exe -s "$HOST/api/timezone?location=Europe/Athens"
curl.exe -s "$HOST/api/timezone?lat=37.98&lon=23.73"
```

### Reboot

```powershell
curl.exe -s -X POST "$HOST/api/reset"
```

Returns `{"status":"ok","message":"Device is restarting..."}` then restarts after ~5s.

### OTA / file upload

`POST /api/ota` accepts multipart or a raw body. Firmware vs SPIFFS is detected from the payload (`X-Filename` or multipart filename). Firmware `.bin` is written to the OTA partition and the device reboots. Other files (fonts, `iconN.jpg`, …) go to SPIFFS.

```powershell
curl.exe -s -X POST "$HOST/api/ota" -H "X-Filename: user1-font.jpg" --data-binary "@user1-font.jpg"
curl.exe -s -X POST "$HOST/api/ota/reinstall"
```

### Wi‑Fi scan

```powershell
curl.exe -s "$HOST/api/wifi/scan"
curl.exe -s "$HOST/api/wifi/status"
```

Status JSON: `scanning`, `scan_done`, `count`, and when done `networks[]` with `ssid`, `rssi`, `signal_strength`, `requires_password`.

### SPIFFS files

```powershell
curl.exe -s "$HOST/api/files"
curl.exe -s -X POST "$HOST/api/files/rename" -H "Content-Type: application/json" --data-binary '{"oldName":"a.jpg","newName":"b.jpg"}'
curl.exe -s -X POST "$HOST/api/files/delete" -H "Content-Type: application/json" --data-binary '{"files":["b.jpg"]}'
```

Names are relative to `/spiffs`, max 110 chars, no `..` or leading `/`.

---

## 10. Reboot, apply, and gotchas

**Auto-reboot after `POST /api/settings`:** only if `p00`, `p34`, `p35`, `p60`, `p61`, `p62`, or `p63` actually changed. Not lat/lon/tz (those persist; reboot yourself if timezone should go live immediately).

**Layout POST** persists and is what the display uses. No reboot.

**Do not POST JSON to `/api/screen`.** Use the 5634-byte blob.

**`p16` vs text widgets:** scroll message has a JSON path; text widgets do not.

**HA `const.py`** does not list `p44`–`p63` or `tz_iana`. Firmware wins.

**`p23`** is `[day, night]`, not a scalar.

**Windows:** `curl.exe`, not `curl`. For binary POST always `--data-binary` (never `--data`, which can strip NULs / convert newlines).
