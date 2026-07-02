#!/usr/bin/env python3
"""Generate the editor icons for the generic Graph widget.

Produces:
  spiffs/palette-graph.png  - palette tile (matches palette-message.png style)
  spiffs/default-graph.jpg  - on-canvas placeholder (graph aspect ~ 90x40)

The geometry mirrors what the firmware actually draws (f-display.c) for the
default 80x36 widget with both axes on: a 17px label gutter left of the Y
axis, an 8px time-label gutter under the X axis, 1px top/right margins and
no top or right border. Axis label text is stood in by short dashes.

Run: python tools/gen_graph_icon.py
"""
from pathlib import Path
from PIL import Image, ImageDraw

SPIFFS = Path(__file__).resolve().parent.parent / "spiffs"

BG = (10, 14, 18)
AXIS = (120, 120, 120)
LINE = (0, 220, 255)   # cyan, matches default graph line colour
BAND = (40, 60, 40)

# Reference widget the proportions are taken from (editor default, f-display.c
# margins with both axes shown): plot x 17..78, y 1..27, axis lines at x=16 /
# y=28, Y labels in the 0..15 gutter, X labels in rows 29..35.
REF_W, REF_H = 80.0, 36.0


def draw_graph(w, h, supersample=4):
    W, H = w * supersample, h * supersample
    img = Image.new("RGB", (W, H), BG)
    d = ImageDraw.Draw(img)

    sx, sy = W / REF_W, H / REF_H
    px1 = round(17 * sx)              # plot left (right of the Y-label gutter)
    px2 = round(78 * sx)              # plot right (1px widget margin, no border)
    py1 = round(1 * sy)               # plot top (1px widget margin, no border)
    py2 = round(27 * sy)              # plot bottom (above the X-label gutter)
    ax = px1 - max(1, round(sx))      # Y-axis line
    ay = py2 + max(1, round(sy))      # X-axis line

    lw = max(1, supersample)

    # subtle target band (spans the plot only, not the gutters)
    by0 = py1 + int((py2 - py1) * 0.32)
    by1 = py1 + int((py2 - py1) * 0.60)
    d.rectangle([px1, by0, px2, by1], fill=BAND)

    # axes: Y down the left of the plot, X along the bottom, joined at the
    # bottom-left corner; the top and right edges stay open like the widget
    d.line([ax, py1, ax, ay], fill=AXIS, width=lw)
    d.line([ax, ay, px2, ay], fill=AXIS, width=lw)

    # label stand-ins (Tiny5 text on device): ymax/ymin right-aligned in the
    # left gutter, window span ("-12h") and "now" under the X axis
    th = max(lw, round(3 * sy))       # dash thickness ~ label text height
    gap = max(1, round(2 * sx))

    def dash(x0, x1_, y):
        d.rectangle([x0, y, x1_, y + th], fill=AXIS)

    dash(ax - gap - round(8 * sx), ax - gap, py1)          # ymax label
    dash(ax - gap - round(8 * sx), ax - gap, py2)          # ymin label
    dash(px1, px1 + round(9 * sx), ay + max(1, round(sy))) # "-12h" label
    dash(px2 - round(7 * sx), px2 - gap, ay + max(1, round(sy)))  # "now" label

    # polyline: a readable rise/dip/rise, inset 2px from the plot edges the
    # same way the firmware insets the curve (#188)
    cy1 = py1 + round(2 * sy)
    cy2 = py2 - round(2 * sy)
    pts_norm = [0.0, 0.55, 0.30, 0.70, 0.45, 0.92, 0.75]
    n = len(pts_norm)
    pts = []
    for i, v in enumerate(pts_norm):
        x = px1 + (px2 - px1) * i / (n - 1)
        y = cy2 - (cy2 - cy1) * v
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
