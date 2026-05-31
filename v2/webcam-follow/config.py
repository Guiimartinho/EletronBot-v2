"""Configuration for the face-tracking webcam app.

All tunable parameters live here. Adjust during first-run calibration
(especially `yaw_sign`, see README).
"""
from dataclasses import dataclass


@dataclass
class Config:
    # --- DLL / connection ---
    # Folder containing ElectronBotSDK-LowLevel.dll + USBInterface.dll +
    # opencv_world*.dll + libusb0.dll. Defaults to the bundled AHK SDK build.
    dll_dir: str = r"..\..\3.Software\_Tools\AHK-ExpansionPack\ElectronBotSDK"
    dll_name: str = "ElectronBotSDK-LowLevel.dll"

    # --- camera ---
    cam_index: int = 0          # robot's UVC camera (may not be 0; see README)
    camera_rotated_180: bool = True   # robot camera is mounted upside-down

    # --- control loop (horizontal axis -> body yaw, joint j6) ---
    kp: float = 6.0             # proportional gain (deg per normalized error tick)
    ema: float = 0.4            # error smoothing factor [0..1], higher = snappier
    deadzone: float = 0.06      # ignore error within +/- this fraction of half-width
    max_rate: float = 4.0       # max yaw change per tick (deg) -> rate limit
    yaw_limit: float = 85.0     # clamp (mechanical limit is +/- 90)
    yaw_sign: int = +1          # +1 or -1; determine on first run (see README)
    recenter_gain: float = 0.02 # how fast yaw returns to 0 when no face

    # --- display ---
    eyes_path: str = "assets/eyes.jpg"   # 240x240 image shown on the robot face

    # --- output ---
    target_fps: int = 30
    vertical_recenter: bool = True       # digital vertical re-centering of the feed
