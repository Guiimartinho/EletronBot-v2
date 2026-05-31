# Phase 0 - Status

| Item | Status | Notes |
|---|---|---|
| Setup guide (`SETUP.md`) | Complete | Driver, flashing, calibration, verification |
| Servo calibration guide | Complete | Per-joint procedure + I2C command map |
| Diagnostic script (`check_robot.py`) | Code-complete | Syntax-checked; **not run against hardware** |
| Driver binding (Zadig) | **Pending hardware** | Manual step on the user's PC |
| Servo calibration | **Pending hardware** | Requires the physical robot + ServoToolKit |
| Turnkey display test | **Pending hardware** | Run `Sample.exe` with the robot connected |

**What is verified:** the diagnostic script is valid Python and self-contained.

**What requires the physical robot:** driver binding, flashing, calibration, and
the display test cannot be validated without the device. Run `check_robot.py` on
the machine with the robot to confirm.
