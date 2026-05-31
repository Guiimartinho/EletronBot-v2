# Phase 1 - Status

| Item | Status | Notes |
|---|---|---|
| Controller logic | **Verified** | 9 unit tests pass (`pytest tests/`) |
| Python syntax (all modules) | **Verified** | `py_compile` clean |
| SDK ctypes binding (`robot.py`) | Code-complete | **Not run against the DLL/robot** |
| Camera capture + detectors | Code-complete | Needs a camera + (for DNN) model files |
| Virtual-camera output | Code-complete | Needs OBS Virtual Camera installed |
| `--no-robot` vision pipeline | Code-complete | Runnable on any PC with a webcam |
| End-to-end on the robot | **Pending hardware** | Needs driver + calibrated robot |

**What is verified here:** the control law (deadzone, EMA, rate-limit, clamp,
recenter, sign) via unit tests, and that every module is syntactically valid.

**What requires hardware:** loading the actual DLL, connecting to the robot,
real camera/OBS output, and tuning `yaw_sign`/`cam`. Use `--no-robot` to test
the vision half without the device, then `../setup/check_robot.py` to validate
the connection.
