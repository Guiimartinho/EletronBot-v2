"""Thin ctypes binding to ElectronBotSDK-LowLevel.dll (AHK build).

Wraps the C-ABI exports (AHK_*) so the robot can be driven from Python with
no C++ compilation. Every Sync() uploads a full 240x240 RGB frame plus the
6 joint set-points (the joints ride in the frame's 32-byte tail).

Raw joint order (from the SDK): j1=head, j2=left-arm-open, j3=right-arm-lift,
j4=right-arm-open, j5=left-arm-lift, j6=body-yaw.
Limits (deg): head +/-15, body +/-90, arm-open +/-30, arm-lift +/-180.
"""
import ctypes
import os


class Robot:
    def __init__(self, dll_dir: str, dll_name: str = "ElectronBotSDK-LowLevel.dll"):
        dll_dir = os.path.abspath(dll_dir)
        if not os.path.isdir(dll_dir):
            raise FileNotFoundError(f"DLL dir not found: {dll_dir}")
        # Let Windows resolve the sibling DLLs (USBInterface/opencv/libusb).
        os.add_dll_directory(dll_dir)
        self.lib = ctypes.CDLL(os.path.join(dll_dir, dll_name))

        self.lib.AHK_New.restype = ctypes.c_void_p
        self.lib.AHK_Connect.argtypes = [ctypes.c_void_p]
        self.lib.AHK_Connect.restype = ctypes.c_bool
        self.lib.AHK_Disconnect.argtypes = [ctypes.c_void_p]
        self.lib.AHK_Sync.argtypes = [ctypes.c_void_p]
        self.lib.AHK_Sync.restype = ctypes.c_bool
        self.lib.AHK_SetImageSrc_Path.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
        self.lib.AHK_SetJointAngles.argtypes = (
            [ctypes.c_void_p] + [ctypes.c_float] * 6 + [ctypes.c_bool]
        )
        self.lib.AHK_GetJointAngles.argtypes = [
            ctypes.c_void_p, ctypes.POINTER(ctypes.c_float)
        ]
        self.p = self.lib.AHK_New()

    def connect(self) -> bool:
        return bool(self.lib.AHK_Connect(self.p))

    def disconnect(self) -> None:
        self.lib.AHK_Disconnect(self.p)

    def set_eyes(self, path: str) -> None:
        """Set the 240x240 image shown on the robot's face (ASCII path only)."""
        self.lib.AHK_SetImageSrc_Path(self.p, path.encode("ascii"))

    def set_pose(self, head: float = 0.0, body: float = 0.0,
                 enable: bool = True) -> None:
        """Queue joint set-points. Only head (j1) and body-yaw (j6) are used."""
        self.lib.AHK_SetJointAngles(self.p, float(head), 0.0, 0.0, 0.0, 0.0,
                                    float(body), enable)

    def sync(self) -> bool:
        """Upload the queued frame + joints to the robot."""
        return bool(self.lib.AHK_Sync(self.p))

    def get_joints(self) -> list:
        """Return the 6 measured joint angles (reflects the last Sync)."""
        buf = (ctypes.c_float * 6)()
        self.lib.AHK_GetJointAngles(self.p, buf)
        return list(buf)
