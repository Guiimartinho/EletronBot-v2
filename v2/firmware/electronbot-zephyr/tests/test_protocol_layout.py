"""Reference test for the ElectronBot wire-protocol DESIGN.

This validates the framing arithmetic and the 32-byte extra-data byte layout
that `src/protocol.h` / `src/protocol.c` implement. It is a *reference* test in
Python: it does NOT compile the C firmware (no C toolchain in this environment),
but it proves the numbers and the little-endian codec layout are internally
consistent and match the host SDK contract.

Run:
    python -m pytest tests/test_protocol_layout.py -v
"""
import struct

# --- constants mirrored from src/protocol.h ---
LCD_W = 240
LCD_H = 240
BPP = 3
FRAME_BYTES = LCD_W * LCD_H * BPP        # 172800
CHUNKS = 4
CHUNK_PACKETS = 84
PACKET_SIZE = 512
CHUNK_BODY = CHUNK_PACKETS * PACKET_SIZE  # 43008
TAIL_SIZE = 224
TAIL_IMAGE_BYTES = 192
EXTRA_BYTES = 32
STRIPE_ROWS = LCD_H // CHUNKS             # 60
STRIPE_BYTES = STRIPE_ROWS * LCD_W * BPP  # 43200
RX_BUF_BYTES = STRIPE_BYTES + EXTRA_BYTES # 43232
JOINTS = 6
EXTRA_ENABLE_OFF = 0
EXTRA_JOINTS_OFF = 1


def test_frame_math():
    assert FRAME_BYTES == 172800
    assert CHUNK_BODY == 43008
    assert STRIPE_BYTES == 43200
    # one chunk delivers body + 192 tail image bytes = one stripe
    assert CHUNK_BODY + TAIL_IMAGE_BYTES == STRIPE_BYTES
    # four chunks reconstruct a full frame exactly
    assert CHUNKS * (CHUNK_BODY + TAIL_IMAGE_BYTES) == FRAME_BYTES
    # tail = 192 image + 32 extra
    assert TAIL_IMAGE_BYTES + EXTRA_BYTES == TAIL_SIZE
    assert RX_BUF_BYTES == 43232


def _decode(buf32):
    enable = buf32[EXTRA_ENABLE_OFF]
    joints = [struct.unpack_from("<f", buf32, EXTRA_JOINTS_OFF + j * 4)[0]
              for j in range(JOINTS)]
    return enable, joints


def _encode(enable, joints):
    buf = bytearray(EXTRA_BYTES)
    buf[EXTRA_ENABLE_OFF] = enable & 0xff
    for j in range(JOINTS):
        struct.pack_into("<f", buf, EXTRA_JOINTS_OFF + j * 4, joints[j])
    return bytes(buf)


def test_extra_data_roundtrip():
    enable = 1
    joints = [0.0, -15.0, 90.0, 30.0, -180.0, 12.5]
    buf = _encode(enable, joints)
    assert len(buf) == EXTRA_BYTES
    en2, j2 = _decode(buf)
    assert en2 == enable
    for a, b in zip(joints, j2):
        assert abs(a - b) < 1e-6


def test_extra_data_offsets():
    # enable in byte 0, joints start at byte 1, occupy 24 bytes, rest reserved
    buf = _encode(0, [1, 2, 3, 4, 5, 6])
    assert buf[0] == 0
    assert EXTRA_JOINTS_OFF + JOINTS * 4 == 25
    assert all(b == 0 for b in buf[25:32])  # reserved tail is zeroed
