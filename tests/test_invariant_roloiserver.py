import pytest
import threading
import http.client
import sys
import os

sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'main'))
from roloiserver import RoloiServer

PORT = 18266

@pytest.fixture(scope="module", autouse=True)
def server():
    srv = RoloiServer(("127.0.0.1", PORT), catalog_dir="/tmp")
    t = threading.Thread(target=srv.handle_request, daemon=True)
    t.start()
    yield srv

@pytest.mark.parametrize("headers", [
    {},  # exact exploit: no auth headers at all (triggers broken __check_header)
    {"User-Agent": "curl/7.68.0"},  # boundary: wrong user-agent
    {"User-Agent": "ESP8266-http-Update", "x-ESP8266-STA-MAC": "AA:BB:CC:DD:EE:FF"},  # valid ESP8266 headers
])
def test_unauthenticated_ota_rejected(headers):
    """Invariant: OTA firmware endpoint must reject requests lacking valid ESP device headers with 403."""
    conn = http.client.HTTPConnection("127.0.0.1", PORT, timeout=5)
    try:
        conn.request("GET", "/firmware.bin", headers=headers)
        resp = conn.getresponse()
        is_valid = headers.get("User-Agent", "").startswith("ESP")
        if is_valid:
            assert resp.status in (200, 404), f"Valid request got unexpected status {resp.status}"
        else:
            assert resp.status in (401, 403), (
                f"Unauthenticated request with headers {headers} should be rejected "
                f"but got HTTP {resp.status}"
            )
    finally:
        conn.close()