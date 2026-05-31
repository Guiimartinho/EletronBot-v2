# Face-Tracking Webcam (ElectronBot v2 — Phase 1)

Turn ElectronBot into a webcam that physically follows your face. The robot's
waist (body-yaw, joint **j6**, ±90°) rotates to keep your face centered
horizontally; the vertical axis is approximated by digital re-centering (no
joint can tilt the camera). The processed feed is exposed as a **virtual
camera** for Zoom / Teams / OBS.

Everything runs on the PC and drives the robot through the pre-built
`ElectronBotSDK-LowLevel.dll` via `ctypes` — **no C++ build and no firmware
changes required.**

## Architecture

```
robot camera ── face detection ── velocity control ── body-yaw (j6)
            └── static eyes on display          └── virtual camera output
```

| File | Role |
|---|---|
| `robot.py` | ctypes binding to the SDK DLL (`AHK_*` exports) |
| `tracker.py` | camera capture + face detection (undoes 180° mount) |
| `controller.py` | horizontal error → body-yaw (velocity/PI-style) |
| `vcam.py` | virtual-camera output + digital vertical re-centering |
| `main.py` | the loop |
| `config.py` | all tunables |

## Prerequisites

1. **Driver** — the robot must be bound to the libusb driver (`BotDriver`,
   typically via Zadig) so it enumerates as a raw USB device. See the
   project root docs.
2. **OBS Studio** installed — provides the "OBS Virtual Camera" backend used
   by `pyvirtualcam`.
3. **Servos calibrated** (I2C IDs, zero, limits) with `ServoToolKit`.
4. **Python** — must match the DLL bitness (the bundled SDK DLLs are likely
   32-bit; use a 32-bit Python if `ctypes` fails to load them).

## Install

```bash
pip install -r requirements.txt
```

Place a 240×240 face image at `assets/eyes.jpg` (shown on the robot's display).

## Run

```bash
python main.py
```

On success it prints `Robot connected!` and starts the virtual camera. Select
**OBS Virtual Camera** in your meeting app.

## First-run calibration

- **`yaw_sign`** (in `config.py`): with your face to one side, check whether
  the waist turns to *center* you. If it turns away, flip `yaw_sign` to `-1`.
- **Oscillation**: increase `deadzone`/`ema`, or reduce `kp`/`max_rate`.
- **Camera index**: the robot camera may not be index `0`; try `1`, `2`, ...

## Known limits (Phase 1)

- Horizontal tracking is mechanical; **vertical is digital only** (the camera
  is in the torso and no joint tilts it).
- Each `Sync()` uploads a full frame, so the display is always driven — keep
  the rate ~20–30 Hz and use a static `eyes.jpg`.
- Windows-only (libusb-win32 transport).
- The robot camera is ~0.3 MP; digital cropping costs resolution.

## Next (v2+)

- DNN/MediaPipe detector + a tracker (KCF/CSRT) between detections.
- Use head pitch (j1, ±15°) for subtle "attention" cues.
- Show a reactive expression on the display instead of static eyes.
