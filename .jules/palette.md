## 2026-06-02 - Interactive Font Samples with Localized ARIA Labels

**Learning:** Static visual samples (like font previews) can be transformed into intuitive, accessible controls by making them interactive. Using a localized template with a placeholder (e.g., `common.select_font`: `Select {name} font`) allows for dynamic, grammatically correct ARIA labels that accurately describe the action for assistive technologies across multiple languages.

**Action:** When implementing visual previews that correspond to configuration settings, implement them as `role="button"` elements that programmatically update the associated form fields. Use a centralized translation key with placeholders to generate accessible labels for these dynamic elements.
