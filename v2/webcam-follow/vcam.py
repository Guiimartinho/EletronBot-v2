"""Virtual-camera output with optional digital vertical re-centering.

The robot camera cannot tilt mechanically (no joint moves it vertically), so
vertical tracking is approximated by shifting the frame so the face sits on
the output's vertical center. Requires the OBS Virtual Camera backend.
"""
import cv2
import numpy as np
import pyvirtualcam


class VCam:
    def __init__(self, width: int, height: int, fps: int = 30):
        self.w = width
        self.h = height
        self.cam = pyvirtualcam.Camera(width=width, height=height, fps=fps)

    def push(self, frame, face_cy=None, recenter: bool = True) -> None:
        if recenter and face_cy is not None:
            shift = int(self.h / 2 - face_cy)
            m = np.float32([[1, 0, 0], [0, 1, shift]])
            frame = cv2.warpAffine(frame, m, (self.w, self.h))
        self.cam.send(cv2.cvtColor(frame, cv2.COLOR_BGR2RGB))
        self.cam.sleep_until_next_frame()

    def close(self) -> None:
        self.cam.close()
