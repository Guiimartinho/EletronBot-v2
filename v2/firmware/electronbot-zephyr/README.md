# ElectronBot Head Firmware - Zephyr Port (Phase 2)

A Zephyr RTOS port of the ElectronBot **head** firmware (STM32F405RGT6),
replacing the original CubeMX/HAL + FreeRTOS super-loop while keeping the PC
host SDK **completely unchanged**. The win is maintainability and a modern,
testable driver model - not new behavior.

> Scope: the head board only. The ServoDrive joint MCU (STM32F042, 6 KB RAM) is
> intentionally out of scope for Zephyr - see Phase 3.

## Why the protocol is the hard part

The host (`ElectronBotSDK-LowLevel`) talks a custom raw-bulk protocol:

- One face frame = 240x240x3 = **172800 bytes**, sent in **4 chunks**.
- Per chunk: host reads **32 bytes** on EP IN (handshake + measured joints),
  writes **84x512 = 43008** image bytes on EP OUT, then a **224-byte tail**
  (192 image + 32 extra). `4 x (43008+192) = 172800`.
- The 224-byte short packet is the **end-of-frame delimiter**.
- The 32-byte extra block = `enable + 6 little-endian float joints`.

To keep the host unchanged, this firmware reproduces that framing **byte-for-
byte**. It lives in [`src/protocol.h`](src/protocol.h) and the state machine in
[`src/usb_app.c`](src/usb_app.c).

## Source map

| File | Role | Status |
|---|---|---|
| `src/protocol.h/.c` | Wire protocol constants + extra-data codec | Complete |
| `src/usb_app.c` | Bulk framing state machine | Complete logic; endpoint glue = TODO |
| `src/display.c` | GC9A01 stripe streaming (SPI+GPIO) | Complete; DMA overlap = TODO |
| `src/servo_bus.c` | I2C master to 6 joints | Complete |
| `src/robot.c` | Per-frame orchestration + joint mapping | Complete |
| `src/main.c` | Entry point | Complete |
| `boards/arm/electronbot_f405/` | Custom board (F405, SPI1/I2C1) | Pins = TODO-confirm |

## Build

Requires the Zephyr SDK + `west`. This repo is **not** a Zephyr workspace by
itself; the manifest pulls Zephyr next to the app:

```bash
# one-time
pip install west
west init -l v2/firmware/electronbot-zephyr
west update
west zephyr-export

# build + flash (ST-Link)
west build -b electronbot_f405 v2/firmware/electronbot-zephyr
west flash
```

## Integration TODOs (need hardware to finalize)

1. **USB endpoint glue.** `usb_app.c` has the full framing; wire its
   `on_out_packet` / `on_in_request` to the chosen Zephyr USB stack (legacy
   `usb_device` or `usb_device_next`), expose one OUT (0x01) + one IN (0x81)
   bulk endpoint at VID `0x1001` / PID `0x8023`.
2. **SPI DMA overlap.** `display.c` streams synchronously; switch to the async
   SPI signal API so a stripe DMA overlaps the next USB receive (as the original
   does with its ping-pong buffers).
3. **Confirm board pins** (CS/DC/RST/backlight, DMA stream) against
   `1.Hardware/ElectronBot/Main.SchDoc`.
4. **COLMOD**: defaults to 18-bit (0x3A=0x06) like the original; confirm against
   the panel.

## Verification status

See [`STATUS.md`](STATUS.md). The protocol/codec logic is unit-testable on the
host; the firmware itself has **not** been compiled here (no Zephyr SDK in this
environment) or flashed. The acceptance test for the port is:
**the unmodified `6.Tests/.../Sample.exe` runs identically against this
firmware.**
