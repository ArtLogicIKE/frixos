# Palette's Journal - Frixos Project

## 2025-05-14 - Initial Observations
**Learning:** The Frixos web interface is a solid ESP32 configuration portal with a clean dark/light theme. However, it lacks some common micro-UX patterns that enhance usability, such as password visibility toggles and more interactive form elements. The status messages are displayed but not marked as ARIA live regions, which is a missed accessibility opportunity.
**Action:** Focus on high-impact micro-UX changes like password visibility toggles that directly improve usability for common tasks (WiFi setup). Ensure toggle buttons use dynamic ARIA labels for better screen reader support.

## 2025-05-14 - [Multilingual ARIA Label Pattern]
**Learning:** In a vanilla JS translation engine that uses `innerHTML` for localization, icon-only buttons (like password toggles) present a challenge: translations would overwrite the icon. The most robust UX pattern for this app is using a `data-i18n-aria-label` attribute and programmatically updating the `aria-label` within the `translate` loop.
**Action:** Always use `data-i18n-aria-label` for localizing interactive elements that must preserve their visual content (icons/emojis) while remaining accessible to screen readers.

## 2025-05-14 - [A11y: Live Regions for Feedback]
**Learning:** In a configuration portal where settings are saved asynchronously, users (especially those using screen readers) need immediate confirmation. Marking the status message container as an ARIA live region ensures that save confirmations and errors are announced immediately.
**Action:** Always use `role="status"` and `aria-live="polite"` on central feedback elements to improve non-visual feedback.

## 2025-05-14 - [CSS: Flexbox Wrapping for Multi-element Fields]
**Learning:** When a `.form-group` uses `flex-wrap: nowrap`, secondary elements like validation errors or metadata masks are forced onto the same line, causing horizontal overflow or layout breakage.
**Action:** Use `flex-wrap: wrap` on form group containers and give secondary elements `flex-basis: 100%` to ensure they correctly stack below the primary input field.
