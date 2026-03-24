## 2025-03-24 - Optimized Character Width Caching in Firmware
**Learning:** The previous character width cache implementation used a linear search ((N)$) and performed redundant string scans ((L)$) inside the character measurement loop, resulting in (L^2)$ complexity for string measurement. This is a significant bottleneck for real-time scrolling text on embedded systems.
**Action:** Use direct-mapped hash lookups ((1)$) for Unicode code points and eliminate inner-loop string operations. Avoid "double-check" logic that bypasses the cache for non-ASCII characters. Mark unused legacy parameters with `(void)` to maintain API compatibility without warnings.

## 2025-03-24 - Redundant LVGL Label Updates
**Learning:** Calling `lv_label_set_text` every frame is extremely expensive in LVGL as it triggers memory reallocation and a full re-layout, even if the text hasn't changed. Furthermore, `static` caches in functions like `display_string_substring` can lead to cross-object state leakage if they don't explicitly track the object pointer (`label_obj`).
**Action:** Always implement a `cache_initialized` flag and an object pointer check (`last_label != label_obj`) to ensure safe caching. Use `strcmp` to bypass `lv_label_set_text` when the visual content is unchanged. Ensure `<string.h>` is included for these operations.
