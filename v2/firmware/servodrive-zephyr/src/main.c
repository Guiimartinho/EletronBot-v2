/*
 * ServoDrive (Zephyr reference firmware) - one joint controller.
 *
 * Reference/blueprint for an RTOS-capable ServoDrive (see ../../hardware/
 * servodrive-v2/DESIGN.md). Default target: nucleo_g431rb (the F042 cannot host
 * Zephyr). The control law matches the original DCE position controller.
 *
 * A 200 Hz control thread:
 *   read pot (ADC) -> angle -> DCE update -> drive H-bridge.
 * An I2C target accepts angle/gain commands from the head MCU.
 */
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "pid.h"
#include "motor.h"
#include "encoder.h"
#include "i2c_slave.h"

LOG_MODULE_REGISTER(eb_servo_main, LOG_LEVEL_INF);

#define CONTROL_HZ      200
#define CONTROL_PERIOD  K_MSEC(1000 / CONTROL_HZ)

/* Defaults mirror the original firmware. */
#define DEF_KP          10.0f
#define DEF_KI          0.0f
#define DEF_KV          0.0f
#define DEF_KD          50.0f
#define DEF_INT_LIMIT   500.0f
#define DEF_OUT_LIMIT   500.0f   /* maps to torque/duty headroom */
#define DEF_NODE_ID     12

struct eb_dce g_dce;             /* referenced by i2c_slave.c for gain updates */
static struct eb_servo_state g_state;

static const struct eb_pot_cal g_cal = {
	.raw_min = 250, .raw_max = 3000, .angle_min = 0.0f, .angle_max = 180.0f,
};

static void control_thread(void)
{
	float prev_angle = 0.0f;

	while (1) {
		int raw = eb_encoder_read_raw();

		if (raw >= 0) {
			float angle = eb_encoder_raw_to_angle(&g_cal, raw);
			float vel = angle - prev_angle;

			g_state.current_angle = angle;
			g_state.current_velocity = vel;
			prev_angle = angle;

			if (g_state.enabled) {
				float cmd = eb_dce_update(&g_dce,
							  g_state.target_angle,
							  angle, vel);
				eb_motor_drive(cmd);
			} else {
				eb_motor_stop();
			}
		}
		k_sleep(CONTROL_PERIOD);
	}
}

K_THREAD_DEFINE(eb_ctrl_tid, 1024, control_thread, NULL, NULL, NULL,
		K_PRIO_COOP(2), 0, 0);

int main(void)
{
	LOG_INF("servodrive-zephyr boot");

	eb_dce_init(&g_dce, DEF_KP, DEF_KI, DEF_KV, DEF_KD,
		    DEF_INT_LIMIT, DEF_OUT_LIMIT);
	g_state.node_id = DEF_NODE_ID;
	g_state.target_angle = 90.0f;
	g_state.enabled = 0;

	if (eb_motor_init() || eb_encoder_init() ||
	    eb_i2c_slave_init(&g_state)) {
		LOG_ERR("peripheral init failed");
		return -EIO;
	}

	/* control_thread runs via K_THREAD_DEFINE */
	return 0;
}
