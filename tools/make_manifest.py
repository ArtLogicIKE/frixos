#!/usr/bin/env python3
"""Build and sign a frixos OTA manifest (frixos-manifest-v1).

Used by push.py; can also be run standalone:

    python tools/make_manifest.py --version 68 --spiffs spiffs \
        --firmware build/frixos.bin --out manifest.txt [--pin 123456]

Format (LF line endings; the signature covers every byte before the "sig" line):

    frixos-manifest-v1
    version <N>
    generated <unix-ts>          <- monotonic per publish; devices reject older
    fw revE<N>.bin <size> <sha256hex>
    f <sha256hex> <size> <relpath>
    ...
    sig <hex raw 64-byte ECDSA-P256 r||s signature>

Signing happens on the release YubiKey (PIV slot 9c) via yubikit (installed
with `pip install yubikey-manager`); you are prompted for the PIV PIN unless
--pin is given, and the key's touch policy requires a tap on the YubiKey.
The fresh signature is then verified locally against
main/include/f-ota-pubkey.h (the exact bytes devices trust), so a wrong
key/slot can never produce a publishable manifest.
"""
import argparse
import getpass
import hashlib
import re
import sys
import time
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
PUBKEY_HEADER = REPO / "main" / "include" / "f-ota-pubkey.h"

# Device-generated / local state files that must never be listed in a manifest
EXCLUDE = {"files.txt", "manifest.txt"}


def sha256_file(path: Path) -> str:
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(65536), b""):
            h.update(chunk)
    return h.hexdigest()


def build_signed_region(version: int, spiffs_dir: Path, generated: int,
                        firmware: Path = None, fw_line: str = None) -> bytes:
    if fw_line is None:
        fw_line = f"fw revE{version}.bin {firmware.stat().st_size} {sha256_file(firmware)}"
    lines = [
        "frixos-manifest-v1",
        f"version {version}",
        f"generated {generated}",
        fw_line,
    ]
    entries = []
    for p in sorted(spiffs_dir.rglob("*")):
        if not p.is_file():
            continue
        rel = p.relative_to(spiffs_dir).as_posix()
        if rel in EXCLUDE:
            continue
        entries.append(f"f {sha256_file(p)} {p.stat().st_size} {rel}")
    lines.extend(entries)
    return ("\n".join(lines) + "\n").encode()


def pubkey_point_from_header() -> bytes:
    text = PUBKEY_HEADER.read_text()
    hexbytes = re.findall(r"0x([0-9a-fA-F]{2})", text.split("{", 1)[1].split("}", 1)[0])
    return bytes(int(b, 16) for b in hexbytes)


def der_sig_to_raw(sig_der: bytes) -> bytes:
    """DER ECDSA signature -> raw 64-byte r||s (what PSA on the device wants)."""
    from cryptography.hazmat.primitives.asymmetric.utils import decode_dss_signature
    r, s = decode_dss_signature(sig_der)
    return r.to_bytes(32, "big") + s.to_bytes(32, "big")


def sign(signed_region: bytes, pin: str | None) -> bytes:
    """ECDSA-P256/SHA-256 sign on the release YubiKey (PIV slot 9c).
    Returns the raw 64-byte r||s signature."""
    from cryptography.hazmat.primitives import hashes
    from ykman.device import list_all_devices
    from yubikit.core.smartcard import SmartCardConnection
    from yubikit.piv import KEY_TYPE, SLOT, PivSession

    devices = list_all_devices()
    if not devices:
        sys.exit("error: no YubiKey connected - the release key is required to publish")
    device, info = devices[0]
    with device.open_connection(SmartCardConnection) as conn:
        session = PivSession(conn)
        session.verify_pin(pin or getpass.getpass(f"PIV PIN (YubiKey {info.serial}): "))
        print("Signing on YubiKey - touch it when it blinks...", flush=True)
        try:
            sig_der = session.sign(SLOT.SIGNATURE, KEY_TYPE.ECCP256,
                                   signed_region, hashes.SHA256())
        except Exception as e:
            if "6982" in str(e):
                sys.exit("error: signing rejected (SW=6982). Either the touch "
                         "timed out (15 s), or gpg's scdaemon is holding the "
                         "card - run: gpg-connect-agent \"scd killscd\" /bye "
                         "and retry.")
            raise
    return der_sig_to_raw(sig_der)


def verify(signed_region: bytes, sig_raw: bytes) -> None:
    """Verify against the committed pubkey header - the bytes devices trust."""
    from cryptography.hazmat.primitives import hashes
    from cryptography.hazmat.primitives.asymmetric import ec
    from cryptography.hazmat.primitives.asymmetric.utils import encode_dss_signature

    point = pubkey_point_from_header()
    pub = ec.EllipticCurvePublicKey.from_encoded_point(ec.SECP256R1(), point)
    sig_der = encode_dss_signature(int.from_bytes(sig_raw[:32], "big"),
                                   int.from_bytes(sig_raw[32:], "big"))
    pub.verify(sig_der, signed_region, ec.ECDSA(hashes.SHA256()))  # raises on mismatch


def make_manifest(version: int, spiffs_dir: Path, firmware: Path,
                  out: Path, pin: str | None = None,
                  fw_line: str = None, min_generated: int = 0) -> None:
    generated = max(int(time.time()), min_generated + 1)
    region = build_signed_region(version, spiffs_dir, generated,
                                 firmware=firmware, fw_line=fw_line)
    sig_raw = sign(region, pin)
    verify(region, sig_raw)
    out.write_bytes(region + b"sig " + sig_raw.hex().encode() + b"\n")
    n_files = region.count(b"\nf ")
    print(f"wrote {out}: version {version}, {n_files} files, signature verified "
          f"against {PUBKEY_HEADER.name}")


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--version", type=int, required=True)
    ap.add_argument("--spiffs", type=Path, required=True)
    ap.add_argument("--firmware", type=Path, required=True,
                    help="firmware .bin whose hash goes into the fw entry")
    ap.add_argument("--out", type=Path, required=True)
    ap.add_argument("--pin", help="PIV PIN (omit to be prompted)")
    a = ap.parse_args()
    make_manifest(a.version, a.spiffs, a.firmware, a.out, a.pin)


if __name__ == "__main__":
    main()
