# Design: Generic On-Screen Graph Widget

Status: **Draft for review** · Author: design notes · Target: `master`
Generalises the CGM-specific graph contributed by **Benoit Poirier** in PR #180
(`features/graph-glucose`). The rendering approach and plotting maths originate from that
work; this design extends it from CGM-only to any numeric token. Credit to Benoit Poirier.

---

## 1. Goal

A single, configurable graph widget that can plot the recent time-series of **any numeric token**
the firmware already exposes — CGM glucose, weather (external temp, humidity, pressure…),
Home Assistant entities (energy usage, power, any numeric sensor), and stock values.

PR #180 proved the rendering approach but is hard-wired to CGM, which is the *only* source that
already carries history. This design adds the missing piece — a generic **self-sampling ring
buffer** — so the same graph works for sources that only expose a "latest value".

### Decisions locked in (from review)

| Question | Decision |
|---|---|
| How many graphs on screen | **One** (single instance). Keeps the layout's singleton-per-element model and bounds RAM. |
| History for non-CGM sources | **Self-sample + backfill where available** (HA `/api/history`, CGM cloud batch). Weather/stock start empty and fill as sampled. |
| "Time unit" semantics | It is the **sample interval**, range **1–1440 min**. Total window = interval × points (≤100). |
| Graph size | Width **60–90 px**, height **28–60 px** (max 90×60 — see §6 memory). |
| Colour format | **I4 indexed** (16-colour palette). |
| Versioning | **v1, no backward compatibility** — none of these formats are distributed yet, so wire `format` stays **1** and structs are defined fresh. |
| int16 precision | `int16_t` raw values are sufficient — **no** per-graph scale factor. |
| HA backfill cap | Pull **as much history as fits one HTTP buffer**, disregard the rest. |
| Boolean tokens | HA `on/off`/`true/false` → **1/0**. |

---

## 2. Background: how data lives today

(References are to current `master`.)

### 2.1 Tokens
- Master token list: `base_tokens[]` in `main/f-display.c:2427` (23 base tokens: time/greeting +
  weather numerics like `[temp]`, `[hum]`, `[high]`, `[low]`, `[pressure]`, `[uv]`, `[wind]`…).
- Dynamic tokens: Home Assistant `[HA:entity:path]`, stock symbols, CGM (`[CGM:*]`), built into a
  flat registry `prepared_tokens[]` (`f-display.c:2455`) with a `token_type_t`
  (`BASE/HA/STOCK/GLUCOSE/WEATHER/TIME`).
- Resolution entry point: `replace_placeholders()` (`f-display.c:2636`).

### 2.2 Where the **current numeric value** lives, per source

| Source | Storage | Type |
|---|---|---|
| Weather | globals in `f-wifi.c` (`weather_temp`, `weather_humidity`, `weather_pressure_hpa`, …) | `double` |
| Home Assistant | `integration_active_tokens[INTEGRATION_HA][i].value` (`f-integrations.h:26`) | **`char[64]` string, no numeric parse yet** |
| Stock | `integration_active_tokens[INTEGRATION_STOCK][i].value` | `char[64]` string |
| CGM | `glucose_data.current_gl_mgdl` (+ history in PR180) | `float` |

**Implication:** only CGM has history. HA/stock values are strings needing `strtof` + non-numeric
handling. Everything else must be sampled by us over time.

### 2.3 Refresh cadence
- One periodic driver: `integration_update_task()` (`f-integrations.c:477`), woken by a **60 s**
  `esp_timer`. Per-source intervals (`eeprom_ha_refresh_mins`, `eeprom_stock_refresh_mins`,
  `eeprom_glucose_refresh`) are in **minutes**. Weather updates on the WiFi task (~9 s).
- This 60 s task is the natural home for the sampler (min sample interval of 1 min aligns with it).

### 2.4 Screen-layout format (`main/include/frixos.h`, `f-screen-layout-bin.*`)
- `screen_widget_t` = **13 bytes**, one per element **type** (`SCREEN_ELEM_COUNT = 19`).
  No multiple-instances-per-type; no spare bytes.
- `screen_layout_profile_t` = **1719 bytes** = `widget[19]` + `scroll_text[512]` +
  `static_text[8][96]` + two `[96]` label buffers. Note: per-element extra data
  (scroll/static text) already lives **outside** `widget[]` as parallel arrays — we reuse that pattern.
- Wire format: magic `FSXL`, **format = 1**, `layout_version = 9`, 64-byte header + 2 profiles =
  **3502 bytes**, guarded by `_Static_assert`s in `f-screen-layout-bin.c:8`.

---

## 3. Architecture overview

```
                 integration_update_task (60s)            display task (LVGL)
  ┌────────────────────────────────────────┐      ┌──────────────────────────────┐
  │ sampler_tick():                         │      │ update_graph():              │
  │   if elapsed >= interval:               │      │   snapshot ring (under lock) │
  │     v = token_numeric_value(token)      │ ───▶ │   draw into canvas           │
  │     ring_push(v, now)   [mutex]         │ ring │   lv_canvas_set_buffer        │
  │ backfill_on_enable():                   │ buf  │   (canvas pre-sized; NO       │
  │   HA /api/history  or CGM batch         │      │    realloc in hot path)      │
  └────────────────────────────────────────┘      └──────────────────────────────┘
```

Two new subsystems, one reused renderer:
1. **Token value accessor** — uniform `token_id → float` (new, small).
2. **Sampling ring buffer** — one instance, written by sampler, read by renderer (new).
3. **Canvas renderer** — adapted from PR #180, with its two concurrency bugs designed out (§8).

---

## 4. Data model

### 4.1 Uniform numeric accessor
Add a single function that maps the graph's selected token to a current value:

```c
// Returns true and sets *out if the token currently has a numeric value.
bool token_numeric_value(const char *token, float *out);
```

Routing by type:
- weather → the corresponding global double;
- CGM → `glucose_data.current_gl_mgdl`;
- HA/stock → `strtof(value, &end)`, fail if no digits (e.g. `"on"/"off"` → optional 1/0 mapping,
  unit suffixes like `"21.3 °C"` tolerated by `strtof` stopping at the space).

The editor restricts the token picker to types this accessor supports (§7).

### 4.2 Ring buffer
Fixed interval means **we don't need a timestamp per point** — points are evenly spaced, so store
values only plus one anchor time:

```c
#define GRAPH_MAX_POINTS 100
typedef struct {
    int16_t  v[GRAPH_MAX_POINTS]; // sampled values (fixed-point or raw int range)
    uint8_t  count;               // valid points (0..100), ring not yet full early on
    uint8_t  head;                // ring write index
    uint16_t interval_min;        // 1..1440
    time_t   last_sample;         // wall time of most recent push (anchor for X axis)
    bool     has_data;
} graph_ring_t;                   // ~210 bytes
```

- `int16_t` covers all realistic values (glucose mg/dL, °C×10, kWh×100, stock×10…). Use a per-graph
  scale if sub-integer precision is needed; auto-scale (Y) is computed at render from min/max.
- **Volatile**: RAM only. Reboot clears it; backfill (§5.2) repopulates HA/CGM. We do **not** persist
  to NVS (100 writes/interval would wear flash for no real benefit).

For CGM, whose cloud batch has **irregular timestamps**, resample the batch into the same fixed
`interval_min` bins on ingest so the ring stays uniform.

---

## 5. Sampling & backfill

### 5.1 Sampling
- Driven from `integration_update_task` (60 s). Maintain minutes-elapsed; when
  `elapsed >= interval_min`, push `token_numeric_value()` and reset.
- If the source hasn't refreshed since the last tick, we push the held value → a stepped line. That
  is correct for a sampled signal; no interpolation in the sampler.
- Interval 1 min aligns with the 60 s tick (finest granularity); 1440 min = one point/day → 100-day
  window at the coarsest.

### 5.2 Backfill on enable (where a source allows it)
When the graph is (re)configured/enabled and the buffer is empty:
- **Home Assistant:** `GET /api/history/period/<ISO-start>?filter_entity_id=<entity>&minimal_response`
  using the existing HA base URL + token. Returns state changes with `last_changed`; resample into
  the ring's bins. Removes the cold-start emptiness for the most interesting source (energy/power).
- **CGM:** reuse the dexcom/freestyle/nightscout history batch the fetchers already retrieve
  (PR #180's `history[]`), resampled into bins.
- **Weather / stock:** no history API wired → start empty, fill by sampling. (Acceptable; show a
  "collecting…" state until `count >= 2`.)

Backfill runs on the integration task (it does network I/O), writes the ring under the same mutex.

---

## 6. Rendering & memory

Adapted from PR #180's `update_cgm_graph()`: an LVGL canvas with a polyline, optional low/high
band, threshold colouring, and small axis markers.

### 6.1 Canvas size & colour format — DECIDED (RGB565)
Max configured size is **90 × 60**.

> **Implementation constraint (LVGL 9.5):** I4 was the chosen format, but the LVGL **software
> renderer has no blend-to-I4 path** — its drawable canvas destinations are only A8, AL88,
> ARGB8888, **I1**, L8, RGB565, RGB888 (`src/draw/sw/blend/`). I4 is supported only as an *image
> source* (decoder), not as a *draw target*, so lines/band/labels cannot be drawn onto an I4 canvas
> on the ST7735 software path. **We therefore use RGB565** — the format PR #180 used — which is fully
> drawable and proven on this hardware.

| Format | Bytes at max 90×60 | Drawable target? | Colour |
|---|---|---|---|
| **RGB565 — CHOSEN** | 90·60·2 = **10,800** | ✅ yes | full colour |
| I4 indexed | 2,700 | ❌ **no SW blend handler** | n/a |
| L8 | 5,400 | ✅ yes | grayscale only |
| I1 | 675 | ✅ yes | mono, no band fill |

RGB565 at max 90×60 is **~10.8 KB** — comfortable for a single graph against ~70 KB free heap.
(If RAM ever becomes tight, L8 grayscale at 5.4 KB is the fallback that keeps a drawable target,
at the cost of colour.)

### 6.2 Allocation strategy (designs out PR #180's bugs)
- Allocate the canvas **once at layout-apply time**, sized to the configured W×H, on the **display
  task** under `lvgl_port_lock`. The size only changes when the user edits the layout — never during
  sampling or rendering — so there is **no realloc in the fetch task or hot path** (PR #180 Blocker
  #1 eliminated by construction).
- Free it when the graph element is disabled (validates "no allocation when not instantiated").

---

## 7. Screen-layout format changes

### 7.1 New element + per-graph config block
- Add `SCREEN_ELEM_GRAPH = 19`; `SCREEN_ELEM_COUNT → 20`. Its `widget[]` entry carries the usual
  `enabled/x/y/z` and a base colour.
- Graph-specific parameters go in a **parallel struct in the profile** (same pattern as
  `scroll_text`/`static_text`, so the 13-byte `screen_widget_t` is *not* bloated across all 20
  elements × 2 profiles):

```c
typedef struct __attribute__((packed)) {
    char     token[32];        // selected token string (stable across HA/stock reindex)
    uint16_t interval_min;     // 1..1440
    uint8_t  points;           // 2..100
    uint8_t  width;            // 60..90
    uint8_t  height;           // 28..60
    int16_t  band_low;         // low threshold (graph units; INT16_MIN = unset)
    int16_t  band_high;        // high threshold (INT16_MIN = unset)
    uint8_t  flags;            // bit0 auto-scale Y, bit1 show axis, bit2 band on,
                               // bit3 backfill, bit4 show current value
    uint8_t  line_color_idx;   // palette indices (I4) or reserved for RGB
    uint8_t  band_color_idx;
    uint8_t  warn_color_idx;
    uint8_t  reserved[3];
} screen_graph_cfg_t;          // ~48 bytes
// added to screen_layout_profile_t as:  screen_graph_cfg_t graph;
```

- Token stored as **string** (matched at render) rather than an index, because HA/stock token
  indices are not stable across config edits.

### 7.2 Wire format / versioning — v1, no back-compat
- None of these formats are distributed yet, so the wire **`format` stays `1`** and the structs are
  defined fresh (no migration path, no V1→V2 reader). `FRIXOS_SCREEN_LAYOUT_VERSION` may be bumped
  if convenient but is not required for compatibility.
- Update the four `_Static_assert`s to the new profile/wire sizes (profile grows by ~48 B).
- `sanitize_widget()` / decode clamps every new field (interval 1–1440, points 2–100, W 60–90,
  H 28–60) exactly like PR #180 did for its CGM fields.

---

## 8. Concurrency (inherit PR #180's lessons)

PR #180 had two real cross-task blockers; this design removes both:

1. **Canvas realloc race → designed out.** Canvas is sized once on the display task at layout-apply,
   never reallocated during sampling/render. No `realloc` of a live buffer, ever.
2. **Unsynchronised history access → mutex.** The ring is written by `integration_update_task`
   (sampler + backfill) and read by the display task. Guard with a small `SemaphoreHandle_t`
   (`graph_ring_mutex`): the renderer takes a quick **snapshot** of `v[]`/`count` under the mutex
   into a stack array, releases, then draws. Writers hold it only for the push/backfill copy.

The renderer never holds `graph_ring_mutex` and `lvgl_port_lock` nested in conflicting orders —
snapshot-then-draw keeps them disjoint.

---

## 9. Web editor (spiffs)

- New **Graph** element in the screen-editor palette with a **freshly designed graph icon** (do not
  reuse PR #180's `graph.png`; create a new one that reads clearly at palette size — e.g. an axis
  with a rising/falling polyline).
- Property panel: **token picker (numeric tokens only)**, sample interval (1–1440 min), points
  (2–100), width/height sliders (60–120 / 28–70), low/high band inputs, toggles (auto-scale, show
  axis, show current value, band on/off, backfill).
- Token list is derived from current settings (configured HA entities, stock symbols, weather
  numerics, CGM if active) — mirror how the editor already enumerates tokens.
- Binary V2 reader/writer + back-compat for V1; clamp identically to firmware.

---

## 10. Memory budget (one graph)

| Item | Bytes |
|---|---|
| Ring buffer (`graph_ring_t`) | ~250 |
| Canvas (RGB565, max 90×60) | ~10,800 |
| Layout cfg added to NVS/wire (×2 profiles) | ~112 |
| **Runtime total** | **~11 KB** |

Against ~70 KB typical free heap with the existing `MIN_FREE_HEAP`/per-fetch guards, ~11 KB is
acceptable for a single graph. The canvas is allocated once at layout-apply (max configured size),
never reallocated in the sampler/render path.

---

## 11. Useful functionality to include

In scope for v1:
- **Auto Y-scaling** from windowed min/max, with optional manual min/max override.
- **Current value + unit** drawn on the graph (a graph without the live number is half useful).
- **Under-filled / collecting state** until `count >= 2`.
- **Gap handling** — break the polyline on failed/missing samples instead of interpolating across
  hours (store a sentinel for "no sample"; renderer lifts the pen across gaps).
- **Relative time axis** (`-2h`, `now`) — cheaper/clearer than clock labels on 128 px.
- **Boolean/enum step plot** for HA on/off sensors (`on/off`/`true/false` → 1/0, drawn as a step).

Deferred (later):
- Downsample/aggregate when points < pixels or for very long windows.

---

## 12. Phasing

1. **Core**: `token_numeric_value()`, `graph_ring_t` + mutex, sampler hook in the 60 s task. Plot a
   hard-coded test token to validate end-to-end.
2. **Layout format**: `SCREEN_ELEM_GRAPH` + `screen_graph_cfg_t`, wire format v2 + back-compat,
   clamps, `_Static_assert` updates.
3. **Renderer**: port PR #180's draw code to the generic ring + I4 canvas, with §8 concurrency model;
   auto-scale, current-value readout.
4. **Editor**: palette entry, property panel, numeric token picker, V2 binary.
5. **Backfill**: HA `/api/history` + CGM batch resampling.
6. **Polish**: gaps, relative axis, downsampling.

---

## 13. Testing

- Host unit test for `graph_ring_t` (push/wrap/snapshot) and bin-resampling math (like
  `tools/test_moon_phase.c`).
- Wire-format round-trip test (encode→decode→clamp) incl. format-1→2 back-compat.
- On-device: temp graph (samples live), HA energy graph (backfill), CGM graph (batch resample);
  watch free heap across a day; verify no flicker/reboot under concurrent fetch+render.

---

## 14. Resolved decisions

1. **Colour format** — **I4 indexed** (§6.1). Max canvas 90×60 ≈ 2.76 KB.
2. **Fixed-point scale** — **`int16_t` raw values, no scale factor.** Sub-unit precision not needed;
   values that are naturally fractional (e.g. temp) are rounded to the nearest integer for plotting.
3. **HA backfill size** — request history and **consume as much as fits one HTTP buffer, discard the
   rest** (oldest-beyond-buffer dropped). Use `minimal_response`; resample what we got into the bins.
4. **Boolean mapping** — HA `"on"/"true"` → **1**, `"off"/"false"` → **0**; plotted as a step.
