# Phase 2 (Zephyr head firmware) - Status

| Item | Status | Notes |
|---|---|---|
| Wire-protocol spec (`protocol.h`) | Complete | Byte-compatible with host SDK |
| Protocol layout + framing math | **Verified (Python reference)** | `tests/test_protocol_layout.py` - 4 tests pass |
| Extra-data codec (`protocol.c`) | Code-complete | C not compiled here; layout validated by the reference test |
| USB framing state machine (`usb_app.c`) | Complete | Endpoint glue is the integration TODO |
| GC9A01 driver (`display.c`) | Code-complete | Sync transfers; DMA overlap = TODO |
| I2C servo master (`servo_bus.c`) | Code-complete | Matches original opcodes/addresses |
| Orchestration (`robot.c`) | Code-complete | Joint mapping/limits + 2s boot guard |
| Board definition (F405) | Draft | **Pins must be confirmed vs schematic** |
| Compiles with west/Zephyr SDK | **Not done here** | No Zephyr SDK in this environment |
| Flashed / runs on hardware | **Pending hardware** | Acceptance = Sample.exe works unchanged |

**Verified without hardware:** the protocol byte-layout and framing arithmetic
(`4*(43008+192)=172800`, stripe=43200, rx-buf=43232, 32-byte extra block with
enable + 6 little-endian floats) via an equivalent Python reference test
(`tests/test_protocol_layout.py`, 4 tests pass). This validates the design; the
C is not compiled in this environment.

**Not verified:** compilation (needs Zephyr SDK + west), USB endpoint behavior,
display timing, and I2C on the real bus. These require the toolchain and the
physical board.
