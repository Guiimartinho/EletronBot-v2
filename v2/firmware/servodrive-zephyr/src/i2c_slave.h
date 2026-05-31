/*
 * I2C slave - command interface from the head MCU.
 *
 * Implements the same opcode set as the original ServoDrive (see the table in
 * the project recommendations / robot9.png). The head writes [opcode][float32]
 * to set targets/gains and reads back the current angle.
 */
#ifndef SERVODRIVE_I2C_SLAVE_H_
#define SERVODRIVE_I2C_SLAVE_H_

#include <stdint.h>

/* Opcodes. */
#define EB_OP_SET_ANGLE    0x01
#define EB_OP_SET_VEL      0x02
#define EB_OP_SET_TORQUE   0x03
#define EB_OP_GET_ANGLE    0x11
#define EB_OP_GET_VEL      0x12
#define EB_OP_SET_ID       0x21
#define EB_OP_SET_KP       0x22
#define EB_OP_SET_KI       0x23
#define EB_OP_SET_KV       0x24
#define EB_OP_SET_KD       0x25
#define EB_OP_SET_TLIMIT   0x26
#define EB_OP_SET_INITPOS  0x27
#define EB_OP_ENABLE       0xff

/* Shared command state updated by the I2C ISR and read by the control loop. */
struct eb_servo_state {
	volatile float target_angle;
	volatile float current_angle;
	volatile float current_velocity;
	volatile int   enabled;
	volatile uint8_t node_id;
};

int eb_i2c_slave_init(struct eb_servo_state *state);

#endif /* SERVODRIVE_I2C_SLAVE_H_ */
