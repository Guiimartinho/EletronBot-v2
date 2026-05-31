# SPDX-License-Identifier: Apache-2.0
cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(electronbot_zephyr C)

target_sources(app PRIVATE
  src/main.c
  src/robot.c
  src/protocol.c
  src/usb_app.c
  src/display.c
  src/servo_bus.c
)
