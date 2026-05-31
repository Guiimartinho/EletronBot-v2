/*
 * Sign-magnitude H-bridge motor driver (Zephyr PWM).
 */
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/logging/log.h>

#include "motor.h"

LOG_MODULE_REGISTER(eb_motor, LOG_LEVEL_INF);

/* Two PWM channels for the H-bridge (devicetree aliases). */
static const struct pwm_dt_spec pwm_a = PWM_DT_SPEC_GET(DT_ALIAS(motor_a));
static const struct pwm_dt_spec pwm_b = PWM_DT_SPEC_GET(DT_ALIAS(motor_b));

/* ~48 kHz like the original (period in nanoseconds). */
#define PWM_PERIOD_NS 20833U

static void set_duty(const struct pwm_dt_spec *ch, uint32_t duty_0_1000)
{
	uint32_t pulse = (uint64_t)PWM_PERIOD_NS * duty_0_1000 / EB_MOTOR_DUTY_MAX;

	(void)pwm_set_dt(ch, PWM_PERIOD_NS, pulse);
}

int eb_motor_init(void)
{
	if (!pwm_is_ready_dt(&pwm_a) || !pwm_is_ready_dt(&pwm_b)) {
		LOG_ERR("PWM channels not ready");
		return -ENODEV;
	}
	eb_motor_stop();
	return 0;
}

void eb_motor_drive(float command)
{
	float mag = command < 0 ? -command : command;
	uint32_t duty = (uint32_t)(mag);

	if (duty > EB_MOTOR_DUTY_MAX) {
		duty = EB_MOTOR_DUTY_MAX;
	}

	if (command >= 0) {
		set_duty(&pwm_a, duty);
		set_duty(&pwm_b, 0);
	} else {
		set_duty(&pwm_a, 0);
		set_duty(&pwm_b, duty);
	}
}

void eb_motor_stop(void)
{
	set_duty(&pwm_a, 0);
	set_duty(&pwm_b, 0);
}
