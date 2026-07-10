#!/usr/bin/env python3
"""Publish a frixos release to the local frixos-web server tree.

Full release (new firmware + files):
    python push.py 68 [--pin <piv-pin>]

Quiet file refresh (web files only, same firmware, devices pick it up on
their ~10th update check without flashing or rebooting):
    python push.py --files-only 68 [--pin <piv-pin>]

Recovery bootstrap (firmware + a one-anchor manifest, no web tree) for devices
whose SPIFFS is too full/fragmented to write the full manifest - they flash
the firmware (app partition, no SPIFFS) and self-heal on a later version:
    python push.py --firmware-only 68 [--pin <piv-pin>]

Releases >= 70 (LittleFS era) are DUAL-manifest, published automatically by
the full push: manifest.txt is the tiny anchored bootstrap (served to legacy
<70 requesters, who cannot write more), manifest_full.txt is the real file
set (served when the device sends ?fw=<70+>). Two YubiKey touches per push.

Every publish signs manifest.txt on the release YubiKey (touch required) and
then re-hashes the COPIED server tree against the manifest, so the published
signature can never disagree with the published content. Any failure aborts
before devices can see an inconsistent state.

Replaces push.bat (now a shim). Layout published, matching the device paths:
    www/revE<ver>.bin            firmware (fetched from server root)
    www/<ver>/...                spiffs tree
    www/<ver>/files.txt          legacy names-only manifest (old firmwares)
    www/<ver>/manifest.txt       signed manifest (fw >= 68)
"""
import argparse
import shutil
import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent / "tools"))
from make_manifest import EXCLUDE, make_manifest, sha256_file  # noqa: E402

REPO = Path(__file__).resolve().parent
SPIFFS = REPO / "spiffs"
FIRMWARE = REPO / "build" / "frixos.bin"
WWW = Path(r"C:\source\frixos-web\www")

# Anchor for --firmware-only: a small, stable, universally-present file. A
# self-healed device is guaranteed to have it byte-for-byte (it is a signed
# manifest entry that has not changed), so reconcile skips it without a write.
ANCHOR_REL = "favicon.ico"


def spiffs_files():
    return sorted(p for p in SPIFFS.rglob("*")
                  if p.is_file() and p.relative_to(SPIFFS).as_posix() not in EXCLUDE)


def write_files_txt(dest: Path):
    """Legacy manifest: bare relative paths, one per line, + manifest.txt so
    old firmwares download the signed manifest too (fw 68 self-heal bootstrap)."""
    names = [p.relative_to(SPIFFS).as_posix() for p in spiffs_files()]
    names.append("manifest.txt")
    dest.write_bytes(("\n".join(names) + "\n").encode())


def parse_manifest(path: Path):
    fw_line, entries, generated = None, {}, 0
    for line in path.read_text().splitlines():
        if line.startswith("fw "):
            fw_line = line
        elif line.startswith("f "):
            _, sha, size, rel = line.split(" ", 3)
            entries[rel] = (sha, int(size))
        elif line.startswith("generated "):
            generated = int(line.split(" ", 1)[1])
    return fw_line, entries, generated


def audit_published(ver_dir: Path, manifest: Path, fw_bin: Path):
    """Re-hash the copied tree against the manifest we just published."""
    fw_line, entries, _ = parse_manifest(manifest)
    _, fw_name, fw_size, fw_sha = fw_line.split(" ")
    bad = []
    if fw_bin.name != fw_name or fw_bin.stat().st_size != int(fw_size) \
            or sha256_file(fw_bin) != fw_sha:
        bad.append(fw_name)
    for rel, (sha, size) in entries.items():
        p = ver_dir / rel
        if not p.is_file() or p.stat().st_size != size or sha256_file(p) != sha:
            bad.append(rel)
    if bad:
        sys.exit(f"PUBLISH AUDIT FAILED - server copy differs from manifest: {bad}")
    print(f"publish audit OK: firmware + {len(entries)} files match the signed manifest")


def copy_tree(ver_dir: Path):
    for src in spiffs_files():
        dst = ver_dir / src.relative_to(SPIFFS)
        dst.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(src, dst)


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("version", type=int)
    ap.add_argument("--files-only", action="store_true",
                    help="refresh web files of an existing release; firmware entry reused")
    ap.add_argument("--firmware-only", action="store_true",
                    help="publish firmware + a 0-file bootstrap manifest (recovery for "
                         "devices whose SPIFFS is too full to write the full manifest)")
    ap.add_argument("--pin", help="PIV PIN (omit to be prompted)")
    args = ap.parse_args()
    if args.files_only and args.firmware_only:
        sys.exit("error: --files-only and --firmware-only are mutually exclusive")

    ver_dir = WWW / str(args.version)
    manifest_out = ver_dir / "manifest.txt"
    fw_bin = WWW / f"revE{args.version}.bin"
    print(f"pushing {'files-only refresh' if args.files_only else 'full release'} "
          f"v{args.version} at {time.strftime('%Y-%m-%d %H:%M:%S')}")

    manifest_full_out = ver_dir / "manifest_full.txt"
    # LittleFS-era releases (>= 70) are dual-manifest: manifest.txt is the tiny
    # anchored bootstrap served to legacy (<70) requesters - the only manifest
    # a device with a wedged SPIFFS can still write - and manifest_full.txt is
    # the real file set, served when the device sends ?fw=<70+>.
    dual = args.version >= 70
    audit_manifest = manifest_full_out if dual else manifest_out

    if args.files_only:
        src_manifest = manifest_full_out if dual else manifest_out
        if not src_manifest.is_file():
            sys.exit(f"error: {src_manifest} does not exist - run a full push first")
        fw_line, _, old_generated = parse_manifest(src_manifest)
        if not fw_line:
            sys.exit(f"error: {src_manifest} has no fw entry - corrupt manifest?")
        ver_dir.mkdir(parents=True, exist_ok=True)
        copy_tree(ver_dir)
        if dual:
            # Re-sign BOTH manifests: the tiny one embeds the anchor's hash,
            # which must match the refreshed tree. Tiny FIRST so the full one
            # carries the higher generation (devices apply tiny, then fetch
            # full; the anti-replay check requires generations to move forward).
            anchor = SPIFFS / ANCHOR_REL
            if not anchor.is_file():
                sys.exit(f"error: anchor {anchor} not found in spiffs tree")
            (ver_dir / "files.txt").write_bytes(f"{ANCHOR_REL}\nmanifest.txt\n".encode())
            print("signing bootstrap manifest (1/2) - YubiKey touch...")
            make_manifest(args.version, None, None, manifest_out, args.pin,
                          fw_line=fw_line, min_generated=old_generated,
                          firmware_only=True, anchor=anchor, anchor_rel=ANCHOR_REL)
            print("signing full manifest (2/2) - YubiKey touch...")
            make_manifest(args.version, SPIFFS, None, manifest_full_out, args.pin,
                          fw_line=fw_line,
                          min_generated=parse_manifest(manifest_out)[2])
        else:
            write_files_txt(ver_dir / "files.txt")
            make_manifest(args.version, SPIFFS, None, manifest_out, args.pin,
                          fw_line=fw_line, min_generated=old_generated)
    elif args.firmware_only:
        # Recovery bootstrap: publish the firmware + a manifest that lists only
        # a single stable ANCHOR file the device already has unchanged, so a
        # device with a wedged (too-full) SPIFFS can still apply it. Reconcile
        # matches the anchor on disk and skips it (no write), the only SPIFFS
        # write is the ~330-byte manifest, and the firmware flashes to the app
        # partition. The manifest is not empty because devices reject a 0-entry
        # manifest (f-manifest.c: entry_count == 0). The newer firmware then
        # heals its filesystem and picks up the full file set from a later
        # (higher) version. No web tree is copied under this version.
        if not FIRMWARE.is_file():
            sys.exit(f"error: {FIRMWARE} not found - run idf.py build first")
        anchor = SPIFFS / ANCHOR_REL
        if not anchor.is_file():
            sys.exit(f"error: anchor {anchor} not found in spiffs tree")
        audit_manifest = manifest_out  # only the tiny manifest exists in this mode
        age_min = (time.time() - FIRMWARE.stat().st_mtime) / 60
        print(f"firmware: {FIRMWARE} ({FIRMWARE.stat().st_size} B, built {age_min:.0f} min ago)")
        print(f"anchor:   {ANCHOR_REL} ({anchor.stat().st_size} B)")
        old_generated = parse_manifest(manifest_out)[2] if manifest_out.is_file() else 0
        ver_dir.mkdir(parents=True, exist_ok=True)
        shutil.copy2(FIRMWARE, fw_bin)
        # Serve the anchor too, so a device whose copy has drifted can still
        # fetch it (if its SPIFFS has room); a matching device never asks.
        shutil.copy2(anchor, ver_dir / ANCHOR_REL)
        # Legacy (fw<=66) devices bootstrap off files.txt; list the anchor +
        # manifest so they take the same firmware-only path.
        (ver_dir / "files.txt").write_bytes(f"{ANCHOR_REL}\nmanifest.txt\n".encode())
        make_manifest(args.version, None, FIRMWARE, manifest_out, args.pin,
                      min_generated=old_generated, firmware_only=True,
                      anchor=anchor, anchor_rel=ANCHOR_REL)
    else:
        if not FIRMWARE.is_file():
            sys.exit(f"error: {FIRMWARE} not found - run idf.py build first")
        age_min = (time.time() - FIRMWARE.stat().st_mtime) / 60
        print(f"firmware: {FIRMWARE} ({FIRMWARE.stat().st_size} B, built {age_min:.0f} min ago)")
        old_generated = max(
            parse_manifest(manifest_out)[2] if manifest_out.is_file() else 0,
            parse_manifest(manifest_full_out)[2] if manifest_full_out.is_file() else 0,
        )
        ver_dir.mkdir(parents=True, exist_ok=True)
        shutil.copy2(FIRMWARE, fw_bin)
        copy_tree(ver_dir)
        if dual:
            anchor = SPIFFS / ANCHOR_REL
            if not anchor.is_file():
                sys.exit(f"error: anchor {anchor} not found in spiffs tree")
            # Legacy (fw<=66) devices bootstrap off files.txt; the mini list
            # sends them down the same tiny-write path as everyone else.
            (ver_dir / "files.txt").write_bytes(f"{ANCHOR_REL}\nmanifest.txt\n".encode())
            # Tiny FIRST so the full manifest carries the higher generation
            # (devices apply tiny, then fetch full; the anti-replay check
            # requires generations to move forward).
            print("signing bootstrap manifest (1/2) - YubiKey touch...")
            make_manifest(args.version, None, FIRMWARE, manifest_out, args.pin,
                          min_generated=old_generated, firmware_only=True,
                          anchor=anchor, anchor_rel=ANCHOR_REL)
            print("signing full manifest (2/2) - YubiKey touch...")
            make_manifest(args.version, SPIFFS, FIRMWARE, manifest_full_out, args.pin,
                          min_generated=parse_manifest(manifest_out)[2])
        else:
            write_files_txt(ver_dir / "files.txt")
            make_manifest(args.version, SPIFFS, FIRMWARE, manifest_out, args.pin,
                          min_generated=old_generated)

    audit_published(ver_dir, audit_manifest, fw_bin)
    print(f"published to {ver_dir}")


if __name__ == "__main__":
    main()
