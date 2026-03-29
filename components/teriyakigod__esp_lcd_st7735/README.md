# ST7735 LCD panel driver (vendored)

ESP-IDF `esp_lcd` vendor driver for Sitronix ST7735 panels.

## Origin

This tree is based on **teriyakigod/esp_lcd_st7735** (ESP Component Registry), version **0.0.1**. The component manifest points at upstream sources on GitHub as **appsou/esp_lcd_st7735**; registry and Git naming differ, but the package is the same family of ST7735 glue for `esp_lcd`.

It lives under **`components/teriyakigod__esp_lcd_st7735`** in the **Frixos** firmware so local changes are tracked with the project and are not replaced when managed dependencies are refreshed.

## Custom changes (Frixos)

These edits are intentional and should be preserved when merging any future upstream release.

1. **`disp_on_off` for ESP-IDF 5.x**  
   The panel vtable always assigns `base.disp_on_off` (older `#if ESP_IDF_VERSION < 5` branches are commented out). This matches the current `esp_lcd` panel API used on IDF 5.x.

2. **Optional RGB565 “color channel” / grayscale path in `draw_bitmap`**  
   A per-panel `color_channel` field (default `0` = off) controls post-processing of each pixel before `RAMWR`:
   - `0` — pass framebuffer through unchanged  
   - `1` / `2` / `3` — drive only R / G / B at a shared luma derived from the original RGB565 sample  
   - Other values — map to monochrome (R, G, and B set to the same luma)  

   Implementation detail: samples are interpreted with **`__builtin_bswap16`** so 565 component extraction matches the byte order Frixos uses on the wire. Alternate little-endian decoding paths are left commented in source for reference.

3. **`panel_st7735_set_color_channel()`**  
   Public API (declared in `include/esp_lcd_st7735.h`) to set the mode above at runtime. **Frixos** calls this from the display layer (e.g. UI “curve” / channel emphasis) so the panel can emphasize one channel or grayscale without changing LVGL color conversion globally.

4. **Logging**  
   Extra `ESP_LOGI` on successful panel creation (`LCD panel create success`) for bring-up; adjust log level if noisy in production.

## Files

| File | Role |
|------|------|
| `esp_lcd_st7735.c` | Panel implementation (init, draw, mirror, custom channel pass) |
| `include/esp_lcd_st7735.h` | ST7735 command constants, vendor config types, `esp_lcd_new_panel_st7735`, `panel_st7735_set_color_channel` |
| `CMakeLists.txt` | Registers component; requires `driver`, `esp_lcd` |
| `idf_component.yml` | Original package metadata (description, license, version tag from registry) |

## License

MIT (see upstream / `idf_component.yml`).
