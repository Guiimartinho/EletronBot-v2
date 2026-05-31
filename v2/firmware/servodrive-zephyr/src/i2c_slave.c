/*
 * I2C target (slave) command handler.
 *
 * Uses Zephyr's I2C target API. The write handler decodes [opcode][float32]
 * and updates the shared servo state; the read handler returns the current
 * angle (float32, little-endian), echoing the original behavior.
 *
 * NOTE: not all STM32 I2C target drivers expose the full callback set; the
 * decode logic here is the portable part. The driver registration is the
 * integration point (marked TODO) depending on the chosen SoC.
 */
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>
#include <string.h>

#include "i2c_slave.h"
#include "pid.h"

LOG_MODULE_REGISTER(eb_i2c, LOG_LEVEL_INF);

extern struct eb_dce g_dce;  /* gain updates land here */

static struct eb_servo_state *s_state;
static uint8_t s_rx[5];
static int s_rx_len;

static float le_float(const uint8_t *p)
{
	uint32_t v = (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
		     ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
	float f;

	memcpy(&f, &v, sizeof(f));
	return f;
}

/* Apply one complete 5-byte command [opcode][float32]. Exposed for testing. */
void eb_i2c_apply_command(const uint8_t *cmd5)
{
	uint8_t op = cmd5[0];
	float arg = le_float(&cmd5[1]);

	switch (op) {
	case EB_OP_SET_ANGLE:   s_state->target_angle = arg; break;
	case EB_OP_SET_TORQUE:  g_dce.output_limit = arg; break;
	case EB_OP_SET_KP:      g_dce.kp = arg; break;
	case EB_OP_SET_KI:      g_dce.ki = arg; break;
	case EB_OP_SET_KV:      g_dce.kv = arg; break;
	case EB_OP_SET_KD:      g_dce.kd = arg; break;
	case EB_OP_SET_TLIMIT:  g_dce.output_limit = arg; break;
	case EB_OP_SET_ID:      s_state->node_id = (uint8_t)arg; break;
	case EB_OP_SET_INITPOS: s_state->target_angle = arg; break;
	case EB_OP_ENABLE:      s_state->enabled = (cmd5[1] != 0); break;
	default:
		LOG_WARN("unknown opcode 0x%02x", op);
		break;
	}
	/* TODO: persist ID/gains/limits to flash (settings subsystem) for
	 * EB_OP_SET_ID / SET_K* / SET_TLIMIT / SET_INITPOS, like the original
	 * emulated-EEPROM behavior. */
}

int eb_i2c_slave_init(struct eb_servo_state *state)
{
	s_state = state;
	s_rx_len = 0;

	/*
	 * TODO (driver integration): register an I2C target on the bus at
	 * address (state->node_id) using the SoC's I2C target driver, and route:
	 *   - write-received bytes -> accumulate into s_rx; on 5 bytes call
	 *     eb_i2c_apply_command(s_rx).
	 *   - read-requested -> return current_angle as 4 little-endian bytes.
	 *
	 * The command decode (eb_i2c_apply_command) is complete and testable.
	 */
	(void)s_rx;
	LOG_INF("i2c slave decode ready (driver registration is the TODO)");
	return 0;
}
