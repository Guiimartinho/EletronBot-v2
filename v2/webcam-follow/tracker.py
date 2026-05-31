"""Camera capture + face detection.

Capture is separated from detection: the detector is pluggable (see
`detector.py`). The tracker only owns the camera and the 180-degree un-rotation
of the robot's upside-down camera mount.
"""
import cv2

from detector import make_detector


class FaceTracker:
    def __init__(self, cam_index: int = 0, rotate_180: bool = True,
                 detector_kind: str = "haar"):
        self.cap = cv2.VideoCapture(cam_index, cv2.CAP_DSHOW)
        if not self.cap.isOpened():
            raise RuntimeError(f"Could not open camera index {cam_index}")
        self.rotate_180 = rotate_180
        self.detector = make_detector(detector_kind)

    def read_upright(self):
        """Read a frame and undo the robot camera's 180-degree mounting."""
        ok, frame = self.cap.read()
        if not ok:
            return None
        if self.rotate_180:
            frame = cv2.rotate(frame, cv2.ROTATE_180)
        return frame

    def biggest_face(self, frame):
        """Return (cx, cy, w, h) of the largest face, or None."""
        return self.detector.detect(frame)

    def release(self):
        if self.cap.isOpened():
            self.cap.release()
