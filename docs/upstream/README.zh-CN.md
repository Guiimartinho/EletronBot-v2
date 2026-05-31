<h1 align="center">ElectronBot v2</h1>

<div align="center">

<i>A downstream continuation of <a href="https://github.com/peng-zhihui/ElectronBot">peng-zhihui/ElectronBot</a> — a WALL-E/EVE-style 6-DOF mini desktop robot with a round LCD face, an onboard camera, and USB control.</i>

</div>

---

## About this fork

This repository keeps the **full original history** of the upstream ElectronBot project and builds new work on top of it. The goal of v2 is to extend the robot with new capabilities (starting with a face-tracking webcam mode) and to modernize the firmware/host software.

> **All new work in this repository — code, documentation, comments, and file names — is written in English.**

The robot is intentionally "thin": all rendering and motion planning happen on a host PC, which streams a 240×240 RGB face image plus 6 joint set-points to the robot over a single USB cable. See the architecture analysis below for the full data/control flow.

## Repository layout

| Path | Contents |
|---|---|
| `1.Hardware/` | Altium PCB projects (head board, ServoDrive, SensorBoard, BaseConnector) |
| `2.Firmware/` | STM32 firmware (ElectronBot-fw on F405, ServoDrive-fw on F042) |
| `3.Software/` | PC host stack: C++ SDK, Unity Studio, tools (driver, calibration) |
| `4.CAD-Model/` | STEP models and emoji video assets |
| `5.Docs/` | Datasheets, images, large build dependencies |
| `6.Tests/` | `TestDisplayUSB` end-to-end bring-up kit |
| `docs/` | **v2 documentation (this fork's analysis, roadmap, and plans)** |
| `docs/upstream/` | Original author READMEs (Chinese + English), preserved |

## v2 documentation

| Document | Description |
|---|---|
| [Architecture analysis](docs/architecture-analysis.md) | In-depth analysis of the whole project (hardware, firmware, host, protocol) |
| [Roadmap](docs/roadmap.md) | v2 roadmap: webcam-follow, possible Zephyr port, phased plan + risks |
| [Phase 1 — Face-tracking webcam](docs/phase-1-webcam-follow.md) | Implementation plan for the face-tracking virtual webcam |
| [Recommendations for upstream](docs/recommendations-upstream.md) | Constructive feedback for the original maintainers |

PDF renders of the analysis, roadmap, and phase-1 plan live alongside their Markdown sources in `docs/`.

## Credits & license

Original project and hardware/firmware/software design by **[peng-zhihui](https://github.com/peng-zhihui)**. The original author READMEs are preserved under [`docs/upstream/`](docs/upstream/). This fork is distributed under the same license — see [LICENSE](LICENSE).
