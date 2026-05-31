# ServoDrive v2 - RTOS Feasibility & MCU Decision

Phase 3 asks whether the per-joint servo firmware can move to an RTOS (Zephyr)
like the head firmware. This document records the engineering decision.

## The constraint

The current ServoDrive board uses an **STM32F042F6P6**: Cortex-M0, ~32 KB flash,
**6 KB RAM**. The original project already shipped a hand-optimized **bare-metal
LL** variant (`ServoDrive-fw-ll`) specifically to fit a tiny MCU.

Zephyr's practical minimum RAM footprint for a useful build (kernel + a couple
of threads + I2C + ADC + PWM drivers) is on the order of **8-16 KB RAM**, beyond
what 6 KB allows comfortably. **Conclusion: Zephyr does not fit the F042.**

## Options considered

| Option | Pros | Cons |
|---|---|---|
| **A. Keep bare-metal LL** (status quo) | Cheapest BOM, proven, fits 6 KB | No RTOS uniformity with the head |
| **B. Respin to a larger MCU + Zephyr** | RTOS everywhere, modern drivers | Board respin, higher BOM, more power |
| C. Different RTOS (FreeRTOS/Zephyr-tiny) | Smaller than full Zephyr | Still tight on 6 KB; little gain |

## Decision

- **Production / existing hardware: keep Option A** (the bare-metal LL build).
  It is the right tool for a 6 KB joint board; an RTOS adds cost and power for no
  functional benefit at the joint level.
- **For RTOS uniformity (optional, future hardware): Option B.** If a ServoDrive
  respin happens, target an RTOS-capable MCU with >=32 KB RAM. Suggested class:
  **STM32G431** (Cortex-M4F, 32 KB RAM, 170 MHz, good timer/ADC/I2C set) - a
  clean Zephyr target and a comfortable upgrade from the M0.

## What ships in this phase

A **Zephyr ServoDrive reference firmware** (`../../firmware/servodrive-zephyr/`)
targeting an RTOS-capable dev board (default `nucleo_g431rb`). It is:

- a working blueprint for Option B, and
- a place to develop/validate the control law (the DCE/PID) with a real RTOS,

without pretending it runs on the existing 6 KB board. The control algorithm is
identical to the original so it can later be folded back into the LL build if
desired. The DCE math is validated by a host reference test.
