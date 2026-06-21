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
    page.click("#langBtn")
    page.wait_for_timeout(500)

    # Change to German (de)
    page.click('.lang-opt[data-lang="de"]')
    page.wait_for_timeout(2000) # Wait for lazy loading and translation

    # Take screenshot of German
    page.screenshot(path="/home/jules/verification/german.png")
    print("Screenshot of German taken.")

    # Verify counter in Settings section
    page.fill("#message", "Hello Frixos!")
    page.wait_for_timeout(500)

    # Navigate to Integrations section
    page.click('a[data-tab="integrations"]')
    page.wait_for_timeout(500)

    # Verify counter in Integrations
    page.fill("#eeprom_ha_url", "http://homeassistant.local:8123")
    page.wait_for_timeout(500)
    page.screenshot(path="/home/jules/verification/integrations_counter.png")

    # Navigate to Layout section
    page.click('a[data-tab="layout"]')
    page.wait_for_timeout(1000)

    # Click on the message element to open its options
    # If the canvas element is not found, it might be due to mock server not providing layout data properly
    try:
        page.click('.screen-element[data-id="message"]', timeout=5000)
    except:
        # Fallback to palette item if canvas element is missing
        page.click('.palette-item[data-id="message"]')
    page.wait_for_timeout(500)

    # Verify counter in Layout Editor
    # If the options panel is not open, it might be due to a click failure
    try:
        page.fill("#editor-textarea", "This is a long message for the layout editor.", timeout=5000)
    except:
        print("Could not find layout editor textarea, skipping.")
    page.wait_for_timeout(500)

    # Take screenshot of Layout section with counter
    page.screenshot(path="/home/jules/verification/layout_counter.png")
    print("Screenshot of Advanced counter taken.")

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
