/*
 * Extra-data (servo side-channel) encode/decode.
 *
 * Layout (32 bytes):
 *   [0]      enable flag
 *   [1..24]  6 little-endian float32 joint values
 *   [25..31] reserved
 *
 * Uses byte-wise copy so it is endian-safe and alignment-safe regardless of the
 * target (Cortex-M4 is little-endian, but this stays explicit on purpose).
 */
#include <string.h>
#include "protocol.h"

static float load_le_float(const uint8_t *p)
{
	uint32_t v = (uint32_t)p[0] |
		     ((uint32_t)p[1] << 8) |
		     ((uint32_t)p[2] << 16) |
		     ((uint32_t)p[3] << 24);
	float f;

	memcpy(&f, &v, sizeof(f));
	return f;
}

static void store_le_float(uint8_t *p, float f)
{
	uint32_t v;

	memcpy(&v, &f, sizeof(v));
	p[0] = (uint8_t)(v & 0xff);
	p[1] = (uint8_t)((v >> 8) & 0xff);
	p[2] = (uint8_t)((v >> 16) & 0xff);
	p[3] = (uint8_t)((v >> 24) & 0xff);
}

void eb_extra_decode(const uint8_t *buf32, struct eb_extra_data *out)
{
	out->enable = buf32[EB_EXTRA_ENABLE_OFF];
	for (int j = 0; j < EB_JOINT_COUNT; j++) {
		out->joints[j] = load_le_float(buf32 + EB_EXTRA_JOINTS_OFF + j * 4);
	}
}

void eb_extra_encode(const struct eb_extra_data *in, uint8_t *buf32)
{
	memset(buf32, 0, EB_EXTRA_BYTES);
	buf32[EB_EXTRA_ENABLE_OFF] = in->enable;
	for (int j = 0; j < EB_JOINT_COUNT; j++) {
		store_le_float(buf32 + EB_EXTRA_JOINTS_OFF + j * 4, in->joints[j]);
	}
}
