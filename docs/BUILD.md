# Building Frixos

This document covers building the Frixos firmware locally and with Docker/Podman containers.

## Prerequisites

### Environment file

All build methods require a `.env` file with your API keys:

```bash
cp env.example .env
# Edit .env and add your OpenWeatherMap API key
```

The build system reads `.env` and generates `main/include/config.h` automatically — never edit `config.h` directly.

## Local Build

Requires ESP-IDF v5.4+ installed on your machine.

```bash
# Activate ESP-IDF
source ~/.espressif/tools/activate_idf_v5.4.sh

# Build
idf.py build
```

Build artifacts are written to the `build/` directory. See [FLASH.md](FLASH.md) for flashing instructions.

### Useful commands

```bash
idf.py reconfigure            # after sdkconfig changes
idf.py update-dependencies    # after editing main/idf_component.yml
idf.py fullclean              # remove all build artifacts
```

## Docker Build

```bash
docker run --rm -v $PWD:/project -w /project \
  espressif/idf:v5.4 \
  idf.py build
```

## Podman Build

```bash
podman run --rm -v $PWD:/project -w /project \
  espressif/idf:v5.4 \
  idf.py build
```

> **Note:** If Podman runs in rootless mode and the build fails with permission errors, add `:Z` to the volume mount: `-v $PWD:/project:Z`

## Flashing from a Container

Pass the serial device into the container to flash and monitor directly.

### Docker

```bash
docker run --rm -v $PWD:/project -w /project \
  --device /dev/ttyUSB0 \
  espressif/idf:v5.4 \
  idf.py -p /dev/ttyUSB0 flash monitor
```

### Podman

```bash
podman run --rm -v $PWD:/project -w /project \
  --device /dev/ttyUSB0 \
  espressif/idf:v5.4 \
  idf.py -p /dev/ttyUSB0 flash monitor
```

> **Device path:** Replace `/dev/ttyUSB0` with your adapter's actual path. Common values:
> - Linux: `/dev/ttyUSB0`, `/dev/ttyACM0`
> - macOS: `/dev/tty.usbserial-*`, `/dev/cu.usbserial-*`
>
> On macOS, Docker Desktop requires USB passthrough configuration in Settings > Resources > USB Devices. Podman on macOS does not support `--device` passthrough natively — flash from the host using `esptool.py` instead (see [FLASH.md](FLASH.md)).

## Build Output

See [FLASH.md](FLASH.md) for a description of the build artifacts and how to flash them.
