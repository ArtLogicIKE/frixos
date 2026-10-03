#!/usr/bin/env python3
"""Host-side tests for every setting the web UI posts.

Runs the real SPIFFS scripts (no browser, no device) and checks the JSON they
POST. Then checks main/f-settings.c: a value the UI sends must survive both
validation and the apply path. Brightness time (p55/p56) and Wi-Fi hours
(p46/p47) are minutes from midnight, 0-1439. An apply gate of 0-23 accepts
the POST and then drops 07:30.

CI runs this file. It does not talk to hardware.
"""

from __future__ import annotations

import json
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SETTINGS_C = ROOT / "main" / "f-settings.c"
PWM_H = ROOT / "main" / "include" / "f-pwm.h"

# What the harness types into the form, and what /api/settings must receive.
EXPECTED_SAVE = {
    "p00": "frixos-ui",
    "p34": "NewNet",
    "p35": "new-secret",
    "p03": 2,
    "p09": 1,
    "p36": 1,
    "p37": 1,
    "p60": "192.168.2.50",
    "p61": "192.168.2.1",
    "p62": "255.255.255.0",
    "p63": "8.8.8.8,8.8.4.4",
    "p17": "40.7128000",
    "p18": "-74.006000",
    "p19": "EST5EDT,M3.2.0,M11.1.0",
    "tz_iana": "America/New_York",
    "p46": 6 * 60 + 30,  # 06:30 Wi-Fi start
    "p47": 22 * 60,  # 22:00 Wi-Fi end
    "p22": 2,
    "p21": 20.5,
    "p20": 8.5,
    "p23": [60, 15],
    "p42": 250,
    "p43": 900,
    "p55": 7 * 60 + 30,  # 07:30 brightness-time start, not hour 7
    "p56": 22 * 60 + 45,  # 22:45 brightness-time end
    "p25": "http://ha.local:8123/",
    "p26": "new-token",
    "p27": 2,
    "p28": "new-key",
    "p29": 10,
    "p64": "KCASANFR123",
    "p65": "new-wu-key",
    "p66": 20,
    "p30": 1,
    "p31": "cgm-user",
    "p32": "glu-pass",
    "p33": 3,
    "p44": 2,
    "p45": 45,
    "p51": 180,
    "p52": 65,
    "p53": 1,
    "p54": "https://ns.example.com",
}

EXPECTED_RELOAD = {
    "dim_start": "07:00",  # stored p55 = 420
    "dim_end": "22:15",  # stored p56 = 1335
    "wifi_start": "08:00",  # stored p46 = 480
    "wifi_end": "00:00",
}

EXPECTED_SCREEN = {
    "p08": 1,
    "p24": 1,
    "p50": 1,
    "p58": '[{"t":2,"d":15}]',
    "p59": '[{"t":0,"d":20}]',
}


def fail(msg: str, failures: list[str]) -> None:
    failures.append(msg)


def same(a, b) -> bool:
    if isinstance(a, bool) or isinstance(b, bool):
        return a is b or a == b
    if isinstance(a, (int, float)) and isinstance(b, (int, float)):
        return float(a) == float(b)
    if isinstance(a, list) and isinstance(b, list):
        return len(a) == len(b) and all(same(x, y) for x, y in zip(a, b))
    return a == b


def run_harness(failures: list[str]) -> dict | None:
    try:
        proc = subprocess.run(
            ["node", str(ROOT / "tools" / "ui_settings_harness.mjs")],
            cwd=ROOT,
            capture_output=True,
            text=True,
            timeout=60,
            check=False,
        )
    except subprocess.TimeoutExpired:
        fail("UI harness timed out", failures)
        return None
    if proc.returncode != 0:
        detail = (proc.stderr or proc.stdout or "harness failed").strip()
        fail("UI harness exited %s\n%s" % (proc.returncode, detail), failures)
        return None
    line = next((ln for ln in proc.stdout.splitlines() if ln.startswith("RESULT ")), None)
    if not line:
        fail("UI harness produced no RESULT line\n" + proc.stdout + proc.stderr, failures)
        return None
    return json.loads(line[len("RESULT ") :])


def check_ui(observed: dict, failures: list[str]) -> None:
    reload = observed.get("reload") or {}
    for key, expected in EXPECTED_RELOAD.items():
        got = reload.get(key)
        if got != expected:
            fail("reload %s: expected %r, got %r" % (key, expected, got), failures)

    if observed.get("p35BeforeDirty"):
        fail("wifi password was posted before the user edited it", failures)

    save = observed.get("save")
    if not isinstance(save, dict):
        fail("Save did not POST /api/settings", failures)
        return
    for key, expected in EXPECTED_SAVE.items():
        if key not in save:
            fail("save omitted %s (expected %r)" % (key, expected), failures)
        elif not same(save[key], expected):
            fail("save %s: expected %r, got %r" % (key, expected, save[key]), failures)
    extra = sorted(set(save) - set(EXPECTED_SAVE))
    if extra:
        fail("save posted unexpected keys %s" % extra, failures)

    immediate = observed.get("immediate") or []
    flat: dict = {}
    for body in immediate:
        if isinstance(body, dict):
            flat.update(body)
    for key, expected in (("p40", 0), ("p41", 1), ("p39", 1)):
        if key not in flat:
            fail("immediate save omitted %s" % key, failures)
        elif not same(flat[key], expected):
            fail("immediate %s: expected %r, got %r" % (key, expected, flat[key]), failures)

    screen = observed.get("screen")
    if not isinstance(screen, dict):
        fail("layout time settings did not POST /api/settings", failures)
        return
    for key, expected in EXPECTED_SCREEN.items():
        if key not in screen:
            fail("screen save omitted %s (expected %r)" % (key, expected), failures)
        elif not same(screen[key], expected):
            fail("screen %s: expected %r, got %r" % (key, expected, screen[key]), failures)


def macros() -> dict[str, int]:
    text = PWM_H.read_text(encoding="utf-8")
    found = {}
    for name, value in re.findall(r"#define\s+(\w+)\s+(\d+)", text):
        found[name] = int(value)
    return found


def resolve(token: str, defined: dict[str, int]) -> int | None:
    if re.fullmatch(r"-?\d+", token):
        return int(token)
    if token in defined:
        return defined[token]
    return None


def slice_between(text: str, start: str, end: str) -> str:
    i = text.find(start)
    if i < 0:
        raise RuntimeError("missing %s" % start)
    j = text.find(end, i + len(start))
    if j < 0:
        raise RuntimeError("missing %s after %s" % (end, start))
    return text[i:j]


def validation_ranges(text: str, defined: dict[str, int]) -> dict[str, tuple[int, int]]:
    chunk = slice_between(text, "static bool validate_json_params", "esp_err_t screen_get_handler")
    parts = re.split(r'cJSON_GetObjectItem\(root, "(p\d+)"\)', chunk)
    ranges: dict[str, tuple[int, int]] = {}
    for i in range(1, len(parts), 2):
        key, body = parts[i], parts[i + 1]
        m = re.search(
            r"CHECK_(?:DOUBLE_)?RANGE\([^,]+,\s*[^,]+,\s*(-?\d+|[A-Z0-9_]+)\s*,\s*(-?\d+|[A-Z0-9_]+)\)",
            body,
        )
        if not m:
            continue
        lo, hi = resolve(m.group(1), defined), resolve(m.group(2), defined)
        if lo is None or hi is None:
            continue
        ranges[key] = (lo, hi)
    return ranges


def apply_upper_bounds(text: str, defined: dict[str, int]) -> dict[str, int]:
    chunk = slice_between(text, "esp_err_t settings_post_handler", "esp_err_t status_api_handler")
    marks = list(re.finditer(r'cJSON_(?:GetObjectItem|HasObjectItem)\(root, "(p\d+)"\)', chunk))
    bounds: dict[str, int] = {}
    for i, mark in enumerate(marks):
        end = marks[i + 1].start() if i + 1 < len(marks) else len(chunk)
        body = chunk[mark.end() : end]
        caps = []
        for token in re.findall(r"if\s*\([^;]*?<=\s*([A-Za-z0-9_]+)", body):
            value = resolve(token, defined)
            if value is not None:
                caps.append(value)
        if caps:
            # Tightest gate in the apply block is what actually gets stored.
            prev = bounds.get(mark.group(1))
            cap = min(caps)
            bounds[mark.group(1)] = cap if prev is None else min(prev, cap)
    return bounds


def check_firmware(failures: list[str]) -> None:
    text = SETTINGS_C.read_text(encoding="utf-8")
    defined = macros()
    ranges = validation_ranges(text, defined)
    bounds = apply_upper_bounds(text, defined)
    if not ranges:
        fail("parsed no validation ranges from f-settings.c", failures)
        return

    for key in ("p46", "p47", "p55", "p56"):
        if key not in ranges or key not in bounds:
            fail("%s: clock field is missing a validation range or apply gate" % key, failures)

    for key, (lo, hi) in sorted(ranges.items()):
        if key in bounds and bounds[key] < hi:
            fail(
                "%s: validation allows %s-%s but apply only stores <= %s"
                % (key, lo, hi, bounds[key]),
                failures,
            )

    for key, expected in EXPECTED_SAVE.items():
        if key not in ranges or isinstance(expected, str):
            continue
        lo, hi = ranges[key]
        values = expected if isinstance(expected, list) else [expected]
        for value in values:
            if not lo <= float(value) <= hi:
                fail("%s UI value %s outside validation %s-%s" % (key, value, lo, hi), failures)


def main() -> int:
    failures: list[str] = []
    observed = run_harness(failures)
    if observed is not None:
        check_ui(observed, failures)
    try:
        check_firmware(failures)
    except Exception as exc:
        fail("firmware contract: %s" % exc, failures)

    if failures:
        print("FAIL %d" % len(failures))
        for item in failures:
            print("- " + item)
        return 1
    print("PASS ui settings (%d save keys, firmware ranges)" % len(EXPECTED_SAVE))
    return 0


if __name__ == "__main__":
    sys.exit(main())
