# Recommendations for ElectronBot maintainers

These notes come from an in-depth read of the firmware and host SDK while planning a
"v2" line of work (a face-tracking webcam mode and a possible Zephyr port). They are
meant as constructive feedback for the upstream project. Each item references the file
where the behavior lives.

## Protocol & API

1. **Document the USB wire protocol explicitly.** The frame contract only exists
   implicitly in `electron_low_level.cpp` (`SyncTask`): one 240x240 RGB888 frame =
   `4 x (84 x 512 B + a 224 B tail)`, and the 224-byte short packet is the
   end-of-frame delimiter the firmware keys on. The trailing 32 bytes carry
   `byte[0]=enable + 6 little-endian float joint set-points`. A one-page spec would
   make alternative hosts/firmware (and ports) far easier.

2. **The device is a raw-bulk pipe, not a real CDC serial port.** It enumerates as CDC
   but line-coding is ignored and traffic is raw bulk on `EP1_OUT 0x01 / EP1_IN 0x81`.
   Worth stating clearly so integrators don't try to open it as a COM port.

3. **Joint indexing is scrambled and undocumented in the core SDK.** Raw
   `SetJointAngles(j1..j6)` order is `j1=head, j2=left-arm-open, j3=right-arm-lift,
   j4=right-arm-open, j5=left-arm-lift, j6=body-yaw` (recovered from the AHK wrapper
   comments). Limits: head +/-15, body +/-90, arm-open +/-30, arm-lift +/-180 (deg).
   Please document the canonical mapping + limits next to the public API.

4. **`GetJointAngles` needs two reads to return fresh data.** The AHK wrapper does
   `Get -> Sleep 10 ms -> Get`. Either document this handshake or fix it so a single
   call returns the latest telemetry.

5. **`USB_ScanDevice(pid, vid)` argument order is inverted relative to its name.** This
   is an easy source of bugs; consider renaming the parameters or the call.

6. **Camera is physically mounted rotated 180 degrees.** `electron_sdk_unity_bridge.cpp`
   uses `flip(img, -1)` (both axes). Documenting the mounting orientation (or exposing it
   as a constant) avoids confusion for anyone consuming the camera feed.

## Build & packaging

7. **Make the C++/CMake build portable.** `CMakeLists.txt` hardcodes
   `D:\Libraries\Cpp\OpenCV_lib` / `Boost_lib` and targets a *debug* OpenCV 3.4.8
   (`opencv_world348d`). Prefer `find_package(OpenCV)` (or a vendored tree via a
   relative path) and drop the unused Boost dependency.

8. **There are three divergent copies of the SDK** (the canonical `SDK/` tree, the
   OpenCV-4.5.5 build under `_Tools/AHK-ExpansionPack`, and the DLLs vendored into the
   Unity `Assets/Plugins`). They are not identical. Consolidating to one source of
   truth would prevent drift.

9. **`ElectronPlayer::SetPose/GetPose` are empty stubs** because the LowLevel header
   bundled with the Player is an older copy without `SetJointAngles`. Syncing the
   headers (or removing the dead methods) would avoid the impression the Player can
   drive servos.

10. **Driver INF binds the bootloader ID, not the running app ID.** The shipped
    `BotDriver` `.inf` covers `VID_0483&PID_5740` (ST bootloader) while the app uses
    `0x1001/0x8023`. New users typically need a manual Zadig step; shipping a matching
    INF (or documenting the step) would smooth first-run setup.

## Hardware notes (for documentation)

11. **Servo feedback is a potentiometer on ADC, not a magnetic encoder**, linearly
    calibrated (~250..3000 -> 0..180 deg). Worth stating in the servo docs alongside the
    calibration range.

12. **Power budget.** Six servos plus the display are powered from a single USB port
    through the SensorBoard hub. A note on simultaneous-motion current draw / brown-out
    risk would help builders.

---

*Filed as part of a downstream v2 effort. Happy to turn any of these into issues or PRs
against the upstream repository if useful.*
