# Frixos Project Technical Guidelines & Best Practices

This document provides specialized instructions for GitHub Copilot when working on the Frixos firmware (ESP-IDF 6.x and LVGL 9).

## ⚡ Workflow Rules
- **Do NOT run `idf.py build`** at the end of a task unless the user explicitly asks for a build or compilation.
- **Do NOT create markdown documentation files** for changes unless explicitly requested.

## 🚀 General Architecture
- **Language**: C (ESP-IDF).
- **Core Strategy**: Performance-focused daylight projection clock with heavy integration (CGM, Weather, Stocks, HA).
- **Modular Design**: Modules follow the `f-module.c` / `f-module.h` naming convention. Components are stored in `main/`.
- **Screen resolution**: 128×128 pixels (`LCD_H_RES`/`LCD_V_RES` in `main/include/frixos.h`).
- **Display buffer**: 128px × 8 lines × 2 bytes (RGB565), partial refresh.
- **Header files**: Module headers live in `main/include/` (e.g. `main/include/f-display.h`, `main/include/frixos.h`).

## 🛠 ESP-IDF Optimization (Flash & Memory)
- **Flash Optimization**:
  - Keep `CONFIG_COMPILER_OPTIMIZATION_SIZE=y` active in `sdkconfig`.
  - Use `spiffs` for all non-volatile UI assets and web interface files.
  - Avoid large static arrays in RAM; use `const` for data that should remain in Flash (`RODATA`).
- **Memory Management**:
  - **Shared Buffers**: Use the custom buffer pooling in [f-membuffer.c](main/f-membuffer.c) (`get_shared_buffer`) for HTTP operations to prevent heap fragmentation.
  - **Stack Sizes**: LVGL task requires a large stack (10KB+). Always monitor stack high-water marks for new tasks.
  - **PSRAM**: Avoid using SPIRAM for display buffers to maintain high refresh rates.

## 🎨 LVGL 9 Best Practices
- **Concurrency**:
  - ALL LVGL calls must be wrapped in `lvgl_port_lock(0)` and `lvgl_port_unlock()`.
  - The `esp_lvgl_port` task runs `lv_timer_handler` internally. **Never call `lv_task_handler()` from `display_task`** — doing so causes two tasks to run the LVGL timer handler simultaneously, corrupting the event list and crashing in `lv_event_mark_deleted` (LVGL issue #6677).
  - `lv_timer_t` callbacks are called from the LVGL port task — **do NOT use `lvgl_port_lock/unlock` inside a timer callback**, it will self-deadlock.
- **Memory**:
  - Use `lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN)` and `lv_obj_remove_flag` instead of frequent creation/deletion of objects to keep the heap stable.
  - Prefer `lv_image` for sprites and digits.
- **Drawing**:
  - Custom drawing events (see `text_label_draw_trim_bg` in [f-display.c](main/f-display.c)) are used to optimize background redraws on low-resolution vertical-aligned screens.
- **LVGL 9 API** (verified in `managed_components/lvgl__lvgl/`):
  - Use `lv_display_t` and `lv_display_set_rotation` instead of the old v8 `lv_disp_drv_t`.
  - Use `lv_draw_buf_create` for custom buffers if needed.
  - Transform scale (independent X/Y): `lv_obj_set_style_transform_scale_x(obj, value, LV_PART_MAIN)` / `_scale_y`. Value 256 = 100%.
  - **⚠️ NEVER use `transform_scale` on animated objects** — LVGL allocates an intermediate layer buffer (~5.4 KB for a 32×22 image) on every single frame, which immediately exhausts the LVGL heap (~22 KB total, 53% already in use) and causes a task watchdog crash (`taskLVGL` blocking `IDLE1`). Use `lv_obj_set_pos()` for smooth animations instead.
  - Transform pivot: `lv_obj_set_style_transform_pivot_x(obj, px, LV_PART_MAIN)` / `_pivot_y`.
  - LVGL timer: `lv_timer_create(cb, period_ms, user_data)` / `lv_timer_delete(timer)`.

## 📡 Networking & Integrations
- **HTTP/HTTPS**: Use `esp_http_client` with shared buffers.
- **NVS Mapping**: Use the `pXX` short-key mapping (defined in [f-settings.c](main/f-settings.c)) to minimize NVS overhead and HTTP response sizes.
  - Last assigned key: **p65**. The `calculate_include_mask()` function uses a `uint64_t` bitmask covering **p0–p63 only**. Keys p64+ must be handled with an explicit `if` outside the mask loop in `send_json_settings()` and in the advanced group of `calculate_include_mask`.
  - Adding a new boolean/uint8 setting requires 5 coordinated edits: `main.c` (global + `settings_table[]`), `frixos.h` (extern), `f-settings.c` (comment, mask, GET serialization, POST validation, POST parsing), `spiffs/index.js` (fetch params, save handler, UI widget), `spiffs/language_*.json` × 9 (translation keys).
  - Settings marked non-critical (not network/timezone/location) take effect **live without reboot** via the `settings_updated = true` flag picked up by `display_task`.
- **Concurrency**: Use `http_mutex` when multiple tasks perform networking operations.

## �️ Display Architecture
- **Weather icon** (`img_weather`): `lv_image` object, 32×22px, sprite-strip technique — `lv_image_set_offset_x(img_weather, -index * 32)` selects the icon frame.
- **Layout system**: Widget positions live in `eeprom_screen_layout.profile[font_index].widget[SCREEN_ELEM_*]`. Positions are only valid after WiFi connects (`screen_layout_positions_live == true`). Functions `layout_abs_x(w)` / `layout_abs_y(w)` retrieve the absolute coordinates.
- **Font index**: `font_index` selects day (0) or night (1) profile automatically.
- **Live setting detection pattern**: Use a `static last_X` variable inside the `display_task` loop, compare to the current `eeprom_X`, call the apply function on change. See `eeprom_dots_breathe` at line ~2192 for the reference pattern.
- **`display_changed()`**: Full re-initialization of the display (font reload, layout, color filter). Call sparingly. Safe to call from any task — it uses `lvgl_port_lock` internally.

## 📝 Coding Standards
- **Logging**: Use `ESP_LOGI`, `ESP_LOGW`, `ESP_LOGE` with the `TAG` defined in each module.
- **Error Handling**: Use `ESP_RETURN_ON_ERROR` and `ESP_GOTO_ON_ERROR` macros for clean error propagation.
- **Language**: All comments, default labels, variable names, log messages, and inline strings in source code **must be written in English**. The only exception is the `language_*.json` translation files in `spiffs/`, which contain localized strings by design.
- **Comments**: Keep technical logic documented in English, but refer to the [Frixos Help Center](https://buyfrixos.com/help-center/) for UX-related terminology.
- **JSON language files**: Always validate with `python3 -m json.tool` after editing — missing commas between the last new key and the next existing key are a common mistake.

## 🔗 Project Context
- **Website**: [buyfrixos.com](https://buyfrixos.com)
- **Support**: [buyfrixos.com/help-center/](https://buyfrixos.com/help-center/)
- **Target OS**: ESP-IDF 6.x (Ubuntu 24.04 Environment).
