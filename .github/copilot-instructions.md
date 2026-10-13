# Frixos Project Instructions

Technical constraints, project specifics and coding practices for the Frixos firmware (ESP32 WROOM) and its embedded Web UI. Read this before changing code, and update it when a practice changes.

## 🛠 Critical Configuration (sdkconfig)

If `sdkconfig` is reset or modified, ensure these values are restored. Failure to do so will cause immediate **Stack Overflow** (Boot Loops) or **UI Failures** (Invisible digits).

### 1. Stack Sizes (Anti-Boot Loop)
LVGL 9 requires significantly larger stacks than LVGL 8.
- `CONFIG_ESP_MAIN_TASK_STACK_SIZE=8192` (Required for `app_main` calling LVGL functions).
- `CONFIG_LV_TIMER_TASK_STACK_SIZE=8192` (Required for the built-in LVGL timer task). Not set in `sdkconfig` or `sdkconfig.defaults` at the last check; verify in menuconfig.
- `CONFIG_ESP_SYSTEM_EVENT_STACK_SIZE=4096` (Increased for network/system stability). Same check as above.

### 2. LVGL Graphics & Decoding
- `CONFIG_LV_USE_TJPGD=y` - **MANDATORY**. The clock digits are JPEG resources. Without this, the time will NOT appear.
- `CONFIG_LV_MEM_SIZE_KILOBYTES=32` in `sdkconfig` (`24` in `sdkconfig.defaults`). The old value of 64 does not match the files. Keep the value the build uses and check the heap after changes.

## 🧠 Hardware & Memory Constraints (ESP32 WROOM)

- **No PSRAM**: The device only has ~320KB of internal DRAM.
- **Available Heap**: Approximately **~35KB** free after LVGL initialization.
- **Display**: ST7735 (128x128 resolution).
- **Color Format**: BGR (Swap enabled).
- **Buffering**: Double buffering is used (2 x 128x8 lines) to minimize RAM footprint while allowing smooth scrolling text.

## 📝 Coding Standards for Frixos

1. **Memory Safety**: Avoid large local arrays. Do not `malloc` HTTP or response buffers: use `get_shared_buffer(size, "OWNER")` (`f-membuffer.c`) and always call `release_shared_buffer()` when done. Watch the ~35 KB heap limit.
2. **Conditional UI**: Use `lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN)` and `lv_obj_remove_flag(obj, LV_OBJ_FLAG_HIDDEN)` for the clock digits.
3. **Time Validation**: Always check `time_valid` before attempting to render or update the clock objects.
4. **Scrolling Text**: The scrolling text is the only element that can handle large string buffers due to its dynamic nature. Keep stock tokens within reasonable length.
5. **Language**: All comments, default labels, variable names, log messages, and inline strings in source code **must be written in English**. The only exception is the `language_*.json` translation files in `spiffs/`, which contain localized strings by design.
6. **Logging**: Use the project function `ESP_LOG_WEB(level, TAG, fmt, ...)` declared in `main/include/frixos.h` (717 call sites, versus 28 direct `ESP_LOGx` calls). It writes to the serial console and to the web log buffer. Each module declares `static const char *TAG = "f-<module>";`. Never log passwords, tokens, account IDs or other secrets; log status codes, endpoint names and lengths instead.
7. **Error Handling**: Use `ESP_RETURN_ON_ERROR` and `ESP_GOTO_ON_ERROR` for clean error propagation (rare in this code base). Typical code uses an explicit check plus an `ESP_LOG_WEB` line.
8. **Compilation**: **NEVER** run `idf.py build` (or any equivalent build task/command) at the end of a request unless specifically asked by the user in the prompt. Do not attempt to "verify" code by building unless explicitly requested.
9. **Numeric constants**: `#define` them, or derive them (e.g. `sizeof(buf) - 1`, `strlen(s) - 2`). No bare magic numbers.
10. **JSON**: Avoid cJSON on large or frequent data (heap heavy). Prefer string functions. For API responses use `cJSON_PrintUnformatted()`, never `cJSON_Print()`.

## 🎞 Animation & Graphics (LVGL 9)

### Memory-Safe Animation
- **AVOID `lv_obj_set_style_transform_scale`**: On this target (ESP32-WROOM without PSRAM), scaling causes LVGL to allocate an intermediate ARGB8888 buffer (~5.4 KB to 11 KB). This often leads to immediate OOM/Watchdog restarts because the largest free heap block is typically < 5 KB.
- **Prefer Position Animation**: Use `lv_obj_set_pos` or `lv_obj_set_x/y` for transformations.
- **Fluidity & Resolution**: On a 128x128 screen, very slow movements (e.g., 1px every 2 seconds) will appear staggered. Use `roundf()` for coordinate calculation to minimize truncation artifacts.
- **Label updates**: In high-frequency loops, compare with `lv_label_get_text()` (strcmp) before calling `lv_label_set_text()`. Setting identical text still triggers re-parsing and re-layout.

### UI Settings Toggling
- **Live Updates**: Use the pattern of `static last_setting` variables within a main loop or dedicated task (like `display_task`) to detect NVS global changes and apply them dynamically (e.g., creating/deleting timers or updating UI objects) without requiring a device reboot.
- **Conditional Visibility**: When a feature (like animation) is toggled off in the Web UI, its sub-parameters (speed, amplitude) should be hidden with the show/hide logic in the Web UI source (`spiffs/js/`, `spiffs/css/`).

## 📚 Documentation Reference
- [docs/developer-guide.md](docs/developer-guide.md): build, flash, SPIFFS deployment, local Web UI work.
- [docs/kb-http-api.md](docs/kb-http-api.md): settings and layout HTTP API.
- [docs/design-generic-graph-widget.md](docs/design-generic-graph-widget.md), [docs/kb-user-font-guide.md](docs/kb-user-font-guide.md), [docs/kb-flashing-esp32-p1-pgm.md](docs/kb-flashing-esp32-p1-pgm.md): feature and hardware notes.

## 🌐 Web UI (SPIFFS)

### Architecture
- Single-page app served from the SPIFFS partition `spiffs` (offset `0x670000`, size `0x180000` = 1.5 MB, see `partitions.csv`).
- Source of truth: `spiffs/` (packed into `build/spiffs.bin`). `frixos-ui/` also exists with `app.js`, `app.css` and `index.html`, which are not in `spiffs/`. Confirm which copy the change belongs to before editing.
- Layout: `spiffs/index.html`; JS in `spiffs/js/` (core, forms, integrations, savebar, screen-editor, support, system, utils, wifi); CSS in `spiffs/css/` (components, forms, layout, screen-editor, variables); i18n in `spiffs/i18n/`; screen layouts `spiffs/*.layout` (Default, Diabetic, HAGraph, HomeAssistant, Weather); images in `spiffs/`.
- **Files are NOT minified.** The `spiffs/` folder is packed as-is.
- JPEG assets dominate the partition size (folder ~1.2 MB in total).
- OTA does not update SPIFFS. Flash it separately: `idf.py spiffs-flash` or `esptool.py write_flash 0x670000 build/spiffs.bin`.

### Settings API
- Endpoint: `GET` / `POST /api/settings`
- Keys use short `pXX` names for HTTP efficiency (e.g. `p60`=static_ip, `p61`=static_gw, `p62`=static_nm, `p63`=static_dns).
- Group filters: `settings`, `advanced`, `integrations`, `theme`.
- Shared HTTP buffer pool: `get_shared_buffer()` in `f-membuffer.c`.
- Status endpoint: `GET /api/status` returns network info (`ip_address`, `ip_gw`, `ip_nm`, `ip_dns1`, `ip_dns2`).

### i18n Rules
- All user-facing strings use `data-i18n="key.path"` attributes; JS resolves them at runtime.
- Translation keys live in `spiffs/i18n/language_XX.json` (en, de, fr, it, pt, sv, da, pl, es).
- When adding a new UI label, always add the key to **all 9 language files**.
- Default fallback text in HTML must be in English.

### UI Layout Conventions
- Settings tab is structured into named `<div class="section">` blocks, each with a `<div class="section-header">` and `<div class="section-content">`.
- **Single Save button** per form, placed at the bottom in a standalone `<div class="section">`.
- Two-column layout (`.connection-layout`) uses `display: flex; align-items: stretch` with `flex: 1 1 0` on both columns so they share equal width and height.
- Responsive breakpoint at **820px**: columns stack vertically.
- Static IP panel is a collapsible section (`.static-ip-section`) toggled by a checkbox; hidden/shown via `style="display:none"` (browser-side HTML, not LVGL flags).

## 🔑 NVS Settings

- Namespace: `frixos`.
- All persistent settings are declared in `settings_table[]` in `main/main.c` with `SETTING_TYPE_STR` or numeric types.
- Extern declarations for cross-module access live in `main/include/frixos.h`.
- Static IP variables: `eeprom_static_ip[16]`, `eeprom_static_gw[16]`, `eeprom_static_nm[16]`, `eeprom_static_dns[40]` (keys p60–p63).
- Static IP applied in `connect_to_wifi()` (`f-provisioning.c`): `esp_netif_dhcpc_stop()` → `esp_netif_set_ip_info()` before `esp_wifi_connect()`. Falls back to DHCP on error.
- DNS field supports two comma-separated values (DNS1,DNS2) stored in a single NVS key.

## 📡 FreeStyle / LibreLinkUp (`main/f-freestyle.c`)

Unofficial client for Abbott's undocumented LibreLinkUp API. It feeds `glucose_data` and `cgm_history_add()` (`f-graph.c`).

- **Endpoints**: `POST /llu/auth/login`, `GET /llu/connections`, `GET /llu/connections/{patientId}/graph`.
- **Headers** (after login): `product: llu.android`, `version` (`LIBRE_CLIENT_VERSION`, currently `4.20.0`, accepted by the live API), `authorization: Bearer <token>`, `account-id` = SHA-256 hex digest of `user.id`.
- **Auth headers on every call**: graph calls without `authorization` and `account-id` return HTTP 400. A cached patient ID in NVS skips `fetch_patient_id()`, so the headers must come from the stored token and account ID on every request.
- **Status is in the JSON body**, not in HTTP 3xx: `status` 0 = OK; 2 = bad credentials; 4 = step required (`tou`/`pp`): log it and stop, do not loop; 429 or `code` 60 = lockout (3 failures lock the account for 300 s); 920 = app version too old (`data.minimumVersion`).
- **Region**: `data.redirect` with `data.region` makes `login_freestyle()` retry once on `https://api-{region}.libreview.io`. The region must be 2-3 lowercase letters. The URL lives in `eeprom_libre_region_url` (RAM only) and is reset at each login.
- **Lockout risk**: never probe regions or retry passwords. Failures count account-wide. After an API-level rejection (status 2, 4, 429, 920, code 60), `login_freestyle()` pauses 300 s (`LIBRE_LOGIN_COOLDOWN_US`).
- **Recovery**: HTTP 401 and 400 on the graph call clear token and patient ID; `fetch_freestyle_glucose()` re-logs in once.
- **Logs**: endpoint, HTTP status and body `status`/`code` only. Never log the password, token, account ID or full response bodies.

## 🔌 Network & HTTP

- Use one persistent `esp_http_client` per service, and release it in the matching cleanup function (see `cleanup_freestyle_client()`).
- TLS RX/TX buffers were shrunk so the handshake fits in ESP32 DRAM (commit `ed77804`). Do not enlarge them without checking the heap.
- Display wake-ups on core 1 can slice TLS handshakes (commit `e5f09fb`). Keep handshakes uninterrupted.
- Home Assistant reuses one TLS session for every token in a cycle (commit `3d63683`).
- `http_mutex` is shared by outbound integrations. `POST /api/settings` returns 503 while it is busy (see `f-settings.c`). Keep lock hold times short.
- TLS uses the ESP certificate bundle (`esp_crt_bundle.h`).

## ⚡ Performance Notes (from `.jules/` and commit history)

- Return `cJSON_PrintUnformatted()` from API handlers (about 23% smaller payloads, less heap).
- Fetch heavy data only on demand, for example through query parameters such as `?logs=1` on `/api/status`.
- Use a `strcmp` guard before `lv_label_set_text()` in loops (see LVGL rules).
- Parse large bodies with string scanning rather than cJSON (see `scan_glucose_window()` in `f-freestyle.c`).

## 🔗 Project Context
- **Website**: [buyfrixos.com](https://buyfrixos.com)
- **Support**: [buyfrixos.com/help-center/](https://buyfrixos.com/help-center/)
- **Build**: ESP-IDF v6.0.1 (per the `ESP-IDF: Build` task: Windows `export.bat` then `idf.py build`). LVGL is `^9.2.2` in `main/idf_component.yml`, resolved to 9.5.0 in `managed_components/`.
- **Dev container**: Ubuntu 24.04 for editing, Python tools and the mock server (`tools/mock_server.py`, see the developer guide). Build and flash run on the Windows host.
- **CI**: `.github/workflows/ci.yml` runs `tools/test_ui_settings.py` on push and pull request (UI settings contract). Keep Web UI setting keys and the firmware apply path in sync.
- **Web Server task**: `Web Server: Start` expects a `frixos-web-server/` folder (uvicorn app), which is not in this workspace.
