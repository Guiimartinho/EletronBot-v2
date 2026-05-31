/*
 * DCE position controller implementation.
 */
#include "pid.h"

static float clampf(float v, float lim)
{
	if (v > lim) {
		return lim;
	}
	if (v < -lim) {
		return -lim;
	}
	return v;
}

void eb_dce_init(struct eb_dce *d, float kp, float ki, float kv, float kd,
		 float integral_limit, float output_limit)
{
	d->kp = kp;
	d->ki = ki;
	d->kv = kv;
	d->kd = kd;
	d->integral_limit = integral_limit;
	d->output_limit = output_limit;
	eb_dce_reset(d);
}

void eb_dce_reset(struct eb_dce *d)
{
	d->integral = 0.0f;
	d->vel_integral = 0.0f;
	d->prev_error = 0.0f;
}

float eb_dce_update(struct eb_dce *d, float target, float measured,
		    float velocity)
{
	float error = target - measured;
	float derivative = error - d->prev_error;

	d->integral = clampf(d->integral + error, d->integral_limit);
	d->vel_integral = clampf(d->vel_integral + velocity, d->integral_limit);

	float out = d->kp * error
		  + d->ki * d->integral
		  + d->kv * d->vel_integral
		  + d->kd * derivative;

	d->prev_error = error;
	return clampf(out, d->output_limit);
}
