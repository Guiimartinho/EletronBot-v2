# SPDX-License-Identifier: Apache-2.0

# Flash/debug via ST-Link + OpenOCD (matches the original stlink.cfg workflow).
board_runner_args(openocd "--config=${BOARD_DIR}/support/openocd.cfg")
board_runner_args(stm32cubeprogrammer "--port=swd" "--reset-mode=hw")

include(${ZEPHYR_BASE}/boards/common/openocd.board.cmake)
include(${ZEPHYR_BASE}/boards/common/stm32cubeprogrammer.board.cmake)
