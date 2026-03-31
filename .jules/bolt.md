## 2025-03-24 - Optimized Character Width Caching in Firmware
**Learning:** The previous character width cache implementation used a linear search ((N)$) and performed redundant string scans ((L)$) inside the character measurement loop, resulting in (L^2)$ complexity for string measurement. This is a significant bottleneck for real-time scrolling text on embedded systems.
**Action:** Use direct-mapped hash lookups ((1)$) for Unicode code points and eliminate inner-loop string operations. Avoid "double-check" logic that bypasses the cache for non-ASCII characters. Mark unused legacy parameters with `(void)` to maintain API compatibility without warnings.

## 2026-03-30 - Conditional Data Fetching for Status API
**Learning:** Large JSON payloads containing system logs and integration tokens were being generated on every status refresh, causing significant CPU and memory pressure on the ESP32. Even when the UI only needed minimal sensor data (e.g., lux levels for auto-dimming), the backend was performing expensive string formatting and JSON array construction.
**Action:** Implement conditional data fetching via query parameters (e.g., `?logs=1`). Wrap resource-intensive JSON construction blocks in conditional checks on the backend and update the frontend to request heavy data only when necessary (e.g., viewing the full Status page or generating support reports).

## 2026-03-31 - Rendering Throttling in LVGL
**Learning:** Calling `lv_label_set_text` on every rendering frame (e.g., during message scrolling) triggers expensive internal LVGL operations like re-parsing, memory re-allocation, and layout invalidation, even if the text content remains identical. This consumes significant CPU cycles in the main rendering loop.
**Action:** Always implement a `strcmp` guard (checking `lv_label_get_text`) before calling `lv_label_set_text` in high-frequency loops. This ensures that the library only performs work when the content actually changes.
