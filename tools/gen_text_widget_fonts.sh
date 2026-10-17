#!/usr/bin/env bash
# Regenerate the text-widget bitmap fonts (main/fonts/tw_<family>_<size>.c).
# Needs node/npx; lv_font_conv is fetched on demand. Run from the repo root.
set -euo pipefail

cd "$(dirname "$0")/.."
OUT="main/fonts"
CONV="npx --yes lv_font_conv@1.5.2"

declare -A FAMILY_TTF=(
  [sans]="managed_components/lvgl__lvgl/scripts/built_in_font/Montserrat-Medium.ttf"
  [bold]="tools/fonts/DejaVuSans-Bold.ttf"
)
# Must match the size tables in main/f-fonts.c. Sans also carries larger sizes (cap height up
# to ~36 px): digit themes use them for units and letters so text matches the sprite digits.
declare -A FAMILY_SIZES=(
  [sans]="8 10 12 14 16 20 24 28 32 36 40 44 48 52"
  [bold]="8 10 12 14 16 20 24 28 32 36"
)
# Up to 16 px: ASCII, Latin-1 (accents, degree, micro) and the trend arrows.
RANGES_TEXT=(--range 0x20-0x7E --range 0xA0-0xFF --range 0x2190-0x2193)
# Larger sizes show values: ASCII, degree/superscripts/micro and arrows only (half the flash).
RANGES_VALUE=(--range 0x20-0x7E --range 0xB0 --range 0xB2-0xB3 --range 0xB5 --range 0x2190-0x2193)

RANGES_FALLBACK=(--range 0x20-0x2F --range 0x3A-0x7E --range 0xB0 --range 0x2190-0x2193)

mkdir -p "$OUT"
for fam in "${!FAMILY_TTF[@]}"; do
  ttf="${FAMILY_TTF[$fam]}"
  [[ -f "$ttf" ]] || { echo "missing $ttf" >&2; exit 1; }
  for size in ${FAMILY_SIZES[$fam]}; do
    # Small sizes stay crisp and compact at 2 bpp; larger ones get 4 bpp anti-aliasing.
    bpp=4
    (( size <= 12 )) && bpp=2
    ranges=("${RANGES_TEXT[@]}")
    (( size > 16 )) && ranges=("${RANGES_VALUE[@]}")
    extra=()
    # Sizes above 36 only serve as unit/letter fallback next to digit sprites: no digits, no kerning.
    if (( size > 36 )); then
      ranges=("${RANGES_FALLBACK[@]}")
      extra=(--no-kerning)
      bpp=2
    fi
    name="tw_${fam}_${size}"
    $CONV --font "$ttf" "${ranges[@]}" --size "$size" --bpp "$bpp" --no-compress "${extra[@]}" \
      --format lvgl --lv-include lvgl.h -o "$OUT/$name.c"
    # lv_font_conv only emits const descriptors for LVGL 8; LVGL 9 never writes them,
    # so make them const to keep them in flash instead of RAM.
    sed -i -e 's/^static lv_font_fmt_txt_dsc_t font_dsc = {/static const lv_font_fmt_txt_dsc_t font_dsc = {/' \
      -e "s/^lv_font_t ${name} = {/const lv_font_t ${name} = {/" "$OUT/$name.c"
    echo "$name ($bpp bpp)"
  done
done
