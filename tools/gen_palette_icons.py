#!/usr/bin/env python3
"""Generate high-quality palette icons for the layout editor."""

from __future__ import annotations

import math
from pathlib import Path

from PIL import Image, ImageDraw, ImageFilter, ImageFont

SPIFFS = Path(__file__).resolve().parent.parent / "spiffs"

BLACK = (0, 0, 0)
MOON_LIGHT = (228, 228, 220)
MOON_SHADOW = (18, 18, 20)
RED = (226, 59, 46)
WHITE = (255, 255, 255)
GRAY = (120, 120, 120)


def save(img: Image.Image, name: str) -> None:
    path = SPIFFS / name
    if name.endswith(".png"):
        img.save(path, "PNG", optimize=True)
    else:
        img.save(path, "JPEG", quality=95)
    print(f"Wrote {path}")


def draw_waxing_gibbous(size: int = 128) -> Image.Image:
    img = Image.new("RGB", (size, size), BLACK)
    draw = ImageDraw.Draw(img)
    pad = size // 10
    box = [pad, pad, size - pad - 1, size - pad - 1]
    draw.ellipse(box, fill=MOON_LIGHT)
    r = (box[2] - box[0]) / 2
    cx = (box[0] + box[2]) / 2
    cy = (box[1] + box[3]) / 2
    # Small shadow on the left — mostly lit disk (waxing gibbous)
    shadow_cx = cx - r * 0.42
    shadow_r = r * 0.96
    draw.ellipse(
        [shadow_cx - shadow_r, cy - shadow_r, shadow_cx + shadow_r, cy + shadow_r],
        fill=BLACK,
    )
    return img


def draw_wifi_off(size: int = 128) -> Image.Image:
    img = Image.new("RGBA", (size, size), (0, 0, 0, 255))
    draw = ImageDraw.Draw(img)
    cx, cy = size // 2, size // 2 + 4
    # WiFi arcs
    for i, radius in enumerate((46, 32, 18)):
        bbox = [cx - radius, cy - radius, cx + radius, cy + radius]
        draw.arc(bbox, start=200, end=340, fill=WHITE, width=5)
    draw.ellipse([cx - 5, cy + 2, cx + 5, cy + 12], fill=WHITE)
    # Prohibition ring + slash
    ring = size // 2 - 6
    draw.ellipse([cx - ring, cy - ring - 6, cx + ring, cy + ring - 6], outline=RED, width=6)
    draw.line([cx - ring + 8, cy - ring - 2, cx + ring - 8, cy + ring - 10], fill=RED, width=6)
    return img.convert("RGB")


def _paste_centered(canvas: Image.Image, icon: Image.Image, margin: float = 0.88) -> Image.Image:
    size = canvas.width
    scale = min(size / icon.width, size / icon.height) * margin
    new_w = max(1, int(icon.width * scale))
    new_h = max(1, int(icon.height * scale))
    scaled = icon.resize((new_w, new_h), Image.Resampling.NEAREST)
    canvas.paste(scaled, ((size - new_w) // 2, (size - new_h) // 2))
    return canvas


def _thicken_white_on_black(img: Image.Image, passes: int = 4) -> Image.Image:
    mask = img.convert("L").point(lambda p: 255 if p > 128 else 0)
    for _ in range(passes):
        mask = mask.filter(ImageFilter.MaxFilter(3))
    out = Image.new("RGB", img.size, BLACK)
    out.paste(WHITE, mask=mask)
    return out


def draw_glucose_level(size: int = 128) -> Image.Image:
    """Green in-range drop with checkmark from default-glucose.jpg (2nd icon)."""
    src = Image.open(SPIFFS / "default-glucose.jpg").convert("RGB")
    icon_w = src.width // 4
    icon = src.crop((icon_w, 0, icon_w * 2, src.height))
    canvas = Image.new("RGB", (size, size), BLACK)
    return _paste_centered(canvas, icon)


def draw_glucose_trend(size: int = 128) -> Image.Image:
    """Horizontal trend arrow from default-trend.jpg (3rd icon), thickened."""
    src = Image.open(SPIFFS / "default-trend.jpg").convert("RGB")
    icon_w = src.width // 5
    icon = src.crop((icon_w * 2, 0, icon_w * 3, src.height))
    upscale = 12
    big = icon.resize((icon.width * upscale, icon.height * upscale), Image.Resampling.NEAREST)
    big = _thicken_white_on_black(big, passes=2)
    canvas = Image.new("RGB", (size, size), BLACK)
    return _paste_centered(canvas, big)


def _load_font(size: int) -> ImageFont.FreeTypeFont | ImageFont.ImageFont:
    for path in (
        "C:/Windows/Fonts/arialbd.ttf",
        "C:/Windows/Fonts/segoeuib.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
    ):
        p = Path(path)
        if p.exists():
            return ImageFont.truetype(str(p), size=size)
    return ImageFont.load_default()


def draw_message_palette(width: int = 160, height: int = 60, text: str = "Welcome to frixos") -> Image.Image:
    img = Image.new("RGB", (width, height), BLACK)
    draw = ImageDraw.Draw(img)
    font_size = 21
    font = _load_font(font_size)
    label = text
    while label and draw.textlength(label, font=font) > width - 4:
        label = label[:-1]
    if label != text:
        label = label.rstrip() + "\u2026"
    bbox = draw.textbbox((0, 0), label, font=font)
    text_h = bbox[3] - bbox[1]
    draw.text((2, max(0, (height - text_h) // 2 - bbox[1])), label, fill=WHITE, font=font)
    return img


def digit_bounds(img: Image.Image) -> tuple[int, int, int, int]:
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


def draw_bold_digit_label(scale: int = 2, line_thickness: int = 18) -> Image.Image:
    src = Image.open(SPIFFS / "bold.jpg").convert("RGB")
    w, h = src.size
    gap = scale
    extra_bottom = gap + line_thickness + scale
    out_w, out_h = w * scale, h * scale + extra_bottom
    img = Image.new("RGB", (out_w, out_h), BLACK)
    scaled = src.resize((w * scale, h * scale), Image.Resampling.NEAREST)
    img.paste(scaled, (0, 0))
    x0, y0, x1, y1 = digit_bounds(scaled)
    draw = ImageDraw.Draw(img)
    line_y0 = y1 + gap + 1
    draw.rectangle([x0, line_y0, x1, line_y0 + line_thickness - 1], fill=RED)
    return img


def main() -> None:
    save(draw_waxing_gibbous(), "palette-moon.jpg")
    save(draw_wifi_off(), "palette-wifi-off.png")
    save(draw_glucose_level(), "palette-glucose.png")
    save(draw_glucose_trend(), "palette-trend.png")
    save(draw_message_palette(), "palette-message.png")
    save(draw_bold_digit_label(), "bold-digit-label.jpg")


if __name__ == "__main__":
    main()
