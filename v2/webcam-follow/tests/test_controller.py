"""Unit tests for the yaw controller (pure logic, no hardware).

Run from the webcam-follow folder:
    python -m pytest tests/ -v
"""
import os
import sys

sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))

from config import Config          # noqa: E402
from controller import YawController  # noqa: E402

WIDTH = 640


def fresh():
    return YawController(Config())


def test_starts_centered():
    assert fresh().yaw == 0.0


def test_centered_face_keeps_yaw_near_zero():
    ctl = fresh()
    for _ in range(20):
        ctl.update(WIDTH / 2, WIDTH)   # face dead-center -> deadzone
    assert abs(ctl.yaw) < 1e-6


def test_face_right_moves_yaw_consistently():
    ctl = fresh()
    first = ctl.update(WIDTH * 0.9, WIDTH)   # face to the right
    second = ctl.update(WIDTH * 0.9, WIDTH)
    # yaw should move and keep moving the same direction while error persists
    assert first != 0.0
    assert abs(second) > abs(first)
    assert (first > 0) == (second > 0)


def test_sign_inverts_direction():
    cfg_pos = Config(); cfg_pos.yaw_sign = +1
    cfg_neg = Config(); cfg_neg.yaw_sign = -1
    a = YawController(cfg_pos).update(WIDTH * 0.9, WIDTH)
    b = YawController(cfg_neg).update(WIDTH * 0.9, WIDTH)
    assert (a > 0) != (b > 0)


def test_rate_limit_respected():
    cfg = Config()
    ctl = YawController(cfg)
    prev = ctl.yaw
    ctl.update(WIDTH, WIDTH)   # maximum error
    assert abs(ctl.yaw - prev) <= cfg.max_rate + 1e-9


def test_yaw_is_clamped_to_limit():
    cfg = Config()
    ctl = YawController(cfg)
    for _ in range(1000):       # push hard to one side for a long time
        ctl.update(WIDTH, WIDTH)
    assert ctl.yaw <= cfg.yaw_limit + 1e-9
    assert ctl.yaw >= -cfg.yaw_limit - 1e-9
    assert abs(ctl.yaw - cfg.yaw_limit) < 1e-3   # should saturate near the limit


def test_deadzone_ignores_small_error():
    cfg = Config()
    ctl = YawController(cfg)
    # error just inside the deadzone -> no movement
    cx = WIDTH / 2 + (cfg.deadzone * 0.5) * (WIDTH / 2)
    ctl.update(cx, WIDTH)
    assert abs(ctl.yaw) < 1e-6


def test_no_face_recenters_toward_zero():
    cfg = Config()
    ctl = YawController(cfg)
    ctl.yaw = 50.0
    ctl.update(None, WIDTH)
    assert 0.0 <= ctl.yaw < 50.0   # drifted toward center, not past it


def test_no_face_from_zero_stays_zero():
    ctl = fresh()
    ctl.update(None, WIDTH)
    assert ctl.yaw == 0.0
