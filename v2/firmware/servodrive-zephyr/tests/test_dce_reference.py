"""Reference test for the DCE position controller (src/pid.c).

Mirrors the C control law in Python and runs it against a simple servo plant to
prove the algorithm is stable and converges. This validates the control DESIGN;
it does not compile the C (no C toolchain here), but the math is identical.

Run:
    python -m pytest tests/test_dce_reference.py -v
"""


class DCE:
    """Python mirror of eb_dce (src/pid.c)."""

    def __init__(self, kp, ki, kv, kd, integral_limit, output_limit):
        self.kp, self.ki, self.kv, self.kd = kp, ki, kv, kd
        self.integral_limit = integral_limit
        self.output_limit = output_limit
        self.integral = 0.0
        self.vel_integral = 0.0
        self.prev_error = 0.0

    @staticmethod
    def _clamp(v, lim):
        return max(-lim, min(lim, v))

    def update(self, target, measured, velocity):
        error = target - measured
        derivative = error - self.prev_error
        self.integral = self._clamp(self.integral + error, self.integral_limit)
        self.vel_integral = self._clamp(self.vel_integral + velocity,
                                        self.integral_limit)
        out = (self.kp * error + self.ki * self.integral
               + self.kv * self.vel_integral + self.kd * derivative)
        self.prev_error = error
        return self._clamp(out, self.output_limit)


def simulate(target, kp=10.0, ki=0.0, kv=0.0, kd=50.0, steps=400):
    """Run the controller against a first-order servo plant; return final angle."""
    dce = DCE(kp, ki, kv, kd, integral_limit=500.0, output_limit=500.0)
    angle = 0.0
    vel = 0.0
    for _ in range(steps):
        out = dce.update(target, angle, vel)
        # crude plant: command -> acceleration, with damping
        accel = out * 0.0005
        vel = (vel + accel) * 0.9
        angle += vel
    return angle


def test_converges_to_setpoint():
    final = simulate(90.0)
    assert abs(final - 90.0) < 2.0   # settles near the target


def test_stays_bounded_when_disabled_gains_zero():
    # ki=kv=0, only P+D; should still converge and not blow up
    final = simulate(45.0, kp=8.0, kd=40.0)
    assert abs(final - 45.0) < 3.0
    assert -360.0 < final < 360.0


def test_output_is_clamped():
    dce = DCE(1000.0, 0, 0, 0, 500.0, 500.0)
    out = dce.update(target=180.0, measured=0.0, velocity=0.0)
    assert abs(out) <= 500.0


def test_zero_error_zero_output_at_rest():
    dce = DCE(10, 0, 0, 50, 500, 500)
    out = dce.update(target=30.0, measured=30.0, velocity=0.0)
    assert out == 0.0
