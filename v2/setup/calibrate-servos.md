# Servo Calibration Guide

The six joint boards (ServoDrive) are I2C slaves on a shared bus. Each needs a
unique address and per-servo calibration before the robot can be driven.

## Joint map

The head MCU addresses joints at **even I2C addresses**. The host-facing joint
order (raw `SetJointAngles(j1..j6)`) and limits:

| Index | Joint | I2C addr | Limit (deg) |
|---|---|---|---|
| j1 | Head (neck pitch) | 2 | +/-15 |
| j2 | Left arm - open (shoulder roll) | 4 | +/-30 |
| j3 | Right arm - lift (pitch) | 6 | +/-180 |
| j4 | Right arm - open (shoulder roll) | 8 | +/-30 |
| j5 | Left arm - lift (pitch) | 10 | +/-180 |
| j6 | Body (waist yaw) | 12 | +/-90 |

> The webcam-follow app (Phase 1) only uses **j6** (body yaw) and optionally
> **j1** (head). The arm joints are not needed for that feature.

## Procedure (ServoToolKit)

1. **Flash the toolkit firmware.** `ServoToolKit` ships its own MCU image
   (`ElectronBot-fw-servo-toolkit.hex`). Flash it onto the head MCU; it
   temporarily replaces the normal app firmware.

2. **Connect ONE servo.** Because un-provisioned servos may share the default
   address, only one may be on the bus while you assign its ID.

3. **Set parameters** in the GUI:
   - **I2C ID / ADDR** - assign from the table above.
   - **Init / zero angle** - move the joint to its mechanical neutral and store.
   - **PID gains** - Kp/Ki/Kv/Kd (firmware defaults: Kp=10, Kd=50, Ki=Kv=0).
   - **Torque limit** - default 0.5; raise carefully if the joint stalls.

4. **Repeat** for all six joints, one at a time.

5. **Re-flash the normal app firmware** (`ElectronBot-fw.hex`).

## I2C command reference (robot9.png / firmware)

| Opcode | Meaning |
|---|---|
| 0x01 | set target angle |
| 0x02 | set velocity |
| 0x03 | set torque |
| 0x11 / 0x12 | read angle / velocity |
| 0x21 | set + save I2C ID |
| 0x22..0x25 | set PID Kp / Ki / Kv / Kd |
| 0x26 | save torque limit |
| 0x27 | save initial angle |
| 0xff | enable / disable motor |

The slave reply always echoes the current angle.

## Notes

- Servo feedback is a **potentiometer on ADC** (not a magnetic encoder),
  linearly calibrated (~250..3000 -> 0..180 deg). If a joint reads wrong angles,
  re-check its calibration range.
- After calibration, verify with `check_robot.py` and a small motion test before
  running the full app.
