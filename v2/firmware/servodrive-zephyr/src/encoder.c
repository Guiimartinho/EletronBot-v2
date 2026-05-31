/*
 * Potentiometer feedback via Zephyr ADC.
 */
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/logging/log.h>

#include "encoder.h"

LOG_MODULE_REGISTER(eb_encoder, LOG_LEVEL_INF);

#define ADC_NODE DT_ALIAS(pot_adc)

static const struct adc_dt_spec adc_ch = ADC_DT_SPEC_GET(DT_ALIAS(pot_adc));
static int16_t s_sample;

int eb_encoder_init(void)
{
	if (!adc_is_ready_dt(&adc_ch)) {
		LOG_ERR("ADC not ready");
		return -ENODEV;
	}
	return adc_channel_setup_dt(&adc_ch);
}

int eb_encoder_read_raw(void)
{
	struct adc_sequence seq = {
		.buffer = &s_sample,
		.buffer_size = sizeof(s_sample),
	};

	(void)adc_sequence_init_dt(&adc_ch, &seq);
	if (adc_read_dt(&adc_ch, &seq) < 0) {
		return -EIO;
	}
	return s_sample;
}

float eb_encoder_raw_to_angle(const struct eb_pot_cal *cal, int raw)
{
	float span_raw = (float)(cal->raw_max - cal->raw_min);
	float span_ang = cal->angle_max - cal->angle_min;
	float t;

	if (span_raw == 0.0f) {
		return cal->angle_min;
	}
	t = ((float)(raw - cal->raw_min)) / span_raw;
	return cal->angle_min + t * span_ang;
}
