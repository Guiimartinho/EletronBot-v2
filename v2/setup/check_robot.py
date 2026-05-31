"""Phase 0 diagnostic for ElectronBot.

Read-only checks (no robot motion): enumerate cameras, locate and load the SDK
DLL, attempt to connect, and read back the 6 joint angles. Prints a PASS/FAIL
summary so you can confirm the setup before running any feature app.

Usage:
    python check_robot.py [--dll-dir PATH] [--max-cam 5]
"""
import argparse
import os
import sys
import ctypes


DEFAULT_DLL_DIR = os.path.join(
    os.path.dirname(__file__),
    r"..\..\3.Software\_Tools\AHK-ExpansionPack\ElectronBotSDK",
)
DLL_NAME = "ElectronBotSDK-LowLevel.dll"


def check_python_bitness():
    bits = 64 if sys.maxsize > 2**32 else 32
    print(f"[info] Python is {bits}-bit ({sys.version.split()[0]})")
    if bits == 64:
        print("       NOTE: the bundled SDK DLLs may be 32-bit. If the DLL fails "
              "to load, retry with a 32-bit Python.")
    return bits


def enumerate_cameras(max_cam: int):
    print("\n=== Cameras ===")
    try:
        import cv2
    except ImportError:
        print("[skip] opencv-python not installed (pip install opencv-python)")
        return []
    found = []
    for i in range(max_cam):
        cap = cv2.VideoCapture(i, cv2.CAP_DSHOW)
        ok = cap.isOpened()
        if ok:
            r, frame = cap.read()
            res = f"{frame.shape[1]}x{frame.shape[0]}" if r and frame is not None else "?"
            print(f"  [ok]  index {i}: opened, frame {res}")
            found.append(i)
        cap.release()
    if not found:
        print("  [warn] no cameras found")
    else:
        print(f"  -> candidate indices: {found} "
              f"(the robot's UVC camera is one of these)")
    return found


def load_dll(dll_dir: str):
    print("\n=== SDK DLL ===")
    dll_dir = os.path.abspath(dll_dir)
    dll_path = os.path.join(dll_dir, DLL_NAME)
    if not os.path.isfile(dll_path):
        print(f"  [fail] not found: {dll_path}")
        return None
    print(f"  [ok]  found: {dll_path}")
    try:
        os.add_dll_directory(dll_dir)
        lib = ctypes.CDLL(dll_path)
        print("  [ok]  loaded")
        return lib
    except OSError as e:
        print(f"  [fail] could not load DLL: {e}")
        print("        likely a bitness mismatch or a missing sibling DLL "
              "(USBInterface/opencv/libusb).")
        return None


def try_connect(lib):
    print("\n=== Robot connection ===")
    lib.AHK_New.restype = ctypes.c_void_p
    lib.AHK_Connect.argtypes = [ctypes.c_void_p]
    lib.AHK_Connect.restype = ctypes.c_bool
    lib.AHK_Disconnect.argtypes = [ctypes.c_void_p]
    lib.AHK_GetJointAngles.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_float)]

    p = lib.AHK_New()
    if not lib.AHK_Connect(p):
        print("  [fail] Connect() returned false")
        print("        -> check the driver binding (Zadig / BotDriver) and that "
              "the app firmware (not bootloader) is running.")
        return False
    print("  [ok]  connected (VID 0x1001 / PID 0x8023)")
    buf = (ctypes.c_float * 6)()
    lib.AHK_GetJointAngles(p, buf)
    print(f"  [info] joint angles read-back: {[round(x, 2) for x in buf]}")
    lib.AHK_Disconnect(p)
    print("  [ok]  disconnected cleanly")
    return True


def main():
    ap = argparse.ArgumentParser(description="ElectronBot Phase-0 diagnostic")
    ap.add_argument("--dll-dir", default=DEFAULT_DLL_DIR)
    ap.add_argument("--max-cam", type=int, default=5)
    args = ap.parse_args()

    print("ElectronBot - Phase 0 diagnostic\n" + "=" * 34)
    check_python_bitness()
    cams = enumerate_cameras(args.max_cam)
    lib = load_dll(args.dll_dir)
    connected = try_connect(lib) if lib else False

    print("\n=== Summary ===")
    print(f"  cameras found : {'yes' if cams else 'NO'}")
    print(f"  dll loaded    : {'yes' if lib else 'NO'}")
    print(f"  robot connect : {'yes' if connected else 'NO'}")
    ok = bool(cams) and connected
    print("\nRESULT:", "PASS - ready for Phase 1" if ok else
          "FAIL - resolve the items above before running the app")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
