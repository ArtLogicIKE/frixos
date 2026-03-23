## 2025-03-24 - Optimized Character Width Caching in Firmware
**Learning:** The previous character width cache implementation used a linear search ((N)$) and performed redundant string scans ((L)$) inside the character measurement loop, resulting in (L^2)$ complexity for string measurement. This is a significant bottleneck for real-time scrolling text on embedded systems.
**Action:** Use direct-mapped hash lookups ((1)$) for Unicode code points and eliminate inner-loop string operations. Avoid "double-check" logic that bypasses the cache for non-ASCII characters. Mark unused legacy parameters with `(void)` to maintain API compatibility without warnings.

## 2026-03-23 - Persistent DOM Element Caching Pitfall
**Learning:** Caching DOM elements lazily in a global variable for translation prevents newly added dynamic elements from being localized. While it avoids repeated `querySelectorAll` calls, it introduces a functional regression in apps with dynamic content.
**Action:** Prefer DOM mutation diffing (comparing new values against `innerHTML`) to avoid layout thrashing without sacrificing the ability to translate dynamic elements.
