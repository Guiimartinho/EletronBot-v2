/*
 * ElectronBot head firmware - Zephyr port entry point.
 *
 * Target: STM32F405RGT6 (Cortex-M4F). Replaces the original CubeMX/HAL +
 * FreeRTOS super-loop firmware while keeping the host SDK protocol unchanged.
 */
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "robot.h"

LOG_MODULE_REGISTER(eb_main, LOG_LEVEL_INF);

int main(void)
{
	LOG_INF("ElectronBot-zephyr boot");

	int rc = eb_robot_init();

	if (rc) {
		LOG_ERR("init failed: %d", rc);
		return rc;
	}

	eb_robot_run(); /* never returns */
	return 0;
}
