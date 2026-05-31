/*
 * Brushed-DC motor driver - sign-magnitude H-bridge on two PWM channels.
 *
 * Matches the original ServoDrive: TIM CH1/CH2 drive the H-bridge; the sign of
 * the control output selects direction, the magnitude sets the duty (0..1000).
 */
#ifndef SERVODRIVE_MOTOR_H_
#define SERVODRIVE_MOTOR_H_

#define EB_MOTOR_DUTY_MAX 1000

int  eb_motor_init(void);

/* Drive the motor. `command` is the DCE output; positive = one direction,
 * negative = the other. Magnitude is mapped to PWM duty and clamped. */
void eb_motor_drive(float command);

void eb_motor_stop(void);

#endif /* SERVODRIVE_MOTOR_H_ */
