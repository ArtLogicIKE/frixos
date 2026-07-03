"""Playwright smoke tests for the Frixos web UI."""

from __future__ import annotations

import sys
from playwright.sync_api import sync_playwright

BASE = sys.argv[1] if len(sys.argv) > 1 else "http://127.0.0.1:8080"


def run() -> int:
    failures: list[str] = []
    with sync_playwright() as p:
        browser = p.chromium.launch(headless=True)
        page = browser.new_page()
        page.goto(BASE, wait_until="networkidle")
        page.wait_for_timeout(1500)

        # Tab navigation & ARIA states
        for tab in ("integrations", "layout", "status", "settings"):
            tab_selector = f'#nav a[data-tab="{tab}"]'
            page.click(tab_selector)
            page.wait_for_timeout(400)
            if not page.locator(f"#tab-{tab}").evaluate("el => el.classList.contains('active')"):
                failures.append(f"Tab {tab} did not activate")
            if page.locator(tab_selector).get_attribute("aria-selected") != "true":
                failures.append(f"Tab {tab} aria-selected not true after click")

        # Keyboard navigation (Tab key to navigate, Enter to activate)
        page.keyboard.press("Tab") # Should focus Device tab (since it's first)
        # We might need multiple tabs to reach the nav if there are elements before it.
        # But for this UI, let's try to focus the first tab link.
        page.focus('#tab-link-settings')
        page.keyboard.press("ArrowRight") # Not implemented yet, we use Tab/Enter
        page.keyboard.press("Tab")
        page.keyboard.press("Enter") # Activate Integrations
        page.wait_for_timeout(400)
        if not page.locator("#tab-integrations").evaluate("el => el.classList.contains('active')"):
            failures.append("Integrations tab did not activate via keyboard")

        # Advanced dim mode panels (now in Settings tab)
        page.click('#nav a[data-tab="settings"]')
        # Find the details box that contains #dim_mode
        page.evaluate("document.querySelector('#dim_mode').closest('details').open = true")
        page.wait_for_timeout(200)
        page.select_option("#dim_mode", "0")
        if not page.locator("#dimBright").is_visible():
            failures.append("Brightness dim panel hidden for mode 0")
        page.select_option("#dim_mode", "2")
        if not page.locator("#dimTime").is_visible():
            failures.append("Time dim panel hidden for mode 2")

        # Language menu & Keyboard
        page.click("#langBtn")
        if page.locator("#langBtn").get_attribute("aria-expanded") != "true":
            failures.append("langBtn aria-expanded not true after click")

        # Test keyboard activation of language option
        page.focus('.lang-opt[data-lang="de"]')
        page.keyboard.press("Enter")
        page.wait_for_timeout(500)
        if page.locator("#langName").inner_text() != "Deutsch":
            failures.append("Language switch to German via keyboard failed")
        if page.locator("#langBtn").get_attribute("aria-expanded") != "false":
            failures.append("langBtn aria-expanded not false after selection")

        # Theme toggle
        was_dark = page.evaluate("document.body.classList.contains('theme-dark')")
        page.click("#themeBtn")
        page.wait_for_timeout(300)
        is_dark = page.evaluate("document.body.classList.contains('theme-dark')")
        if was_dark == is_dark:
            failures.append("Theme toggle did not change body class")

        browser.close()

    for f in failures:
        print(f"FAIL {f}")
    passed = 6 - len(failures)  # approximate check count
    print(f"\n{passed} checks passed, {len(failures)} failed")
    return 1 if failures else 0


if __name__ == "__main__":
    raise SystemExit(run())
