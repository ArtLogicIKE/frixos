# Palette Journal

## 2024-05-29 - Interactive and Accessible Font Samples
**Learning:** Static visual samples (like font previews) are often perceived as interactive buttons by users. Providing a direct "click-to-apply" behavior on these samples, paired with proper ARIA roles and keyboard support, significantly reduces interaction cost compared to using separate dropdown menus.
**Action:** Always evaluate if visual configuration previews can be made interactive. Ensure any new interactive element has a role="button", tabindex="0", and localized aria-label that describes the action (e.g., "Select [Font] font").
