#!/usr/bin/env python3
"""Round-trip API tests for Frixos device settings (excludes WiFi p34/p35)."""

from __future__ import annotations

import json
import sys
import time
import urllib.error
import urllib.request
from typing import Any

DEFAULT_HOST = "192.168.2.129"
SKIP_KEYS = {"p34", "p35", "status", "message"}
REBOOT_KEYS = {"p00", "p17", "p18", "p19", "tz_iana", "p60", "p61", "p62", "p63"}

# Test values per parameter key. Omitted keys are not mutated.
TEST_VALUES: dict[str, Any] = {
    "p00": "frixos-test-host",
    "p03": 2,
    "p09": 1,
    "p16": "UI test message [temp]",
    "p36": 1,
    "p37": 1,
    "p39": 0,
    "p40": 0,
    "p41": 1,
    "p17": "40.7128000",
    "p18": "-74.0060000",
    "p19": "EST5EDT,M3.2.0,M11.1.0",
    "tz_iana": "America/New_York",
    "p46": 480,
    "p47": 1320,
    "p20": 8.0,
    "p21": 20.0,
    "p22": 1,
    "p23": [80, 40],
    "p42": 250,
    "p43": 900,
    "p55": 420,
    "p56": 1320,
    "p24": 1,
    "p50": 1,
    "p08": 1,
    "p27": 2,
    "p29": 10,
    "p30": 0,
    "p33": 3,
    "p44": 0,
    "p45": 45,
    "p51": 180,
    "p52": 75,
    "p53": 1,
    "p58": '[{"t":0,"d":30},{"t":2,"d":10}]',
    "p59": '[{"t":0,"d":20}]',
}


class DeviceClient:
    def __init__(self, host: str) -> None:
        self.base = f"http://{host}"

    def get_settings(self) -> dict[str, Any]:
        return self._request("GET", "/api/settings")

    def post_settings(self, payload: dict[str, Any]) -> dict[str, Any]:
        return self._request("POST", "/api/settings", payload)

    def get_status(self) -> dict[str, Any]:
        return self._request("GET", "/api/status")

    def get_settings_group(self, group: str, params: str = "") -> dict[str, Any]:
        q = f"group={group}"
        if params:
            q += f"&params={params}"
        return self._request("GET", f"/api/settings?{q}")

    def _request(self, method: str, path: str, body: dict | None = None) -> dict[str, Any]:
        data = None
        headers = {"Accept": "application/json"}
        if body is not None:
            data = json.dumps(body).encode()
            headers["Content-Type"] = "application/json"
        req = urllib.request.Request(self.base + path, data=data, headers=headers, method=method)
        with urllib.request.urlopen(req, timeout=15) as resp:
            return json.loads(resp.read().decode())


def values_equal(a: Any, b: Any) -> bool:
    if isinstance(a, list) and isinstance(b, list):
        return a == b
    if isinstance(a, (int, float)) and isinstance(b, (int, float)):
        return float(a) == float(b)
    if isinstance(a, str) and isinstance(b, str):
        if a.strip().startswith("[") and b.strip().startswith("["):
            try:
                return json.loads(a) == json.loads(b)
            except json.JSONDecodeError:
                pass
    return str(a) == str(b)


def wait_online(client: DeviceClient, timeout: int = 90) -> bool:
    deadline = time.time() + timeout
    while time.time() < deadline:
        try:
            st = client.get_status()
            if st.get("wifi_connected"):
                return True
        except (urllib.error.URLError, TimeoutError, ConnectionError):
            pass
        time.sleep(2)
    return False


def test_group_masks(client: DeviceClient, results: list[str]) -> None:
    cases = [
        ("settings", "p03,p09,p16", {"p03", "p09", "p16", "p00"}),
        ("advanced", "", {"p17", "p22", "p23", "p42"}),
        ("integrations", "", {"p27", "p51", "p58"}),
        ("theme", "", {"p40", "p41"}),
    ]
    for group, params, expect_any in cases:
        q = f"group={group}"
        if params:
            q += f"&params={params}"
        data = client._request("GET", f"/api/settings?{q}")
        keys = {k for k in data if k.startswith("p") or k == "tz_iana"}
        missing = expect_any - keys
        if missing:
            results.append(f"FAIL group {group}: missing keys {sorted(missing)}")
        else:
            results.append(f"PASS group {group}: keys present")


def test_roundtrip(client: DeviceClient, backup: dict[str, Any], results: list[str]) -> None:
    for key, test_val in TEST_VALUES.items():
        if key in SKIP_KEYS:
            continue
        payload = {key: test_val}
        try:
            resp = client.post_settings(payload)
            if resp.get("status") != "ok":
                results.append(f"FAIL {key} save: {resp}")
                continue
            if key in REBOOT_KEYS:
                if not wait_online(client):
                    results.append(f"FAIL {key}: device did not come back online")
                    continue
                time.sleep(2)
            got = client.get_settings().get(key)
            if values_equal(got, test_val):
                results.append(f"PASS {key}")
            else:
                results.append(f"FAIL {key}: expected {test_val!r}, got {got!r}")
        except Exception as exc:
            results.append(f"FAIL {key}: {exc}")


def restore_backup(client: DeviceClient, backup: dict[str, Any]) -> None:
    payload = {k: v for k, v in backup.items() if k.startswith("p") or k == "tz_iana"}
    payload = {k: v for k, v in payload.items() if k not in SKIP_KEYS}
    if payload:
        client.post_settings(payload)
        if any(k in REBOOT_KEYS for k in payload):
            wait_online(client)


def main() -> int:
    host = sys.argv[1] if len(sys.argv) > 1 else DEFAULT_HOST
    client = DeviceClient(host)
    results: list[str] = []

    print(f"Testing device at {host}")
    try:
        status = client.get_status()
        print(f"Device: {status.get('app')} v{status.get('version')} @ {status.get('ip_address')}")
    except Exception as exc:
        print(f"Cannot reach device: {exc}")
        return 1

    backup = client.get_settings()
    print(f"Backed up {len(backup)} settings keys")

    test_group_masks(client, results)
    test_roundtrip(client, backup, results)

    print("\n--- Results ---")
    passed = failed = 0
    for line in results:
        print(line)
        if line.startswith("PASS"):
            passed += 1
        elif line.startswith("FAIL"):
            failed += 1

    print(f"\n{passed} passed, {failed} failed")
    print("Restoring original settings...")
    try:
        restore_backup(client, backup)
        print("Restore complete")
    except Exception as exc:
        print(f"Restore failed: {exc}")
        return 2

    return 1 if failed else 0


if __name__ == "__main__":
    raise SystemExit(main())
