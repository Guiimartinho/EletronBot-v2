"""Camera capture + face detection.

v1 uses OpenCV's Haar cascade (fast, dependency-free). Swap for a DNN
detector (res10 SSD) or MediaPipe later for robustness — see README.
"""
import cv2


class FaceTracker:
    def __init__(self, cam_index: int = 0, rotate_180: bool = True):
        self.cap = cv2.VideoCapture(cam_index, cv2.CAP_DSHOW)
        if not self.cap.isOpened():
            raise RuntimeError(f"Could not open camera index {cam_index}")
        self.rotate_180 = rotate_180
        self.det = cv2.CascadeClassifier(
            cv2.data.haarcascades + "haarcascade_frontalface_default.xml"
        )

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
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        faces = self.det.detectMultiScale(gray, 1.2, 5, minSize=(60, 60))
        if len(faces) == 0:
            return None
        x, y, w, h = max(faces, key=lambda f: f[2] * f[3])
        return (x + w / 2.0, y + h / 2.0, w, h)

    def release(self):
        if self.cap.isOpened():
            self.cap.release()
