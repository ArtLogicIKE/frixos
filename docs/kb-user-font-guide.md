# Creating a Custom User Font for Frixos

This guide explains how to create and upload custom fonts for your Frixos daylight projection clock. You can either modify an existing font (such as Bold) or design your own from scratch.

---

## Table of Contents

1. [Understanding the Font Format](#understanding-the-font-format)
2. [Modifying an Existing Font (Using Bold as a Template)](#modifying-an-existing-font-using-bold-as-a-template)
3. [Designing Your Own Font from Scratch](#designing-your-own-font-from-scratch)
4. [Theme Elements You Can Customize](#theme-elements-you-can-customize)
5. [Uploading Your Custom Font](#uploading-your-custom-font)
6. [Setting Day and Night Fonts](#setting-day-and-night-fonts)

---

## Understanding the Font Format

Frixos displays the time using a **sprite sheet**—a single image containing all ten digits (0–9) arranged horizontally. The display engine crops each digit from this sheet as needed.

### Font Sprite Sheet Dimensions

| Property | Value | Description |
|----------|-------|-------------|
| **Digit width** | 18 pixels | Width of each digit cell |
| **Digit height** | 36 pixels | Height of each digit cell |
| **Total width** | 180 pixels | 10 digits × 18 px |
| **Total height** | 36 pixels | Single row |
| **Format** | JPG or BMP | JPEG recommended for smaller file size |

The digits must appear in order from left to right: **0, 1, 2, 3, 4, 5, 6, 7, 8, 9**. Each digit occupies exactly 18×36 pixels.

```
┌────┬────┬────┬────┬────┬────┬────┬────┬────┬────┐
│ 0  │ 1  │ 2  │ 3  │ 4  │ 5  │ 6  │ 7  │ 8  │ 9  │
│18px│18px│18px│18px│18px│18px│18px│18px│18px│18px│
└────┴────┴────┴────┴────┴────┴────┴────┴────┴────┘
  ←────────────────── 180 px total ──────────────────→
  ←── 36 px height ──→
```

---

## Modifying an Existing Font (Using Bold as a Template)

The easiest way to create a custom font is to start with an existing one. **Bold** is a good choice because it’s widely used and well-tested.

### Step 1: Get the Bold Font Image

1. Connect to your Frixos device’s web interface.
2. Open the **Font Samples** section to see preview images.
3. The built-in Bold font is stored at `themes/bold/font.jpg` on the device. You can also use the sample image from the web UI (e.g. `themes/bold/preview.jpg`) as a reference—the font sprite uses the same layout.

### Step 2: Create Your Modified Version

1. **Copy the Bold font file**  
   Use `themes/bold/font.jpg` as your starting point. If you don’t have direct access to the file, create a new 180×36 image and use the Bold sample as a visual reference.

2. **Edit in an image editor** (e.g. GIMP, Photoshop, Photopea):
   - Open the 180×36 image.
   - Use a grid or guides: 18 px columns, 36 px rows.
   - Edit each digit (0–9) within its 18×36 cell.
   - Keep digits aligned so they don’t overlap or shift into adjacent cells.

3. **Export**  
   Save as JPG (or BMP). Keep the exact size: **180×36 pixels**.

### Step 3: Name Your File Correctly

For **User1** font slot:
- Filename: `user1-font.jpg`

For **User2** font slot:
- Filename: `user2-font.jpg`

The device looks for `{theme}-font.jpg`, so `user1` and `user2` map to these filenames.

---

## Designing Your Own Font from Scratch

If you prefer to design a font from scratch:

### Dimensions Reference

| Dimension | Value |
|-----------|-------|
| Canvas width | 180 px |
| Canvas height | 36 px |
| Cell width | 18 px |
| Cell height | 36 px |
| Number of cells | 10 (digits 0–9) |

### Design Tips

1. **Use a grid**  
   Enable an 18×36 grid in your editor so each digit stays within its cell.

2. **Consider projection**  
   Frixos projects the display. Simple, high-contrast designs usually look best.

3. **Test readability**  
   Digits should be clear at small sizes. Avoid very thin strokes or fine details.

4. **Background**  
   Use a solid background (e.g. black). Transparent areas may not render as expected.

5. **Color**  
   The device applies color filters (red, green, blue, B&W) per theme. Design in grayscale or a single color for predictable results.

### Export Settings

- **Format:** JPG (recommended) or BMP
- **Size:** 180×36 px
- **Color mode:** RGB or grayscale

---

## Theme Elements You Can Customize

Frixos uses a **theme system**: when you select a font (e.g. Bold, User1, Nixie), the device loads matching assets for that theme. You can provide custom versions of these elements for your user font.

### Theme File Naming

Theme assets are stored in `themes/<theme>/`, with the element name as the filename. For example, User1 assets are stored as `themes/user1/font.jpg`, `themes/user1/weather.jpg`, and `themes/user1/moon.jpg`.

For **User1**, examples:
- `themes/user1/font.jpg` – time digits
- `themes/user1/weather.jpg` – weather icons
- `themes/user1/moon.jpg` – moon phases
- `themes/user1/dot.jpg` – time separator dots
- `themes/user1/ampm.jpg` – AM/PM indicator
- `themes/user1/glucose.jpg` – glucose icon (CGM)
- `themes/user1/trend.jpg` – glucose trend arrows
- `themes/user1/units.jpg` – mg/dL / mmol/L label
- `user1-wifi-off.jpg` – WiFi disabled icon

Replace `user1` with `user2` for the second user font slot.

### Element Dimensions

| Element | File | Width | Height | Frames | Total Width | Notes |
|--------|------|-------|--------|--------|-------------|-------|
| **Font** | `themes/<theme>/font.jpg` | 18 | 36 | 10 | 180 | Digits 0–9 |
| **Weather** | `themes/<theme>/weather.jpg` | 32 | 22 | 8 | 256 | OpenWeatherMap icons |
| **Moon** | `themes/<theme>/moon.jpg` | 14 | 14 | 8 | 112 | Moon phases 0–7 |
| **Dot** | `themes/<theme>/dot.jpg` | varies | varies | 1 | — | Time separator |
| **AM/PM** | `themes/<theme>/ampm.jpg` | 10 | 20 | 2 | 20 | AM, PM |
| **Glucose** | `themes/<theme>/glucose.jpg` | 14 | 20 | 4 | 56 | Red, green, yellow, gray |
| **Trend** | `themes/<theme>/trend.jpg` | 12 | 14 | 5 | 60 | Trend arrows |
| **Units** | `themes/<theme>/units.jpg` | 12 | 24 | 2 | 24 | mg/dL, mmol/L |
| **WiFi-off** | `themes/<theme>/wifi-off.jpg` | 20 | 20 | 1 | 20 | WiFi disabled icon |

### Fallback Behavior

If a theme-specific file is missing (e.g. `themes/user1/weather.jpg`), the device falls back to `themes/default/weather.jpg`. So you only need to create the assets you want to customize.

### Sample: Custom Weather Icons

The weather sprite has 8 horizontal frames (32×22 px each):

| Index | OpenWeatherMap | Condition |
|-------|-----------------|------------|
| 0 | 01d/01n | Clear sky |
| 1 | 02d/02n | Few clouds |
| 2 | 03d/03n | Scattered clouds |
| 3 | 04d/04n | Broken clouds |
| 4 | 09/10 | Rain |
| 5 | 11 | Thunderstorm |
| 6 | 13 | Snow |
| 7 | 50 | Mist/fog |

Create a 256×22 px image with 8 icons side by side.

### Sample: Custom Moon Phases

The moon sprite has 8 phases (14×14 px each):

| Index | Phase |
|-------|-------|
| 0 | New moon |
| 1 | Waxing crescent |
| 2 | First quarter |
| 3 | Waxing gibbous |
| 4 | Full moon |
| 5 | Waning gibbous |
| 6 | Last quarter |
| 7 | Waning crescent |

Create a 112×14 px image with 8 moon icons in order.

---

## Uploading Your Custom Font

### Prerequisites

- Frixos device connected to your Wi‑Fi network
- Access to the device’s web interface (e.g. `http://frixos.local` or its IP address)

### Upload Steps

1. **Open the Update section**
   - In the web UI, go to **Update** (or **Upload Firmware / Files**).

2. **Select your file**
   - Click **Choose File** (or **Select File**).
   - Select your custom font file (e.g. `user1-font.jpg` or `user2-font.jpg`).
   - Ensure the filename is exactly `user1-font.jpg` or `user2-font.jpg` so the device recognizes it.

3. **Upload**
   - Click **Upload File**.
   - Wait for the upload to complete. Do not power off or disconnect the device during the upload.

4. **Confirmation**
   - When you see “File upload successful!”, the file is stored on the device.

5. **Optional: upload more assets**
   - You can upload more theme files (e.g. `user1-weather.jpg`, `user1-moon.jpg`) using the same process.

### Important Notes

- For recognized theme asset names, the web UI stores the uploaded file under `themes/<theme>/<element>.<extension>`. Rename your file to `user1-font.jpg` or `user2-font.jpg` before uploading if needed.
- Non‑binary files (e.g. `.bin` firmware) are handled differently; font and theme files are uploaded as regular files to SPIFFS.
- After upload, the display refreshes automatically. If you’ve selected User1 or User2 as the active font, you should see your custom digits immediately.

---

## Setting Day and Night Fonts

Frixos supports different fonts for day and night. You can use your custom font for one or both.

### Steps

1. **Open the web UI**
   - Connect to your Frixos device’s web interface.

2. **Go to Advanced settings**
   - Open the **Advanced** section.

3. **Day font**
   - Find **Day Settings** → **Font**.
   - Select **User1** or **User2** from the dropdown.

4. **Night font**
   - Find **Night Settings** → **Font**.
   - Select **User1** or **User2** from the dropdown.

5. **Save**
   - Click **Save** (or equivalent) to apply the settings.

The device switches between day and night fonts based on your configured sunrise/sunset or schedule.

### Font Options

Available fonts include:

- **Built-in:** Bold, Light, LCD, Nixie, Robrito, Ficasso, Lichten, Kablame, Kablamo, Kaboom
- **User slots:** User1, User2

User1 and User2 use the files `user1-font.jpg` and `user2-font.jpg` (and any other `user1-*` or `user2-*` theme assets you upload).

---

## Quick Reference

| Task | File to upload | Stored as |
|------|----------------|-----------|
| Custom digits for User1 | `user1-font.jpg` | `themes/user1/font.jpg` |
| Custom digits for User2 | `user2-font.jpg` | `themes/user2/font.jpg` |
| Custom weather icons | `user1-weather.jpg` or `user2-weather.jpg` | `themes/user1/weather.jpg` or `themes/user2/weather.jpg` |
| Custom moon phases | `user1-moon.jpg` or `user2-moon.jpg` | `themes/user1/moon.jpg` or `themes/user2/moon.jpg` |

---

## Troubleshooting

- **Font not showing:**  
  Check that the filename is exactly `user1-font.jpg` or `user2-font.jpg` and that the image is 180×36 pixels.

- **Digits misaligned:**  
  Ensure each digit is drawn within its 18×36 cell and no cell overlaps another.

- **Wrong colors:**  
  Color filters (Advanced → Color Filter) affect the display. Try “None” to see your design without filtering.

- **Upload fails:**  
  Ensure the device is powered and connected to Wi‑Fi, and that the file size is within limits (typically under 2.5 MB for non‑firmware uploads).
