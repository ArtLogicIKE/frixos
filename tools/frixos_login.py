#!/usr/bin/env python3
"""Log in to a Frixos that has an access password and print the session cookie.

    FRIXOS_PASSWORD=... python tools/frixos_login.py 192.168.2.175
    curl -b "$(FRIXOS_PASSWORD=... python tools/frixos_login.py 192.168.2.175)" http://192.168.2.175/api/status

The password stays on this machine: the device sends a salt and a one-time nonce,
and gets back HMAC-SHA256(PBKDF2(password, salt), nonce). See main/f-auth.c.
"""
import hashlib
import hmac
import json
import os
import sys
import urllib.error
import urllib.request


def login(host: str, password: str) -> str:
    base = "http://" + host
    with urllib.request.urlopen(base + "/api/auth/challenge", timeout=10) as r:
        c = json.load(r)
    if not c.get("auth"):
        return ""  # no password set: no cookie needed
    if c.get("locked"):
        raise SystemExit("locked for %d s" % c["locked"])
    key = hashlib.pbkdf2_hmac("sha256", password.encode(), c["salt"].encode(), c["iter"])
    proof = hmac.new(key, bytes.fromhex(c["nonce"]), "sha256").hexdigest()
    body = json.dumps({"nonce": c["nonce"], "proof": proof}).encode()
    req = urllib.request.Request(base + "/api/auth/login", body, {"Content-Type": "application/json"})
    try:
        with urllib.request.urlopen(req, timeout=10) as r:
            return r.headers["Set-Cookie"].split(";", 1)[0]
    except urllib.error.HTTPError as e:
        raise SystemExit("login failed: %d %s" % (e.code, e.read().decode(errors="replace")))


if __name__ == "__main__":
    if len(sys.argv) != 2 or "FRIXOS_PASSWORD" not in os.environ:
        raise SystemExit(__doc__)
    print(login(sys.argv[1], os.environ["FRIXOS_PASSWORD"]))
