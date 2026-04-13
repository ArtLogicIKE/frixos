# Flashing the onboard ESP32 via the P1 PGM connector

This article explains how to flash firmware to the **ESP32** on your Frixos projection clock using the **P1 PGM** programming header, a **USB-to-serial (UART) adapter**, and the same tools used to build official firmware (ESP-IDF).

---

## Table of contents

1. [When to use P1 PGM](#when-to-use-p1-pgm)
2. [What you need](#what-you-need)
3. [Connector and orientation](#connector-and-orientation)
4. [Pinout](#pinout)
5. [Wiring the adapter](#wiring-the-adapter)
6. [Entering download (bootloader) mode](#entering-download-bootloader-mode)
7. [Flashing from a computer](#flashing-from-a-computer)
8. [Troubleshooting](#troubleshooting)

---

## When to use P1 PGM

The USB connector on the device is **for power only**—it does not present a serial port to your PC. For factory-style flashing or **recovery** when over-the-air or web updates are not enough, you use **UART** on **P1 PGM** with a separate **USB-to-serial** adapter.

---

## What you need

| Item | Notes |
|------|--------|
| **USB-to-serial adapter** | Must use **3.3 V** I/O (TTL). **Do not** use 5 V TTL on ESP32 pins. Common chips: CP2102, CH340, FT232RL (jumpered to 3.3 V). |
| **Cable or breakout for P1** | **6-pin JST, 1.0 mm pitch** (JST-SH style 1 mm). Use a matching pigtail, or carefully probe with fine dupont wires if you do not have the correct connector. |
| **Computer** | Windows, macOS, or Linux with **ESP-IDF** installed (same major version as the firmware you are building). |
| **Firmware** | Official releases or your own `idf.py build` output (`build/`). |

**Reference diagram:** pin names and numbering match the schematic graphic **P1 PGM pinout** (see repository `artwork/P1 PGM pinout.png` or the equivalent image on the support site).

---

## Connector and orientation

- **Designator:** P1 (PGM / programming)
- **Mechanical:** **6-position JST, 1.0 mm pitch**
- **Keying:** The schematic shows a **small red dot** next to **pin 1**. Align your cable or counting so **pin 1 is TXD**—if the plug is reversed, TX/RX and the rest will be wrong.

Pins **7** and **8** on the drawing are tied to **GND** with the shield/tabs; your 6-pin cable uses **pins 1–6** only.

---

## Pinout

Numbering is **1 at the end marked by the red dot**, then 2–6 along the connector (see pinout artwork).

| Pin | Signal | Description |
|-----|--------|-------------|
| **1** | **TXD** | ESP32 UART0 TX → connect to **RX** on your USB-serial adapter |
| **2** | **IO0** | Boot strap: hold **LOW** while releasing reset to enter UART download mode |
| **3** | **RXD** | ESP32 UART0 RX → connect to **TX** on your USB-serial adapter |
| **4** | **GND** | Common ground (required) |
| **5** | **+3V3** | **3.3 V** supply pin for the ESP32 rail—see [power notes](#wiring-the-adapter) |
| **6** | **EN** | Chip enable / reset (active high); pulse **LOW** then high to reset |

---

## Wiring the adapter

### UART lines (always)

- **GND (P1 pin 4)** ↔ **GND** on the adapter  
- **P1 TXD (pin 1)** ↔ **RX** on the adapter  
- **P1 RXD (pin 3)** ↔ **TX** on the adapter  

UART lines are **crossed**: board **TX** goes to adapter **RX**, and board **RX** to adapter **TX**.

### Power (choose one approach)

**Recommended — board already powered**

1. Power the clock from its **normal USB / power input** as usual.  
2. Connect **only GND + TXD + RXD** (and IO0/EN as needed for boot mode) from the adapter.  
3. **Do not** connect the adapter’s **3.3 V** output to **P1 pin 5** while the board is also powered from another source, unless you know the design allows it—**avoid dual-feeding** 3.3 V rails.

**Alternate — adapter supplies 3.3 V**

If the board is **not** powered elsewhere, you may use a **3.3 V-only** serial adapter that can supply enough current for the ESP32 and related circuitry, wired **GND** and **+3V3 (pin 5)** per your hardware engineer’s guidance. **Never apply 5 V** to pin 5 or to the signal pins.

### Logic levels

All signals on P1 are **3.3 V**. Using **5 V TTL** on RXD/TXD/IO0/EN can damage the ESP32.

---

## Entering download (bootloader) mode

The ROM bootloader runs when **GPIO0 is held low** as the chip comes out of reset.

**Typical manual sequence (works with simple TX/RX/GND adapters):**

1. Connect **GND**, **TXD**, **RXD** as above.  
2. **Hold IO0 (pin 2) shorted to GND (pin 4).**  
3. While keeping IO0 low, briefly pull **EN (pin 6) low** to GND and release (or power-cycle the board if EN is awkward to reach).  
4. Release **IO0** after the chip has entered download mode (often **~1 second** after reset is enough; if `idf.py flash` still fails, try holding IO0 low until flashing starts).

Some adapters expose **RTS/DTR** and custom cables can automate toggling **EN** and **IO0**; that is optional and depends on your cable wiring.

---

## Flashing from a computer

1. Install and export the **ESP-IDF** environment (match the project’s IDF version, e.g. 5.x).  
2. Open a terminal in the firmware project directory.  
3. Build if needed:  
   `idf.py build`  
4. Flash using the serial port created by your adapter, for example:

   ```bash
   idf.py -p COM3 flash
   ```

   On Linux/macOS the port is often `/dev/ttyUSB0` or `/dev/tty.usbserial-*`.

5. Optional serial monitor:

   ```bash
   idf.py -p COM3 monitor
   ```

Use the **same** `flash` / `partition` / **SPIFFS** expectations as the official Frixos build (see project README: flash size, custom partition table, SPIFFS image if you ship a full image).

---

## Troubleshooting

| Symptom | Things to check |
|---------|------------------|
| **Port not listed** | USB cable and drivers (CP210x, CH340, FTDI). Try another USB socket. |
| **“Failed to connect” / no sync** | IO0 not low at reset; wrong TX/RX swap; GND missing; wrong baud not usually the issue—first fix wiring and boot mode. |
| **Garbled serial** | Wrong UART pins; baud mismatch in monitor (ROM often **115200** or **74880** depending on crystal—try both in monitor). |
| **Flash errors** | Poor contact on 1 mm JST; insufficient power; bad USB cable; use shorter wires. |

If you are unsure about powering the board from **+3V3** on P1, prefer **normal device power + signal-only** wiring from the adapter.

---

## Related

- **Build and flash overview** — project `README.md` (Build & Flash, Serial flashing recovery).  
- **Visual pinout** — `artwork/P1 PGM pinout.png`.
