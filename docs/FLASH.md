# Flashing Frixos from a Release

This document explains how to flash Frixos firmware onto an ESP32 using the binary files from a GitHub release.

For hardware wiring and boot mode instructions, see [PROGRAMMING.md](PROGRAMMING.md).

## Prerequisites

- Python and `esptool.py` installed:
  ```bash
  pip install esptool
  ```
- USB-to-serial adapter connected to the P1 header (see [PROGRAMMING.md](PROGRAMMING.md))

## Release Contents

| File | Flash Offset | Description |
|------|-------------|-------------|
| `bootloader.bin` | `0x1000` | Second-stage bootloader |
| `partition-table.bin` | `0x8000` | Partition layout |
| `frixos.bin` | `0x10000` | Main application firmware |
| `spiffs.bin` | `0x670000` | Web UI and assets |
| `flasher_args.json` | — | Flash parameters manifest |

## Full Flash (first-time or recovery)

Flashes all partitions. Replace `/dev/tty.usbserial-*` with your adapter's port.

```bash
esptool.py --chip esp32 --port /dev/tty.usbserial-* --baud 460800 write_flash \
  0x1000  bootloader.bin \
  0x8000  partition-table.bin \
  0x10000 frixos.bin \
  0x670000 spiffs.bin
```

## App Only (firmware update via USB)

If the bootloader and partition table are already correct:

```bash
esptool.py --chip esp32 --port /dev/tty.usbserial-* --baud 460800 write_flash \
  0x10000 frixos.bin
```

## Using flasher_args.json

`flasher_args.json` contains the exact flash parameters used during the build. You can use it directly with esptool:

```bash
esptool.py --port /dev/tty.usbserial-* write_flash $(python3 -c "
import json, sys
a = json.load(open('flasher_args.json'))
args = [a['extra_esptool_args'].get('chip','esp32')]
for addr, f in a['flash_files'].items():
    args += [addr, f]
print(' '.join(args))
")
```
