from playwright.sync_api import Page, expect, sync_playwright
import time

def verify_translations(page: Page):
    # Navigate to mock server
    page.goto("http://127.0.0.1:8080")
    page.wait_for_timeout(2000) # Wait for initial load and theme/settings fetch

    # Take screenshot of English (default)
    page.screenshot(path="/home/jules/verification/english.png")
    print("Screenshot of English taken.")

    # Toggle language dropdown
    page.click("#language-toggle")
    page.wait_for_timeout(500)

    # Change to German (de)
    page.click('[data-lang="de"]')
    page.wait_for_timeout(2000) # Wait for lazy loading and translation

    # Take screenshot of German
    page.screenshot(path="/home/jules/verification/german.png")
    print("Screenshot of German taken.")

    # Navigate to Advanced section
    page.click('a[href="#advanced"]')
    page.wait_for_timeout(500)

    # Enter some text to verify counter
    page.fill("#message", "Hello Frixos!")
    page.wait_for_timeout(500)

    # Take screenshot of Advanced section with counter
    page.screenshot(path="/home/jules/verification/advanced_counter.png")
    print("Screenshot of Advanced counter taken.")

    # Navigate to Integrations section
    page.click('a[href="#integrations"]')
    page.wait_for_timeout(1000)

    # Take screenshot of Integrations section with toggles
    page.screenshot(path="/home/jules/verification/integrations_toggles.png", full_page=True)
    print("Screenshot of Integrations toggles taken.")

    # Final screenshot for verification function
    page.screenshot(path="/home/jules/verification/verification.png")

if __name__ == "__main__":
    with sync_playwright() as p:
        browser = p.chromium.launch(headless=True)
        context = browser.new_context(record_video_dir="/home/jules/verification/video")
        page = context.new_page()
        try:
            verify_translations(page)
        finally:
            context.close()
            browser.close()
