#!/usr/bin/env python3
"""Generate bold-digit-label.jpg: Digit Display preview with a thick red underline."""

from pathlib import Path

from PIL import Image, ImageDraw

SPIFFS = Path(__file__).resolve().parent.parent / "spiffs"
SRC = SPIFFS / "bold.jpg"
OUT = SPIFFS / "bold-digit-label.jpg"

RED = (226, 59, 46)  # matches --proj-red / frixos alarm red
LINE_THICKNESS = 3
GAP = 1  # px between digit bottom and line top


def digit_bounds(img):
    w, h = img.size
    x0, y0, x1, y1 = w, h, 0, 0
    found = False
    for y in range(h):
        for x in range(w):
            if img.getpixel((x, y))[0] > 200:
                found = True
                x0 = min(x0, x)
                y0 = min(y0, y)
                x1 = max(x1, x)
                y1 = max(y1, y)
    if not found:
        return 0, 0, w - 1, h - 1
    return x0, y0, x1, y1


def main():
    img = Image.open(SRC).convert("RGB")
    x0, y0, x1, y1 = digit_bounds(img)
    draw = ImageDraw.Draw(img)
    line_y0 = min(y1 + GAP + 1, img.height - LINE_THICKNESS)
    draw.rectangle([x0, line_y0, x1, line_y0 + LINE_THICKNESS - 1], fill=RED)
    img.save(OUT, "JPEG", quality=92)
    print(f"Wrote {OUT}")


if __name__ == "__main__":
    main()
