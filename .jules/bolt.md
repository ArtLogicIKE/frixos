## 2025-03-24 - Optimized Character Width Caching in Firmware
**Learning:** The previous character width cache implementation used a linear search ((N)$) and performed redundant string scans ((L)$) inside the character measurement loop, resulting in (L^2)$ complexity for string measurement. This is a significant bottleneck for real-time scrolling text on embedded systems.
**Action:** Use direct-mapped hash lookups ((1)$) for Unicode code points and eliminate inner-loop string operations. Avoid "double-check" logic that bypasses the cache for non-ASCII characters. Mark unused legacy parameters with `(void)` to maintain API compatibility without warnings.

## 2026-03-30 - Conditional Data Fetching for Status API
**Learning:** Large JSON payloads containing system logs and integration tokens were being generated on every status refresh, causing significant CPU and memory pressure on the ESP32. Even when the UI only needed minimal sensor data (e.g., lux levels for auto-dimming), the backend was performing expensive string formatting and JSON array construction.
**Action:** Implement conditional data fetching via query parameters (e.g., `?logs=1`). Wrap resource-intensive JSON construction blocks in conditional checks on the backend and update the frontend to request heavy data only when necessary (e.g., viewing the full Status page or generating support reports).

## 2026-03-31 - Rendering Throttling in LVGL
**Learning:** Calling `lv_label_set_text` on every rendering frame (e.g., during message scrolling) triggers expensive internal LVGL operations like re-parsing, memory re-allocation, and layout invalidation, even if the text content remains identical. This consumes significant CPU cycles in the main rendering loop.
**Action:** Always implement a `strcmp` guard (checking `lv_label_get_text`) before calling `lv_label_set_text` in high-frequency loops. This ensures that the library only performs work when the content actually changes.

## 2026-04-06 - Optimized Status API Serialization
**Learning:** Using `cJSON_Print()` for API responses on an embedded system is inefficient due to its "pretty-printing" overhead. The added whitespace and newlines increase heap allocation during string generation and inflate the network payload size without providing any benefit to the consuming web UI, which is agnostic to JSON formatting.
**Action:** Use `cJSON_PrintUnformatted()` for all JSON API responses. This reduces the heap memory requirement for the serialized string and shrinks the network payload by approximately 23% for standard status objects, improving both system stability and response latency.

## 2026-04-07 - Optimized Token Replacement in Scrolling Message
**Learning:** The previous implementation of `replace_placeholders` performed redundant string copies into a temporary buffer and used a nested O(N*M) loop to match dynamic integration tokens (HA/Stock). Additionally, it called `strlen` repeatedly inside the matching loop, which is a hot path for the scrolling display.
**Action:** Refactor `token_t` to include pre-calculated lengths. Iterate directly over the input string to eliminate the temporary buffer copy. Bind dynamic values directly in `prepare_tokens` to achieve O(N) replacement complexity. Use `memcpy` and `strlcpy` for faster string operations compared to `snprintf`.

## 2026-04-08 - High-Frequency Loop Optimization in Display Task
**Learning:** Frequent system calls and library updates in high-frequency tasks (e.g., the 65ms display loop) create significant CPU overhead. Specifically, restarting an `esp_timer` (watchdog) and calling `lv_obj_set_style_opa` (breathing dots) every iteration triggered expensive system context switches and internal LVGL invalidations, even when states remained identical.
**Action:** Replace `esp_timer_stop`/`start` with a simple `volatile` heartbeat counter that a slow periodic timer callback (30s) monitors for progress. Implement a state-and-value guard for breathing dots to ensure `lv_obj_set_style_opa` and port locks only occur on actual changes. This pattern preserves CPU cycles for critical rendering tasks.

## 2025-04-09 - Optimized Settings API Parameter Filtering
**Learning:** The Settings API was using a shared static buffer for query parameters, which led to a bug where multiple filters could not be applied simultaneously (the second filter would overwrite the first). Additionally, filtering logic performed O(N*M) string parsing and comparisons for every request, where N is the number of parameters (~55) and M is the number of tokens in the filter.
**Action:** Replace repeated string parsing with a one-time calculation of an inclusion bitmask. Refactor query parameter retrieval to use caller-provided buffers to eliminate shared state bugs. Use bitwise AND operations for O(1) inclusion checks per parameter.

## 2025-05-15 - Optimized Web UI Translation Engine
**Learning:** The translation engine in the web UI was performing redundant O(N) DOM operations (dataset lookups and innerHTML reads) for every translation pass, even when the content remained unchanged. With over 180 translatable elements, this caused measurable layout thrashing and CPU spikes during section navigation and language switching.
**Action:** Implement an element-specific metadata cache using a `WeakMap`. Store translation keys and last-applied values to enable dirty checking. This avoids unnecessary DOM mutations and property lookups. Hoist invariant translation lookups (e.g., common labels) outside of iteration loops to reduce redundant object traversals.
