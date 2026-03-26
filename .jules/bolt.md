## 2025-03-24 - Optimized Character Width Caching in Firmware
**Learning:** The previous character width cache implementation used a linear search ((N)$) and performed redundant string scans ((L)$) inside the character measurement loop, resulting in (L^2)$ complexity for string measurement. This is a significant bottleneck for real-time scrolling text on embedded systems.
**Action:** Use direct-mapped hash lookups ((1)$) for Unicode code points and eliminate inner-loop string operations. Avoid "double-check" logic that bypasses the cache for non-ASCII characters. Mark unused legacy parameters with `(void)` to maintain API compatibility without warnings.

## 2025-03-26 - Optimized Scrolling Text and Font Cache
**Learning:** Frequent updates to LVGL labels (via `lv_label_set_text`) trigger expensive re-layouts even if the text remains identical, which is a major performance drain in high-frequency loops (like scrolling). Additionally, using `static` variables for caching without tracking the target object (e.g. `label_obj`) leads to correctness bugs when the same function is used across different UI elements.
**Action:** Always implement a `strcmp` guard before updating UI label text. For function-level static caches, ensure the cache key includes all relevant context (object pointers, fonts, etc.) to maintain multi-label safety and prevent state leakage between components.
