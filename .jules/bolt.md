## 2025-03-24 - Optimized Character Width Caching in Firmware
**Learning:** The previous character width cache implementation used a linear search ((N)$) and performed redundant string scans ((L)$) inside the character measurement loop, resulting in (L^2)$ complexity for string measurement. This is a significant bottleneck for real-time scrolling text on embedded systems.
**Action:** Use direct-mapped hash lookups ((1)$) for Unicode code points and eliminate inner-loop string operations. Avoid "double-check" logic that bypasses the cache for non-ASCII characters. Mark unused legacy parameters with `(void)` to maintain API compatibility without warnings.

## 2026-03-30 - Conditional Data Fetching for Status API
**Learning:** Large JSON payloads containing system logs and integration tokens were being generated on every status refresh, causing significant CPU and memory pressure on the ESP32. Even when the UI only needed minimal sensor data (e.g., lux levels for auto-dimming), the backend was performing expensive string formatting and JSON array construction.
**Action:** Implement conditional data fetching via query parameters (e.g., `?logs=1`). Wrap resource-intensive JSON construction blocks in conditional checks on the backend and update the frontend to request heavy data only when necessary (e.g., viewing the full Status page or generating support reports).

## 2026-04-03 - Memory Optimization for JSON API Responses
**Learning:** Using `cJSON_Print` (pretty-print) for JSON API responses in memory-constrained environments like the ESP32 introduces significant overhead. It requires larger temporary heap allocations for whitespace/indentation and increases the network payload size unnecessarily, as the web UI consumes minified JSON equally well.
**Action:** Replace `cJSON_Print` with `cJSON_PrintUnformatted` in performance-critical or large-payload API handlers. This reduces response size by ~23% and lowers peak heap usage.
