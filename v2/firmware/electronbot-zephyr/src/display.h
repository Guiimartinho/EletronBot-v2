/*
 * GC9A01 round 240x240 LCD - stripe streaming over SPI + DMA.
 *
 * The original firmware bypasses a generic display API and streams raw RGB888
 * stripes straight to the panel via SPI DMA, double-buffered so a stripe DMA
 * overlaps the USB receive of the next stripe. We keep that model.
 */
#ifndef ELECTRONBOT_DISPLAY_H_
#define ELECTRONBOT_DISPLAY_H_

#include <stdint.h>
#include <stddef.h>

/* Initialize SPI, control GPIOs (CS/DC/RST/backlight) and run the GC9A01
 * power-on / magic init sequence. Returns 0 on success.
 */
int eb_display_init(void);

/* Begin a new frame (issues column/row address window + RAMWR 0x2C). */
void eb_display_begin_frame(void);

/* Push one RGB888 stripe (EB_STRIPE_BYTES). `first` selects RAMWR (0x2C) vs
 * RAM-continue (0x3C). Transfer is via SPI DMA; returns immediately. Call
 * eb_display_wait() before reusing the same buffer.
 */
void eb_display_push_stripe(const uint8_t *rgb, size_t len, int first);

/* Block until the in-flight DMA stripe completes. */
void eb_display_wait(void);

/* Backlight on/off. */
void eb_display_backlight(int on);

#endif /* ELECTRONBOT_DISPLAY_H_ */
