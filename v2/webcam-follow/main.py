"""Face-tracking virtual webcam for ElectronBot.

Pipeline (all on the PC):
  robot camera -> face detection -> P/velocity control -> body-yaw (j6)
              -> static eyes on the display -> virtual camera output

Examples:
  python main.py                 # full run against the robot
  python main.py --no-robot      # vision + virtual cam only (no hardware)
  python main.py --preview       # also show a local debug window
  python main.py --detector dnn  # use the DNN face detector

Press Ctrl+C (or 'q' in the preview window) to stop; the robot returns to
center on exit.
"""
import argparse
import os
import sys

import cv2

from config import Config
from tracker import FaceTracker
from controller import YawController
from vcam import VCam


def parse_args():
    ap = argparse.ArgumentParser(description="ElectronBot face-tracking webcam")
    ap.add_argument("--no-robot", action="store_true",
                    help="run the vision/virtual-cam pipeline without the robot")
    ap.add_argument("--preview", action="store_true",
                    help="show a local debug window with the tracked face")
    ap.add_argument("--no-vcam", action="store_true",
                    help="do not open the virtual camera (debug only)")
    ap.add_argument("--detector", choices=["haar", "dnn"], default="haar")
    ap.add_argument("--cam", type=int, default=None, help="camera index override")
    return ap.parse_args()


def connect_robot(cfg):
    from robot import Robot
    bot = Robot(cfg.dll_dir, cfg.dll_name)
    if not bot.connect():
        print("Robot did not connect. Check the driver (Zadig/BotDriver) and USB.")
        return None
    print("Robot connected!")
    if os.path.exists(cfg.eyes_path):
        bot.set_eyes(cfg.eyes_path)
    return bot


def main():
    args = parse_args()
    cfg = Config()
    cam_index = args.cam if args.cam is not None else cfg.cam_index

    bot = None
    if not args.no_robot:
        bot = connect_robot(cfg)
        if bot is None:
            return 1
    else:
        print("[no-robot] running vision + virtual-cam only")

    tracker = FaceTracker(cam_index, cfg.camera_rotated_180, args.detector)
    ctl = YawController(cfg)

    probe = tracker.read_upright()
    if probe is None:
        print("Could not read from camera.")
        return 1
    h, w = probe.shape[:2]

    vc = None
    if not args.no_vcam:
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
            if bot is not None:
                bot.set_pose(head=0.0, body=yaw, enable=True)
                bot.sync()

            if vc is not None:
                vc.push(frame, cy, cfg.vertical_recenter)

            if args.preview:
                if face:
                    x, y, fw, fh = int(cx - face[2] / 2), int(cy - face[3] / 2), \
                        int(face[2]), int(face[3])
                    cv2.rectangle(frame, (x, y), (x + fw, y + fh), (0, 255, 0), 2)
                cv2.putText(frame, f"yaw={yaw:6.1f}", (10, 30),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0, 255, 0), 2)
                cv2.imshow("ElectronBot webcam-follow (q to quit)", frame)
                if cv2.waitKey(1) & 0xFF == ord("q"):
                    break
    except KeyboardInterrupt:
        print("\nStopping...")
    finally:
        if bot is not None:
            bot.set_pose(0.0, 0.0, enable=True)
            bot.sync()
            bot.disconnect()
        tracker.release()
        if vc is not None:
            vc.close()
        if args.preview:
            cv2.destroyAllWindows()
    return 0


if __name__ == "__main__":
    sys.exit(main())
