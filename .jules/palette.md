# Palette's Journal - Frixos Project

## 2025-05-14 - Initial Observations
**Learning:** The Frixos web interface is a solid ESP32 configuration portal with a clean dark/light theme. However, it lacks some common micro-UX patterns that enhance usability, such as password visibility toggles and more interactive form elements. The status messages are displayed but not marked as ARIA live regions, which is a missed accessibility opportunity.
**Action:** Focus on high-impact micro-UX changes like password visibility toggles that directly improve usability for common tasks (WiFi setup). Ensure toggle buttons use dynamic ARIA labels for better screen reader support.

## 2025-05-14 - [Multilingual ARIA Label Pattern]
**Learning:** In a vanilla JS translation engine that uses `innerHTML` for localization, icon-only buttons (like password toggles) present a challenge: translations would overwrite the icon. The most robust UX pattern for this app is using a `data-i18n-aria-label` attribute and programmatically updating the `aria-label` within the `translate` loop.
**Action:** Always use `data-i18n-aria-label` for localizing interactive elements that must preserve their visual content (icons/emojis) while remaining accessible to screen readers.

## 2025-05-15 - [Programmatic A11y Initialization]
**Learning:** For legacy or vanilla JS projects with many forms, programmatically associating inputs with their descriptions (errors, counters, help text) using `aria-describedby` in a central `initA11y` function is more maintainable and less error-prone than manual HTML updates.
**Action:** Implement a central accessibility initialization helper that uses naming conventions (e.g., `id` vs `id-error`) to link UI elements for assistive technologies.

## 2025-05-15 - [Credential UX vs Security]
**Learning:** While `autocomplete` attributes like `current-password` and `username` greatly improve UX for WiFi and user accounts, they should be omitted for API tokens and integration keys to prevent password managers from incorrectly identifying them as primary site credentials.
**Action:** Selectively apply `autocomplete` only to standard user/password fields and avoid them for technical tokens or machine keys.

## 2025-05-16 - [Keyboard Accessible Lists]
**Learning:** Making dynamic lists (like WiFi networks) keyboard accessible requires `tabindex="0"`, `role="button"`, and explicit keyboard listeners (`Enter`, `Space`) because simple `div` elements with `onclick` are invisible to keyboard-only users.
**Action:** When creating interactive list items dynamically, ensure all ARIA roles and keyboard listeners are applied at creation time, and provide visible focus indicators in CSS.
