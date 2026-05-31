"""Face-tracking virtual webcam for ElectronBot.

Pipeline (all on the PC):
  robot camera -> face detection -> P/velocity control -> body-yaw (j6)
              -> static eyes on the display -> virtual camera output

Press Ctrl+C to stop; the robot returns to center on exit.
"""
import os
import sys

from config import Config
from robot import Robot
from tracker import FaceTracker
from controller import YawController
from vcam import VCam


def main() -> int:
    cfg = Config()

    bot = Robot(cfg.dll_dir, cfg.dll_name)
    if not bot.connect():
        print("Robot did not connect. Check the driver (Zadig/BotDriver) and USB.")
        return 1
    print("Robot connected!")

    if os.path.exists(cfg.eyes_path):
        bot.set_eyes(cfg.eyes_path)

    tracker = FaceTracker(cfg.cam_index, cfg.camera_rotated_180)
    ctl = YawController(cfg)

    probe = tracker.read_upright()
    if probe is None:
        print("Could not read from camera.")
        return 1
    h, w = probe.shape[:2]
    vc = VCam(w, h, cfg.target_fps)
    print(f"Virtual camera started ({w}x{h} @ {cfg.target_fps}fps). "
          f"Select 'OBS Virtual Camera' in your meeting app.")

    try:
        while True:
            frame = tracker.read_upright()
            if frame is None:
                continue
            face = tracker.biggest_face(frame)
            cx = face[0] if face else None
            cy = face[1] if face else None

            yaw = ctl.update(cx, w)
            bot.set_pose(head=0.0, body=yaw, enable=True)
            bot.sync()

            vc.push(frame, cy, cfg.vertical_recenter)
    except KeyboardInterrupt:
        print("\nStopping...")
    finally:
        bot.set_pose(0.0, 0.0, enable=True)
        bot.sync()
        bot.disconnect()
        tracker.release()
        vc.close()
    return 0


if __name__ == "__main__":
    sys.exit(main())
