#!/usr/bin/env python3
"""Publish a frixos release to the local frixos-web server tree.

Full release (new firmware + files):
    python push.py 68 [--pin <piv-pin>]

Quiet file refresh (web files only, same firmware, devices pick it up on
their ~10th update check without flashing or rebooting):
    python push.py --files-only 68 [--pin <piv-pin>]

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
    ap.add_argument("--pin", help="PIV PIN (omit to be prompted)")
    args = ap.parse_args()

    ver_dir = WWW / str(args.version)
    manifest_out = ver_dir / "manifest.txt"
    fw_bin = WWW / f"revE{args.version}.bin"
    print(f"pushing {'files-only refresh' if args.files_only else 'full release'} "
          f"v{args.version} at {time.strftime('%Y-%m-%d %H:%M:%S')}")

    if args.files_only:
        if not manifest_out.is_file():
            sys.exit(f"error: {manifest_out} does not exist - run a full push first")
        fw_line, _, old_generated = parse_manifest(manifest_out)
        if not fw_line:
            sys.exit(f"error: {manifest_out} has no fw entry - corrupt manifest?")
        ver_dir.mkdir(parents=True, exist_ok=True)
        copy_tree(ver_dir)
        write_files_txt(ver_dir / "files.txt")
        make_manifest(args.version, SPIFFS, None, manifest_out, args.pin,
                      fw_line=fw_line, min_generated=old_generated)
    else:
        if not FIRMWARE.is_file():
            sys.exit(f"error: {FIRMWARE} not found - run idf.py build first")
        age_min = (time.time() - FIRMWARE.stat().st_mtime) / 60
        print(f"firmware: {FIRMWARE} ({FIRMWARE.stat().st_size} B, built {age_min:.0f} min ago)")
        old_generated = parse_manifest(manifest_out)[2] if manifest_out.is_file() else 0
        ver_dir.mkdir(parents=True, exist_ok=True)
        shutil.copy2(FIRMWARE, fw_bin)
        copy_tree(ver_dir)
        write_files_txt(ver_dir / "files.txt")
        make_manifest(args.version, SPIFFS, FIRMWARE, manifest_out, args.pin,
                      min_generated=old_generated)

    audit_published(ver_dir, manifest_out, fw_bin)
    print(f"published to {ver_dir}")


if __name__ == "__main__":
    main()
