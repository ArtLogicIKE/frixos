#!/usr/bin/env python3
"""Generate the editor icons for the generic Graph widget.

Produces:
  spiffs/palette-graph.png  - palette tile (matches palette-message.png style)
  spiffs/default-graph.jpg  - on-canvas placeholder (graph aspect ~ 90x40)

Run: python tools/gen_graph_icon.py
"""
from pathlib import Path
from PIL import Image, ImageDraw

SPIFFS = Path(__file__).resolve().parent.parent / "spiffs"

BG = (10, 14, 18)
AXIS = (120, 120, 120)
LINE = (0, 220, 255)   # cyan, matches default graph line colour
BAND = (40, 60, 40)


def draw_graph(w, h, supersample=4):
    W, H = w * supersample, h * supersample
    img = Image.new("RGB", (W, H), BG)
    d = ImageDraw.Draw(img)

    m = int(min(W, H) * 0.16)        # margin
    x0, y0 = m, m // 2
    x1, y1 = W - m // 2, H - m

    # subtle target band
    by0 = y0 + int((y1 - y0) * 0.32)
    by1 = y0 + int((y1 - y0) * 0.60)
    d.rectangle([x0, by0, x1, by1], fill=BAND)

    # axes
    aw = max(1, supersample)
    d.line([x0, y0, x0, y1], fill=AXIS, width=aw)        # y axis
    d.line([x0, y1, x1, y1], fill=AXIS, width=aw)        # x axis

    # polyline: a readable rise/dip/rise
    pts_norm = [0.0, 0.55, 0.30, 0.70, 0.45, 0.92, 0.75]
    n = len(pts_norm)
    pts = []
    for i, v in enumerate(pts_norm):
        x = x0 + (x1 - x0) * i / (n - 1)
        y = y1 - (y1 - y0) * v
        pts.append((x, y))
    d.line(pts, fill=LINE, width=max(2, supersample + 1), joint="curve")
    r = supersample + 1
    for (x, y) in pts:
        d.ellipse([x - r, y - r, x + r, y + r], fill=LINE)

    return img.resize((w, h), Image.LANCZOS)


def main():
    palette = draw_graph(160, 60)
    palette.save(SPIFFS / "palette-graph.png", "PNG", optimize=True)
    print("Wrote", SPIFFS / "palette-graph.png")

    canvas = draw_graph(180, 80)  # 2x of 90x40
    canvas.save(SPIFFS / "default-graph.jpg", "JPEG", quality=92)
    print("Wrote", SPIFFS / "default-graph.jpg")


if __name__ == "__main__":
    main()
