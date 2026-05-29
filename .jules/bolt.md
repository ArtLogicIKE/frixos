## 2025-05-14 - Dataset Access and Dirty Checking in Translation Loops

**Learning:** Accessing `element.dataset` in a loop over 190+ elements for every language switch incurs significant overhead due to the creation and parsing of the `DOMStringMap`. Combined with redundant DOM writes for content that hasn't changed, this leads to measurable layout thrashing.

**Action:** Pre-parse dataset attributes into a static array of objects during the first translation pass. Implement dirty checking (`el.innerHTML !== translation`) to skip unnecessary DOM updates. This pattern should be applied to any high-frequency or bulk UI update logic in this codebase.
