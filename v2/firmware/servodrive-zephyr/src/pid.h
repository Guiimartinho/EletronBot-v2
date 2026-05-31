/*
 * DCE ("Digital Control Engine") - the ServoDrive position controller.
 *
 * Same control law as the original firmware:
 *   out = kp*err + ki*integral + kv*vel_integral + kd*(err - prev_err)
 * with anti-windup clamping on the integrators and a final torque clamp.
 *
 * Kept as a pure, dependency-free struct so it can be unit-tested on the host
 * and reused by either a bare-metal or an RTOS build.
 */
#ifndef SERVODRIVE_PID_H_
#define SERVODRIVE_PID_H_

struct eb_dce {
	/* gains */
	float kp, ki, kv, kd;
	/* limits */
	float integral_limit;  /* anti-windup clamp (original default ~500) */
	float output_limit;    /* torque/output clamp */
	/* state */
	float integral;
	float vel_integral;
	float prev_error;
};

void  eb_dce_init(struct eb_dce *d, float kp, float ki, float kv, float kd,
		  float integral_limit, float output_limit);

void  eb_dce_reset(struct eb_dce *d);

/* One control step: target and measured in the same units (degrees).
 * `velocity` is the measured angular velocity (deg/tick); pass 0 if unused.
 * Returns the clamped control output (motor command, e.g. -limit..+limit).
 */
float eb_dce_update(struct eb_dce *d, float target, float measured,
		    float velocity);

#endif /* SERVODRIVE_PID_H_ */
