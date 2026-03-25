# Palette's Journal - Critical UX & Accessibility Learnings

This journal tracks critical UX and accessibility learnings discovered during the development of Frixos.

## 2025-05-15 - Password Visibility Toggle Pattern
**Learning:** Password visibility toggles should not use `data-i18n` directly on the button if they contain an icon (emoji), as the translation engine will overwrite the icon with the text label. Instead, accessibility labels (`aria-label`) must be updated programmatically in the translation loop using `getNestedTranslation` to ensure both the visual icon and screen reader support work correctly across all languages.
**Action:** Update the `translate` function in `index.js` to specifically handle `.password-toggle` elements and refresh their `aria-label` based on their current state (show/hide).
