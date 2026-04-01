## 2025-03-24 - Optimized Character Width Caching in Firmware
**Learning:** The previous character width cache implementation used a linear search ((N)$) and performed redundant string scans ((L)$) inside the character measurement loop, resulting in (L^2)$ complexity for string measurement. This is a significant bottleneck for real-time scrolling text on embedded systems.
**Action:** Use direct-mapped hash lookups ((1)$) for Unicode code points and eliminate inner-loop string operations. Avoid "double-check" logic that bypasses the cache for non-ASCII characters. Mark unused legacy parameters with `(void)` to maintain API compatibility without warnings.

## 2026-03-30 - Conditional Data Fetching for Status API
**Learning:** Large JSON payloads containing system logs and integration tokens were being generated on every status refresh, causing significant CPU and memory pressure on the ESP32. Even when the UI only needed minimal sensor data (e.g., lux levels for auto-dimming), the backend was performing expensive string formatting and JSON array construction.
**Action:** Implement conditional data fetching via query parameters (e.g., `?logs=1`). Wrap resource-intensive JSON construction blocks in conditional checks on the backend and update the frontend to request heavy data only when necessary (e.g., viewing the full Status page or generating support reports).

## 2024-05-15 - Redundant Rendering and Math Optimization in Firmware
**Learning:** Calling `lv_label_set_text` in a high-frequency loop (e.g., scrolling text) triggers expensive re-layout and re-rendering in LVGL even if the text content remains identical. Additionally, using `pow(x, 2)` in easing functions on embedded systems (ESP32) is significantly slower than direct multiplication `(x * x)` due to the overhead of the general math library.
**Action:** Always implement a `strcmp` or similar check before updating UI labels that are refreshed frequently. Replace generic math functions with simple arithmetic counterparts for basic operations in hot paths to save CPU cycles and reduce power consumption.
