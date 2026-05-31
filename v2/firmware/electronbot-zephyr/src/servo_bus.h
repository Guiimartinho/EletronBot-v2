/*
 * Servo bus - I2C master to the six ServoDrive joint boards.
 *
 * The head MCU is the I2C master; each joint is a slave at an even address
 * (2,4,6,8,10,12). A command is a 5-byte write (opcode + float32), with an
 * optional 5-byte read for telemetry.
 */
#ifndef ELECTRONBOT_SERVO_BUS_H_
#define ELECTRONBOT_SERVO_BUS_H_

#include <stdint.h>
#include "protocol.h"

/* I2C opcodes (see docs/recommendations + robot9.png). */
#define EB_SERVO_SET_ANGLE     0x01
#define EB_SERVO_SET_VEL       0x02
#define EB_SERVO_SET_TORQUE    0x03
#define EB_SERVO_GET_ANGLE     0x11
#define EB_SERVO_GET_VEL       0x12
#define EB_SERVO_ENABLE        0xff

/* 7-bit slave addresses per joint index (j1..j6). */
extern const uint8_t eb_servo_addr[EB_JOINT_COUNT];

int  eb_servo_init(void);
int  eb_servo_set_angle(int joint_index, float angle_deg);
int  eb_servo_get_angle(int joint_index, float *angle_deg_out);
void eb_servo_enable_all(int enable);

/* Push all six set-points (used per frame). `enable` mirrors the host flag. */
void eb_servo_update_all(const float joints[EB_JOINT_COUNT], int enable);

#endif /* ELECTRONBOT_SERVO_BUS_H_ */
