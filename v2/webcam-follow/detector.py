"""Pluggable face detectors.

Two backends behind one interface so the rest of the app does not care which is
used:
  - HaarDetector: OpenCV Haar cascade. Fast, zero extra files, less robust.
  - DnnDetector:  OpenCV DNN (res10 SSD). More robust to angle/lighting; needs
                  the model files (see README) at runtime.

Each detector returns the largest face as (cx, cy, w, h) in pixel coordinates,
or None.
"""
import os
import cv2


def _largest(boxes):
    """boxes: list of (x, y, w, h) -> (cx, cy, w, h) of the biggest, or None."""
    if not boxes:
        return None
    x, y, w, h = max(boxes, key=lambda b: b[2] * b[3])
    return (x + w / 2.0, y + h / 2.0, float(w), float(h))


class HaarDetector:
    def __init__(self, min_size=(60, 60), scale_factor=1.2, min_neighbors=5):
        path = cv2.data.haarcascades + "haarcascade_frontalface_default.xml"
        self.cascade = cv2.CascadeClassifier(path)
        if self.cascade.empty():
            raise RuntimeError(f"Failed to load Haar cascade: {path}")
        self.min_size = min_size
        self.scale_factor = scale_factor
        self.min_neighbors = min_neighbors

    def detect(self, frame):
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        boxes = self.cascade.detectMultiScale(
            gray, self.scale_factor, self.min_neighbors, minSize=self.min_size
        )
        return _largest([tuple(b) for b in boxes])


class DnnDetector:
    """OpenCV DNN SSD face detector (res10).

    Requires two files next to this module (or via explicit paths):
      - deploy.prototxt
      - res10_300x300_ssd_iter_140000.caffemodel
    Download links are in the app README.
    """

    def __init__(self, proto=None, model=None, conf_threshold=0.5):
        here = os.path.dirname(__file__)
        proto = proto or os.path.join(here, "models", "deploy.prototxt")
        model = model or os.path.join(
            here, "models", "res10_300x300_ssd_iter_140000.caffemodel"
        )
        if not (os.path.isfile(proto) and os.path.isfile(model)):
            raise FileNotFoundError(
                "DNN model files not found; see README to download them, "
                "or use the Haar detector."
            )
        self.net = cv2.dnn.readNetFromCaffe(proto, model)
        self.conf_threshold = conf_threshold

    def detect(self, frame):
        h, w = frame.shape[:2]
        blob = cv2.dnn.blobFromImage(
            cv2.resize(frame, (300, 300)), 1.0, (300, 300),
            (104.0, 177.0, 123.0)
        )
        self.net.setInput(blob)
        det = self.net.forward()
        boxes = []
        for i in range(det.shape[2]):
            conf = float(det[0, 0, i, 2])
            if conf < self.conf_threshold:
                continue
            x1 = int(det[0, 0, i, 3] * w)
            y1 = int(det[0, 0, i, 4] * h)
            x2 = int(det[0, 0, i, 5] * w)
            y2 = int(det[0, 0, i, 6] * h)
            boxes.append((x1, y1, x2 - x1, y2 - y1))
        return _largest(boxes)


def make_detector(kind: str = "haar"):
    kind = (kind or "haar").lower()
    if kind == "dnn":
        return DnnDetector()
    if kind == "haar":
        return HaarDetector()
    raise ValueError(f"unknown detector kind: {kind!r} (use 'haar' or 'dnn')")
