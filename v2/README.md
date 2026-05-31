# ElectronBot v2

New work built on top of the original ElectronBot, kept **separate** from the
upstream tree (`1.Hardware`..`6.Tests`) so the original history stays intact and
future upstream changes can still be merged cleanly.

> Everything here is in **English** and lives under `v2/`. The original numbered
> folders are never modified.

## Phases

| Phase | Folder | What it is | Verified now |
|---|---|---|---|
| **0 - Bring-up** | [`setup/`](setup/) | Driver/flash/calibration guides + read-only connection diagnostic | Script syntax OK |
| **1 - Webcam-follow** | [`webcam-follow/`](webcam-follow/) | Face-tracking virtual webcam (Python, drives the robot via the SDK DLL) | 9 unit tests pass |
| **2 - Zephyr head fw** | [`firmware/electronbot-zephyr/`](firmware/electronbot-zephyr/) | STM32F405 head firmware ported to Zephyr; host SDK unchanged | 3 protocol tests pass |
| **3 - RTOS servo fw** | [`firmware/servodrive-zephyr/`](firmware/servodrive-zephyr/) | RTOS reference servo firmware + MCU decision | 4 DCE tests pass |
| (HW) servo respin | [`hardware/servodrive-v2/`](hardware/servodrive-v2/) | Feasibility/MCU decision for an RTOS-capable joint board | n/a |

Each folder has its own `README.md` and a `STATUS.md` that states precisely what
is verified vs. what needs hardware.

## What "verified" means here

This environment has **no robot and no embedded toolchain**, so:

- **Verified:** pure-logic parts run as real tests on the PC -
  the webcam yaw controller, the USB protocol byte-layout/framing math, and the
  servo DCE control law. Run them:

  ```bash
  python -m pytest v2/webcam-follow/tests -q
  python -m pytest v2/firmware/electronbot-zephyr/tests -q
  python -m pytest v2/firmware/servodrive-zephyr/tests -q
  ```

- **Pending hardware:** driver binding, flashing, the Zephyr builds (need the
  Zephyr SDK + `west`), and anything touching the real USB/SPI/I2C/servos. The
  integration points are marked `TODO` in the code and listed in each `STATUS.md`.

## Design intent

The robot stays "thin": the PC renders the face and computes motion; the firmware
is a bridge that blits pixels and relays joint angles. The Zephyr ports keep the
**exact host USB protocol** (`v2/firmware/electronbot-zephyr/src/protocol.h`), so
the unchanged `6.Tests/.../Sample.exe` is the acceptance test for the head port.

See [`../docs/roadmap.md`](../docs/roadmap.md) for the full rationale and risks.
