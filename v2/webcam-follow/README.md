# Face-Tracking Webcam (ElectronBot v2 - Phase 1)

Turn ElectronBot into a webcam that physically follows your face. The robot's
waist (body-yaw, joint **j6**, +/-90 deg) rotates to keep your face centered
horizontally; the vertical axis is approximated by digital re-centering (no
joint can tilt the camera). The processed feed is exposed as a **virtual
camera** for Zoom / Teams / OBS.

Everything runs on the PC and drives the robot through the pre-built
`ElectronBotSDK-LowLevel.dll` via `ctypes` - **no C++ build and no firmware
changes required.**

## Architecture

```
robot camera -- face detection -- velocity control -- body-yaw (j6)
            \-- static eyes on display          \-- virtual camera output
```

| File | Role |
|---|---|
| `robot.py` | ctypes binding to the SDK DLL (`AHK_*` exports) |
| `detector.py` | pluggable face detector (Haar or DNN) |
| `tracker.py` | camera capture + un-rotates the 180-deg mount |
| `controller.py` | horizontal error -> body-yaw (velocity/PI-style) |
| `vcam.py` | virtual-camera output + digital vertical re-centering |
| `main.py` | the loop + CLI |
| `config.py` | all tunables |
| `tests/` | unit tests for the controller (no hardware needed) |

## Prerequisites

See [`../setup/SETUP.md`](../setup/SETUP.md) (Phase 0). In short:

1. **Driver** bound via Zadig/BotDriver so the robot is a raw USB device.
2. **OBS Studio** installed (provides the "OBS Virtual Camera" backend).
3. **Servos calibrated** with `ServoToolKit`.
4. **Python** matching the DLL bitness (the bundled SDK DLLs are likely 32-bit).

Run the Phase-0 diagnostic first: `python ../setup/check_robot.py`.

## Install

```bash
pip install -r requirements.txt
```

Place a 240x240 face image at `assets/eyes.jpg` (shown on the robot's display).

## Run

```bash
python main.py                 # full run against the robot
python main.py --no-robot      # vision + virtual cam only (no hardware)
python main.py --preview       # also show a local debug window
python main.py --detector dnn  # use the DNN detector (needs model files)
python main.py --cam 1         # override the camera index
```

On success it prints `Robot connected!` and starts the virtual camera. Select
**OBS Virtual Camera** in your meeting app.

`--no-robot` lets you validate the camera, face detection and virtual-camera
output on any PC, before the robot/driver are set up.

## First-run calibration

- **`yaw_sign`** (in `config.py`): with your face to one side, check whether the
  waist turns to *center* you. If it turns away, flip `yaw_sign` to `-1`.
- **Oscillation**: increase `deadzone`/`ema`, or reduce `kp`/`max_rate`.
- **Camera index**: the robot camera may not be index `0`; `check_robot.py`
  lists candidates, or use `--cam N`.

## DNN detector (optional, more robust)

Download these into `models/` to use `--detector dnn`:
- `deploy.prototxt`
- `res10_300x300_ssd_iter_140000.caffemodel`

(OpenCV's standard res10 SSD face model; widely mirrored.) Without them the app
falls back to the Haar detector.

## Tests

```bash
pip install -r requirements-dev.txt
python -m pytest tests/ -v
```

The controller tests are pure logic and run without the robot or a camera.

## Known limits (Phase 1)

- Horizontal tracking is mechanical; **vertical is digital only** (the camera is
  in the torso and no joint tilts it).
- Each `Sync()` uploads a full frame, so the display is always driven - keep the
  rate ~20-30 Hz and use a static `eyes.jpg`.
- Windows-only (libusb-win32 transport).
- The robot camera is ~0.3 MP; digital cropping costs resolution.

## Next (v2+)

- A tracker (KCF/CSRT) between detections for smoothness.
- Use head pitch (j1, +/-15 deg) for subtle "attention" cues.
- Show a reactive expression on the display instead of static eyes.
