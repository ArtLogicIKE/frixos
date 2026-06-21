#!/usr/bin/env python3
"""Upload SPIFFS files to a Frixos device via /api/ota (non-firmware files)."""

from __future__ import annotations

import json
import sys
import time
import urllib.error
import urllib.request
from pathlib import Path

DEFAULT_HOST = "192.168.2.129"
SPIFFS_ROOT = Path(__file__).resolve().parent.parent / "spiffs"

# Upload only web UI assets (skip large images unless requested)
DEFAULT_PATTERNS = [
    "index.html",
    "css/*.css",
    "js/*.js",
    "i18n/*.json",
]


def upload_file(host: str, local_path: Path, spiffs_name: str, retries: int = 3) -> tuple[bool, str]:
    data = local_path.read_bytes()
    last_err = ""
    for attempt in range(retries):
        req = urllib.request.Request(
            f"http://{host}/api/ota",
            data=data,
            method="POST",
            headers={
                "X-Filename": spiffs_name,
                "Content-Type": "application/octet-stream",
            },
        )
        try:
            with urllib.request.urlopen(req, timeout=120) as resp:
                body = json.loads(resp.read().decode())
                ok = resp.status == 200 and body.get("status") == "ok"
                return ok, body.get("message", str(body))
        except Exception as exc:
            last_err = str(exc)
            if attempt + 1 < retries:
                time.sleep(5 * (attempt + 1))
    return False, last_err


def collect_files(patterns: list[str]) -> list[Path]:
    files: list[Path] = []
    for pat in patterns:
        files.extend(sorted(SPIFFS_ROOT.glob(pat)))
    return files


def main() -> int:
    host = sys.argv[1] if len(sys.argv) > 1 else DEFAULT_HOST
    patterns = sys.argv[2:] if len(sys.argv) > 2 else DEFAULT_PATTERNS
    files = collect_files(patterns)
    if not files:
        print("No files matched")
        return 1

    print(f"Uploading {len(files)} file(s) to http://{host}/")
    failed = 0
    for path in files:
        name = path.relative_to(SPIFFS_ROOT).as_posix()
        ok, msg = upload_file(host, path, name)
        status = "OK" if ok else "FAIL"
        print(f"  [{status}] {name}: {msg}")
        if not ok:
            failed += 1
        time.sleep(2)
    return 1 if failed else 0


if __name__ == "__main__":
    raise SystemExit(main())
