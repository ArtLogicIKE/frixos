# Palette's Journal - Critical UX & Accessibility Learnings

## 2025-05-15 - Password Visibility Toggle & i18n
**Learning:** Using `data-i18n` on elements with persistent visual content (like emojis or custom icons) causes the translation engine to overwrite the `innerHTML`, effectively removing the icon.
**Action:** For accessible icon-only elements, avoid `data-i18n` on the icon container. Instead, programmatically update the `aria-label` within the translation loop and interaction handlers using `getNestedTranslation` to ensure both localization and visual persistence.
