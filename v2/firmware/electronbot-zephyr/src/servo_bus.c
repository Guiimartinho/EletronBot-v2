/*
 * I2C master to the ServoDrive joint boards (Zephyr i2c API).
 *
 * Per joint, a "set angle" is a 5-byte write: [opcode][float32 LE].
 * A "get angle" writes the opcode then reads 4 bytes (float32 LE).
 */
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>
#include <string.h>

#include "servo_bus.h"

LOG_MODULE_REGISTER(eb_servo, LOG_LEVEL_INF);

#define I2C_NODE DT_NODELABEL(eb_servo_i2c)

static const struct device *i2c_dev;

/* j1=head, j2=L-arm-open, j3=R-arm-lift, j4=R-arm-open, j5=L-arm-lift, j6=body */
const uint8_t eb_servo_addr[EB_JOINT_COUNT] = {
	0x02 >> 1, 0x04 >> 1, 0x06 >> 1, 0x08 >> 1, 0x0a >> 1, 0x0c >> 1
};

static void pack_le_float(uint8_t *p, float f)
{
	uint32_t v;

	memcpy(&v, &f, sizeof(v));
	p[0] = v & 0xff; p[1] = (v >> 8) & 0xff;
	p[2] = (v >> 16) & 0xff; p[3] = (v >> 24) & 0xff;
}

static float unpack_le_float(const uint8_t *p)
{
	uint32_t v = (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
		     ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
	float f;

	memcpy(&f, &v, sizeof(f));
	return f;
}

int eb_servo_init(void)
{
	i2c_dev = DEVICE_DT_GET(I2C_NODE);
	if (!device_is_ready(i2c_dev)) {
		LOG_ERR("I2C not ready");
		return -ENODEV;
	}
	/* Standard mode 100kHz to match the original bus. */
	i2c_configure(i2c_dev, I2C_SPEED_SET(I2C_SPEED_STANDARD) | I2C_MODE_CONTROLLER);
	return 0;
}

int eb_servo_set_angle(int j, float angle_deg)
{
	uint8_t buf[5];

	if (j < 0 || j >= EB_JOINT_COUNT) {
		return -EINVAL;
	}
	buf[0] = EB_SERVO_SET_ANGLE;
	pack_le_float(&buf[1], angle_deg);
	return i2c_write(i2c_dev, buf, sizeof(buf), eb_servo_addr[j]);
}

int eb_servo_get_angle(int j, float *out)
{
	uint8_t op = EB_SERVO_GET_ANGLE;
	uint8_t rx[4];
	int rc;

	if (j < 0 || j >= EB_JOINT_COUNT) {
		return -EINVAL;
	}
	rc = i2c_write(i2c_dev, &op, 1, eb_servo_addr[j]);
	if (rc) {
		return rc;
	}
	rc = i2c_read(i2c_dev, rx, sizeof(rx), eb_servo_addr[j]);
	if (rc) {
		return rc;
	}
	*out = unpack_le_float(rx);
	return 0;
}

void eb_servo_enable_all(int enable)
{
	uint8_t buf[5] = { EB_SERVO_ENABLE, 0, 0, 0, 0 };

	buf[1] = enable ? 1 : 0;
	for (int j = 0; j < EB_JOINT_COUNT; j++) {
		(void)i2c_write(i2c_dev, buf, sizeof(buf), eb_servo_addr[j]);
	}
}

void eb_servo_update_all(const float joints[EB_JOINT_COUNT], int enable)
{
	if (!enable) {
		return;
	}
	for (int j = 0; j < EB_JOINT_COUNT; j++) {
		(void)eb_servo_set_angle(j, joints[j]);
	}
}
