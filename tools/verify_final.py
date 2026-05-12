from playwright.sync_api import Page, expect, sync_playwright
import os

def verify_language_selector_final(page: Page):
    os.makedirs("/home/jules/verification", exist_ok=True)
    page.goto("http://127.0.0.1:8080")
    page.wait_for_timeout(2000)

    # Open dropdown
    toggle = page.locator("#language-toggle")
    toggle.click()
    page.wait_for_timeout(500)

    expect(toggle).to_have_attribute("aria-expanded", "true")

    # Check active state
    en_option = page.locator('.language-option[data-lang="en"]')
    expect(en_option).to_have_class("language-option is-active")
    expect(en_option).to_have_attribute("aria-selected", "true")

    page.screenshot(path="/home/jules/verification/final_verification.png")

    # Select another language
    de_option = page.locator('.language-option[data-lang="de"]')
    de_option.click()
    page.wait_for_timeout(2000)

    expect(toggle).to_have_attribute("aria-expanded", "false")

    toggle.click()
    page.wait_for_timeout(500)
    expect(de_option).to_have_class("language-option is-active")

if __name__ == "__main__":
    with sync_playwright() as p:
        browser = p.chromium.launch(headless=True)
        page = browser.new_page()
        try:
            verify_language_selector_final(page)
        finally:
            browser.close()
