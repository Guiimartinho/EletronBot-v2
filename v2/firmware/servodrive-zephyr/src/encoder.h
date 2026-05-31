/*
 * Position feedback - potentiometer read via ADC.
 *
 * The ServoDrive uses a potentiometer on an ADC channel (NOT a magnetic
 * encoder), linearly calibrated raw counts -> mechanical angle.
 */
#ifndef SERVODRIVE_ENCODER_H_
#define SERVODRIVE_ENCODER_H_

#include <stdint.h>

/* Calibration: raw ADC counts at the mechanical 0 and 180 degree ends. */
struct eb_pot_cal {
	int   raw_min;   /* e.g. 250  */
	int   raw_max;   /* e.g. 3000 */
	float angle_min; /* e.g. 0    */
	float angle_max; /* e.g. 180  */
};

int   eb_encoder_init(void);

/* Read the raw ADC value (averaged). Returns counts, or <0 on error. */
int   eb_encoder_read_raw(void);

/* Convert raw counts to angle (deg) using the calibration. */
float eb_encoder_raw_to_angle(const struct eb_pot_cal *cal, int raw);

#endif /* SERVODRIVE_ENCODER_H_ */
