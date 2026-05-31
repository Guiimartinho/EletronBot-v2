"""Horizontal-axis controller: face error -> body-yaw (joint j6).

Velocity-style (incremental) control: the waist turns at a rate proportional
to the horizontal error until the face is centered. Robust without an exact
pixel-to-degree mapping.
"""


class YawController:
    def __init__(self, cfg):
        self.cfg = cfg
        self.yaw = 0.0        # current yaw command (deg)
        self.err_ema = 0.0    # smoothed error

    def update(self, face_cx, frame_w: int) -> float:
        if face_cx is None:
            # No face: drift slowly back to center, do not stick in a corner.
            self.yaw += (0.0 - self.yaw) * self.cfg.recenter_gain
            return self._clamp(self.yaw)

        err = (face_cx - frame_w / 2.0) / (frame_w / 2.0)   # normalized [-1, 1]
        if abs(err) < self.cfg.deadzone:
            err = 0.0
        self.err_ema = self.cfg.ema * err + (1.0 - self.cfg.ema) * self.err_ema

        step = self.cfg.kp * self.err_ema * self.cfg.yaw_sign   # deg per tick
        step = max(-self.cfg.max_rate, min(self.cfg.max_rate, step))  # rate limit
        self.yaw = self._clamp(self.yaw + step)
        return self.yaw

    def _clamp(self, v: float) -> float:
        lim = self.cfg.yaw_limit
        return max(-lim, min(lim, v))
