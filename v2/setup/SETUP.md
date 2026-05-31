# Phase 0 - Bring-up & Setup

This is the foundation every other phase depends on. It gets a freshly built (or
freshly received) ElectronBot to the point where the PC can talk to it, the
display works, and the servos are calibrated.

> Nothing here changes the original firmware/software. It is setup + verification.

## 0. Prerequisites checklist

- [ ] ElectronBot assembled and connected to the PC via USB-C.
- [ ] Windows 10/11 (the host stack is Windows-only - libusb-win32 transport).
- [ ] [Zadig](https://zadig.akeo.ie/) downloaded (for driver binding).
- [ ] An ST-Link V2 / V2-1 (for flashing firmware over SWD), if building firmware.
- [ ] [OBS Studio](https://obsproject.com/) installed (needed later, for the
      virtual-camera output in Phase 1).
- [ ] Python 3.x installed. **Bitness must match the SDK DLLs** (the bundled SDK
      DLLs are likely 32-bit; if `ctypes` fails to load them, use a 32-bit Python).

## 1. Install the USB driver

The robot is a **raw bulk USB device**, not a COM port. Windows must bind it to
the libusb driver before any host tool can open it.

Option A - bundled installer:
- Run `3.Software/_Tools/BotDriver` (libusb-win32 package).

Option B - Zadig (recommended, binds the exact running-app VID/PID):
1. Plug in the robot and run the **app firmware** (not the bootloader).
2. Open Zadig -> Options -> *List All Devices*.
3. Select the ElectronBot interface. The running app enumerates as
   **VID 0x1001 / PID 0x8023** (`ElectronBot@PZH`).
4. Pick **libusb-win32** (or WinUSB) as the driver and click *Replace Driver*.

> The shipped `BotDriver` INF only covers the **bootloader** ID
> (`VID_0483 / PID_5740`), so the app ID usually needs the manual Zadig step.
> This is item #10 in `docs/recommendations-upstream.md`.

Verify with `check_robot.py` (see section 4).

## 2. Flash firmware (only if building from source)

For the turnkey test, use the prebuilt hex:

```
6.Tests/TestDisplayUSB/_Released/STM32F4 Firmware/ElectronBot-fw.hex
```

Flash over ST-Link/SWD (OpenOCD, target `stm32f4x`), e.g.:

```bash
openocd -f interface/stlink.cfg -f target/stm32f4x.cfg \
        -c "program ElectronBot-fw.hex verify reset exit"
```

## 3. Calibrate the servos

Each joint board is an I2C slave with its own address; out of the box the
addresses are unset, so calibrate **one servo at a time**. See
[`calibrate-servos.md`](calibrate-servos.md) for the full procedure with
`ServoToolKit`. Summary:

1. Flash `3.Software/_Tools/ServoToolKit`'s own firmware
   (`ElectronBot-fw-servo-toolkit.hex`) onto the head MCU.
2. Connect a single servo, set its **I2C ID** (one of 2, 4, 6, 8, 10, 12),
   **zero/init angle**, **PID gains** (Kp/Ki/Kv/Kd) and **torque limit**.
3. Repeat for all six joints.
4. Re-flash the normal app firmware.

## 4. Verify the connection

Run the diagnostic (no robot motion, read-only):

```bash
cd v2/setup
python check_robot.py
```

It will:
- enumerate available cameras (to find the robot's UVC camera index),
- locate and load `ElectronBotSDK-LowLevel.dll`,
- attempt `Connect()` and read back the 6 joint angles,
- print a clear PASS/FAIL summary.

## 5. Run the turnkey display test

```
6.Tests/TestDisplayUSB/_Released/Windows EXE/Sample.exe
```

Success = console prints **"Robot connected!"** and `happy.mp4` animates on the
robot's round display.

## Definition of Done

- [ ] Driver bound; `check_robot.py` reports the device is open.
- [ ] `Sample.exe` shows video on the face.
- [ ] All six servos respond and are calibrated.
- [ ] Power is stable when multiple servos move (no brown-out / disconnect).

Once all four are checked, Phase 1 (`v2/webcam-follow/`) is ready to run.
