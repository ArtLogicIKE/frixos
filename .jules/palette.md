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

## 2025-05-16 - [Visual Samples as Interactive Controls]
**Learning:** When visual samples (such as fonts) correspond to underlying configuration settings, they should be implemented as accessible, interactive controls. This reduces cognitive load by allowing users to directly interact with what they see, rather than translating a visual preference into a dropdown selection.
**Action:** Implement `role="button"`, `tabindex="0"`, and localized `aria-label` for all visual sample boxes to ensure they are discoverable and usable as primary input mechanisms.

## 2026-06-22 - [Accessible Menu State Management]
**Learning:** When implementing interactive menus in vanilla JS, failing to reset the `aria-expanded` attribute to `false` when the menu closes via item selection or clicking outside creates a confusing experience for screen reader users. Ensuring this state is always synchronized with the visual visibility of the menu is a high-impact micro-UX win for accessibility.
**Action:** Always include explicit `aria-expanded` reset logic in both item selection handlers and "click-outside" document listeners for accessible menus.

## 2026-06-23 - [Character Counter Accessibility]
**Learning:** Adding character counters is a great UX win, but they must be accessible. Using `aria-describedby` to link the input to its counter and `aria-live="polite"` on the counter ensures screen reader users are kept informed of the remaining space as they type.
**Action:** Always pair character counters with appropriate ARIA attributes (`aria-describedby` and `aria-live`) to ensure accessibility for all users.

## 2026-06-24 - [Semantic Toggle Switches]
**Learning:** Using a simple `.on` class for visual toggle states is insufficient for accessibility. Implementing `role="switch"` and `aria-checked` ensures that assistive technologies correctly announce the element's purpose and state. For buttons acting as switches, `type="button"` is also essential to prevent default form submission behavior.
**Action:** Always complement visual state classes (like `.on` or `.active`) with semantic ARIA attributes (`role="switch"`, `aria-checked`) and ensure explicit `type="button"` on toggle controls.

## 2025-05-18 - [Vanilla JS Keyboard Navigation Pattern]
**Learning:** In a vanilla JS SPA that uses non-native elements (like `div` or `a` without `href`) for critical navigation components like tabs and custom menus, standard browser keyboard interactions are lost. Implementing the WAI-ARIA `tablist`/`tab` and `listbox`/`option` patterns manually is essential. This includes adding `tabindex="0"`, appropriate `role` attributes, and manual `keydown` listeners for `Enter` and `Space` to maintain parity with native controls.
**Action:** For every custom interactive component, explicitly define its ARIA role, manage `tabindex` for focusability, and implement keyboard listeners that mirror native activation behavior.
