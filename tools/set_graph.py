#!/usr/bin/env python3
"""Enable a graph widget on the device via /api/screen (read-modify-write).

Used to test the generic graph + HA backfill end to end.
Usage: python tools/set_graph.py [host] [token]
"""
import struct
import sys
import urllib.request

HOST = sys.argv[1] if len(sys.argv) > 1 else "http://192.168.2.129"
TOKEN = sys.argv[2] if len(sys.argv) > 2 else "[HA:sensor.living_kitchen_power:state]"

WIRE = 3704
HEADER, PROFILE = 64, 1820
GW = 247    # graph widget offset within profile (index 19 * 13)
GC = 1732   # graph cfg offset within profile
MIME = "application/vnd.frixos.screen-layout+1"

F_AUTOSCALE, F_AXIS, F_BAND, F_BACKFILL, F_VALUE = 1, 2, 4, 8, 0x10
flags = F_AUTOSCALE | F_AXIS | F_VALUE | F_BACKFILL

data = bytearray(urllib.request.urlopen(HOST + "/api/screen", timeout=10).read())
assert len(data) == WIRE, f"unexpected screen size {len(data)} != {WIRE}"

for prof in range(2):
    base = HEADER + prof * PROFILE
    w = base + GW
    data[w + 0] = 1            # enabled
    data[w + 1] = 4            # x
    data[w + 2] = 4            # y
    data[w + 3] = 1            # z
    data[w + 4] = 0            # font (unused)
    data[w + 5] = 0            # width (unused for graph)
    data[w + 6] = 0            # align
    data[w + 7], data[w + 8], data[w + 9] = 0, 220, 255   # line colour (cyan)
    data[w + 10], data[w + 11], data[w + 12] = 0, 0, 0    # background (black)

    g = base + GC
    tok = TOKEN.encode()[:63]
    data[g:g + 64] = tok + b"\x00" * (64 - len(tok))
    struct.pack_into("<H", data, g + 64, 5)    # interval_min = 5
    data[g + 66] = 60                          # points
    data[g + 67] = 90                          # width
    data[g + 68] = 40                          # height
    data[g + 69] = flags
    for o in (70, 72, 74, 76):                 # band_low/high, y_min/max = unset
        struct.pack_into("<h", data, g + o, -32768)
    data[g + 78], data[g + 79], data[g + 80] = 40, 60, 40
    data[g + 81], data[g + 82], data[g + 83] = 255, 80, 80
    data[g + 84], data[g + 85], data[g + 86] = 120, 120, 120
    data[g + 87] = 0

req = urllib.request.Request(HOST + "/api/screen", data=bytes(data), method="POST",
                            headers={"Content-Type": MIME})
print(urllib.request.urlopen(req, timeout=15).read().decode())
print(f"configured graph token={TOKEN!r}")
