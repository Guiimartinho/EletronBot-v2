/*
 * GC9A01 stripe streaming driver (Zephyr SPI + GPIO).
 *
 * Implementation notes:
 *  - Uses the Zephyr SPI driver (async API for DMA-backed transfers where the
 *    SoC SPI supports it). On STM32 the SPI DMA is wired via the devicetree.
 *  - DC/CS/RST/backlight are GPIOs from the devicetree (see board overlay).
 *  - The init sequence below mirrors the original firmware's GC9A01 magic
 *    registers (0xEF / 0xEB / 0xFE ... MADCTL 0x36, COLMOD 0x3A, etc.).
 *
 * Hardware-tuning TODOs are marked; these need a scope/panel to finalize.
 */
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

#include "display.h"
#include "protocol.h"

LOG_MODULE_REGISTER(eb_display, LOG_LEVEL_INF);

#define SPI_NODE   DT_NODELABEL(eb_lcd_spi)
#define DC_NODE    DT_NODELABEL(eb_lcd_dc)
#define RST_NODE   DT_NODELABEL(eb_lcd_rst)
#define BL_NODE    DT_NODELABEL(eb_lcd_bl)

static const struct device *spi_dev;
static struct gpio_dt_spec dc_gpio  = GPIO_DT_SPEC_GET_OR(DC_NODE,  gpios, {0});
static struct gpio_dt_spec rst_gpio = GPIO_DT_SPEC_GET_OR(RST_NODE, gpios, {0});
static struct gpio_dt_spec bl_gpio  = GPIO_DT_SPEC_GET_OR(BL_NODE,  gpios, {0});

static struct spi_config spi_cfg = {
	.frequency = 40000000U, /* TODO: confirm max stable SPI clock for the panel */
	.operation = SPI_WORD_SET(8) | SPI_TRANSFER_MSB | SPI_OP_MODE_MASTER,
	.slave = 0,
};

/* GC9A01 init program: {cmd, n_args, args...}. Condensed from the original
 * firmware's screen.cpp magic sequence. */
static const uint8_t gc9a01_init[] = {
	0xEF, 0,
	0xEB, 1, 0x14,
	0xFE, 0,
	0xEF, 0,
	0xEB, 1, 0x14,
	0x84, 1, 0x40,
	0x86, 1, 0xFF,
	0x88, 1, 0x0A,
	0x8A, 1, 0x00,
	0x8D, 1, 0x14,
	0x36, 1, 0x48,        /* MADCTL */
	0x3A, 1, 0x06,        /* COLMOD: 18-bit (3 bytes/pixel) */
	0x90, 4, 0x08, 0x08, 0x08, 0x08,
	0xBD, 1, 0x06,
	0xBC, 1, 0x00,
	0xFF, 3, 0x60, 0x01, 0x04,
	0xC3, 1, 0x13,
	0xC4, 1, 0x13,
	0xC9, 1, 0x22,
	0xBE, 1, 0x11,
	0xE1, 2, 0x10, 0x0E,
	0xDF, 3, 0x21, 0x0c, 0x02,
	0x11, 0,              /* sleep out */
	0x00                  /* terminator (cmd 0x00 unused -> end) */
};

static int spi_write_bytes(const uint8_t *data, size_t len)
{
	struct spi_buf buf = { .buf = (void *)data, .len = len };
	struct spi_buf_set set = { .buffers = &buf, .count = 1 };

	return spi_write(spi_dev, &spi_cfg, &set);
}

static void write_cmd(uint8_t cmd)
{
	gpio_pin_set_dt(&dc_gpio, 0); /* command */
	spi_write_bytes(&cmd, 1);
}

static void write_data(const uint8_t *data, size_t len)
{
	gpio_pin_set_dt(&dc_gpio, 1); /* data */
	spi_write_bytes(data, len);
}

static void run_init_program(const uint8_t *prog)
{
	size_t i = 0;

	while (prog[i] != 0x00 || i == 0) {
		uint8_t cmd = prog[i++];
		uint8_t n = prog[i++];

		write_cmd(cmd);
		if (n) {
			write_data(&prog[i], n);
			i += n;
		}
		if (cmd == 0x11) {
			k_msleep(120); /* sleep-out settle */
		}
		if (prog[i] == 0x00) {
			break;
		}
	}
}

int eb_display_init(void)
{
	spi_dev = DEVICE_DT_GET(SPI_NODE);
	if (!device_is_ready(spi_dev)) {
		LOG_ERR("SPI not ready");
		return -ENODEV;
	}
	gpio_pin_configure_dt(&dc_gpio, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&rst_gpio, GPIO_OUTPUT_ACTIVE);
	gpio_pin_configure_dt(&bl_gpio, GPIO_OUTPUT_INACTIVE);

	/* Reset pulse */
	gpio_pin_set_dt(&rst_gpio, 1);
	k_msleep(10);
	gpio_pin_set_dt(&rst_gpio, 0);
	k_msleep(10);
	gpio_pin_set_dt(&rst_gpio, 1);
	k_msleep(120);

	run_init_program(gc9a01_init);

	write_cmd(0x29); /* display on */
	k_msleep(20);
	eb_display_backlight(1);
	LOG_INF("GC9A01 initialized");
	return 0;
}

void eb_display_begin_frame(void)
{
	/* Full-screen address window 0..239 x 0..239. */
	uint8_t col[] = {0x00, 0x00, 0x00, EB_LCD_W - 1};
	uint8_t row[] = {0x00, 0x00, 0x00, EB_LCD_H - 1};

	write_cmd(0x2A); write_data(col, sizeof(col));
	write_cmd(0x2B); write_data(row, sizeof(row));
}

void eb_display_push_stripe(const uint8_t *rgb, size_t len, int first)
{
	write_cmd(first ? 0x2C : 0x3C); /* RAMWR / RAM-continue */
	/* TODO: switch to spi_transceive_signal()/async for true DMA overlap. */
	write_data(rgb, len);
}

void eb_display_wait(void)
{
	/* With synchronous spi_write above this is a no-op. When async DMA is
	 * enabled, block on the SPI completion signal here. */
}

void eb_display_backlight(int on)
{
	gpio_pin_set_dt(&bl_gpio, on ? 1 : 0);
}
