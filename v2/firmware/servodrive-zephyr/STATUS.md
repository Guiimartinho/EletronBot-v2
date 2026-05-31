# Phase 3 (ServoDrive RTOS) - Status

| Item | Status | Notes |
|---|---|---|
| MCU/RTOS feasibility decision | Complete | `hardware/servodrive-v2/DESIGN.md` |
| DCE control law (`pid.c`) | **Verified (Python reference)** | 4 tests: convergence, clamp, rest |
| Motor H-bridge driver (`motor.c`) | Code-complete | PWM sign-magnitude |
| Encoder/pot ADC (`encoder.c`) | Code-complete | Linear calibration |
| I2C target decode (`i2c_slave.c`) | Code-complete | Driver registration = TODO |
| Control thread (`main.c`) | Code-complete | 200 Hz, original defaults |
| Board overlay | Draft | Confirm pins for target board |
| Compiles with west/Zephyr SDK | **Not done here** | No Zephyr SDK in this environment |
| Runs on hardware | **Pending hardware** | Needs an RTOS-capable board (respin) |

**Decision recorded:** the F042 (6 KB RAM) cannot host Zephyr; production keeps
the bare-metal LL build. This Zephyr firmware targets a respun, larger MCU
(default `nucleo_g431rb`) and serves as the RTOS blueprint + control-law sandbox.

**Verified without hardware:** the DCE/PID control law converges and is bounded
(Python reference, `tests/test_dce_reference.py`, 4 tests pass).

**Not verified:** compilation, the I2C target on a real bus, PWM/ADC timing.
Requires the toolchain and target hardware.
