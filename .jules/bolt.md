## 2025-03-24 - Optimized Character Width Caching in Firmware
**Learning:** The previous character width cache implementation used a linear search ((N)$) and performed redundant string scans ((L)$) inside the character measurement loop, resulting in (L^2)$ complexity for string measurement. This is a significant bottleneck for real-time scrolling text on embedded systems.
**Action:** Use direct-mapped hash lookups ((1)$) for Unicode code points and eliminate inner-loop string operations. Avoid "double-check" logic that bypasses the cache for non-ASCII characters. Mark unused legacy parameters with `(void)` to maintain API compatibility without warnings.

## 2025-05-15 - Redundant LVGL UI Updates in Hot Path
**Learning:** In LVGL, `lv_label_set_text` is expensive because it involves string copying and triggers layout recalculations. In high-frequency loops (like scrolling text), calling this unconditionally every frame (~30-65ms) creates significant CPU overhead even if the visible substring hasn't changed.
**Action:** Always check the current label content using `lv_label_get_text` and compare it with the new content before calling `lv_label_set_text`. Use bitwise AND (`&`) instead of modulo (`%`) for hash-style cache lookups when the cache size is a power of two to save cycles in hot paths.
