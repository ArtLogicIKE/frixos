#!/usr/bin/env python3
"""Generate default-settings.jpg for the layout palette (32x32, Frixos copper style)."""

from pathlib import Path

from PIL import Image

OUT = Path(__file__).resolve().parent.parent / "spiffs" / "default-settings.jpg"

BG = (38, 38, 40)
COPPER = (196, 102, 58)
COPPER_DIM = (120, 58, 34)
COPPER_BRIGHT = (232, 140, 82)

SIZE = 32


def set_px(img, x, y, color):
    if 0 <= x < SIZE and 0 <= y < SIZE:
        img.putpixel((x, y), color)


def rect(img, x0, y0, x1, y1, color, fill=False):
    if fill:
        for y in range(y0, y1 + 1):
            for x in range(x0, x1 + 1):
                set_px(img, x, y, color)
    else:
        for x in range(x0, x1 + 1):
            set_px(img, x, y0, color)
            set_px(img, x, y1, color)
        for y in range(y0, y1 + 1):
            set_px(img, x0, y, color)
            set_px(img, x1, y, color)


def hline(img, y, x0, x1, color):
    for x in range(x0, x1 + 1):
        set_px(img, x, y, color)


def knob(img, cx, cy, color):
    for dy in (-1, 0, 1):
        for dx in (-1, 0, 1):
            if abs(dx) + abs(dy) <= 1:
                set_px(img, cx + dx, cy + dy, color)


def main():
    img = Image.new("RGB", (SIZE, SIZE), BG)

    # Display bezel (screen outline)
    rect(img, 5, 4, 26, 23, COPPER_DIM)
    rect(img, 6, 5, 25, 22, COPPER)

    # Inner screen area
    rect(img, 8, 7, 23, 20, BG, fill=True)

    # Three horizontal sliders — settings / tuning metaphor
    slider_rows = (10, 14, 18)
    knob_cols = (18, 11, 21)
    for y, kx in zip(slider_rows, knob_cols):
        hline(img, y, 9, 22, COPPER_DIM)
        hline(img, y, 9, 22, COPPER_DIM)
        knob(img, kx, y, COPPER_BRIGHT)
        set_px(img, kx, y, COPPER)

    # Small stand / base
    hline(img, 25, 13, 18, COPPER_DIM)
    set_px(img, 15, 26, COPPER_DIM)
    set_px(img, 16, 26, COPPER)
    set_px(img, 17, 26, COPPER_DIM)

    img.save(OUT, "JPEG", quality=92)
    print(f"Wrote {OUT}")


if __name__ == "__main__":
    main()
