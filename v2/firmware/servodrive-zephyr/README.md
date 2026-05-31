# ServoDrive Firmware - Zephyr Reference (Phase 3)

An RTOS (Zephyr) reference firmware for a single ElectronBot joint controller.

> **Important:** the existing ServoDrive board (STM32F042, 6 KB RAM) **cannot**
> host Zephyr. See [`../../hardware/servodrive-v2/DESIGN.md`](../../hardware/servodrive-v2/DESIGN.md)
> for the analysis and decision. This firmware is a **blueprint for a respun,
> RTOS-capable board** (default target `nucleo_g431rb`) and a place to develop
> and validate the control law. For the current hardware, keep the original
> bare-metal LL build.

## What it does

A 200 Hz control thread mirrors the original DCE position controller:

```
read potentiometer (ADC) -> angle -> DCE(PID) -> sign-magnitude H-bridge PWM
```

An I2C target accepts the same command set as the original (set angle/gains/
torque, get angle, set ID, enable).

## Source map

| File | Role | Status |
|---|---|---|
| `src/pid.c/.h` | DCE position controller | Logic **verified** (host reference test) |
| `src/motor.c/.h` | Sign-magnitude H-bridge (PWM) | Code-complete |
| `src/encoder.c/.h` | Potentiometer feedback (ADC) | Code-complete |
| `src/i2c_slave.c/.h` | I2C target command decode | Decode complete; driver reg = TODO |
| `src/main.c` | 200 Hz control thread + init | Code-complete |
| `app.overlay` | Board aliases (PWM/ADC/I2C) | Draft - confirm for target |

## Build

```bash
# in a Zephyr workspace (see the head firmware README for west setup)
west build -b nucleo_g431rb v2/firmware/servodrive-zephyr
west flash
```

## Tests

```bash
python -m pytest tests/test_dce_reference.py -v
```

The DCE controller is mirrored in Python and run against a simple servo plant to
prove convergence, clamping and rest behavior - validating the control design
without a C toolchain.

## Integration TODOs (need hardware)

1. **I2C target registration** for the chosen SoC (decode logic is done).
2. **Confirm `app.overlay`** PWM channels, ADC channel and pins for the board.
3. **Persist** ID/gains/limits to flash (settings subsystem), like the original
   emulated EEPROM.
4. **Tune** the control period and gains on the real joint.

## Verification status

See [`STATUS.md`](STATUS.md).
