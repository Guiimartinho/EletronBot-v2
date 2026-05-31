/*
 * Robot orchestration.
 */
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <string.h>

#include "robot.h"
#include "usb_app.h"
#include "display.h"
#include "servo_bus.h"

LOG_MODULE_REGISTER(eb_robot, LOG_LEVEL_INF);

/* j1=head, j2=L-open, j3=R-lift, j4=R-open, j5=L-lift, j6=body */
const float eb_joint_limit[EB_JOINT_COUNT] = {15.f, 30.f, 180.f, 30.f, 180.f, 90.f};
const int   eb_joint_invert[EB_JOINT_COUNT] = {0, 0, 0, 1, 1, 0};

/* Latest measured angles, returned to the host on each IN handshake. */
static float s_measured[EB_JOINT_COUNT];
static struct eb_extra_data s_last_cmd;

float eb_robot_map_angle(int j, float model_angle)
{
	float lim = eb_joint_limit[j];
	float a = model_angle;

	if (a > lim) {
		a = lim;
	}
	if (a < -lim) {
		a = -lim;
	}
	if (eb_joint_invert[j]) {
		a = -a;
	}
	return a;
}

/* Called by the USB transport for each completed stripe (chunk). */
static void on_chunk(int chunk_index, const uint8_t *stripe_rgb,
		     const struct eb_extra_data *extra)
{
	if (chunk_index == 0) {
		eb_display_begin_frame();
	}

	eb_display_wait(); /* ensure previous DMA done before reusing the bus */
	eb_display_push_stripe(stripe_rgb, EB_STRIPE_BYTES, chunk_index == 0);

	/* The extra-data block is identical across the 4 chunks of a frame; act
	 * on the last one to push servo set-points once per frame. */
	if (chunk_index == EB_CHUNKS - 1) {
		s_last_cmd = *extra;
		float mapped[EB_JOINT_COUNT];

		for (int j = 0; j < EB_JOINT_COUNT; j++) {
			mapped[j] = eb_robot_map_angle(j, extra->joints[j]);
		}
		eb_servo_update_all(mapped, extra->enable);

		/* refresh telemetry (best-effort, non-blocking cadence) */
		for (int j = 0; j < EB_JOINT_COUNT; j++) {
			(void)eb_servo_get_angle(j, &s_measured[j]);
		}
	}
}

/* Called when the host reads the IN endpoint: return measured joints. */
static void fill_telemetry(uint8_t out_buf32[EB_EXTRA_BYTES])
{
	struct eb_extra_data tel;

	tel.enable = s_last_cmd.enable;
	memcpy(tel.joints, s_measured, sizeof(s_measured));
	eb_extra_encode(&tel, out_buf32);
}

int eb_robot_init(void)
{
	int rc;

	rc = eb_display_init();
	if (rc) {
		return rc;
	}
	rc = eb_servo_init();
	if (rc) {
		return rc;
	}

	/* Give the six servo MCUs time to boot before any I2C traffic, exactly
	 * like the original HAL_Delay(2000) guard against bus lockup. */
	k_msleep(2000);
	eb_servo_enable_all(0);

	return eb_usb_init(on_chunk, fill_telemetry);
}

void eb_robot_run(void)
{
	LOG_INF("ElectronBot (Zephyr) running");
	while (1) {
		/* The pipeline is interrupt/endpoint-driven; idle here. */
		k_msleep(100);
	}
}
