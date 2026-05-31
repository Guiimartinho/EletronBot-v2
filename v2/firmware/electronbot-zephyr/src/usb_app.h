/*
 * USB bulk transport for the ElectronBot frame protocol.
 *
 * The original device enumerates as CDC but is driven as a raw bulk pipe. On
 * Zephyr we expose a minimal vendor/bulk interface with one OUT and one IN
 * endpoint and implement the chunked frame handshake (see protocol.h).
 */
#ifndef ELECTRONBOT_USB_APP_H_
#define ELECTRONBOT_USB_APP_H_

#include <stdint.h>
#include "protocol.h"

/* Callback invoked when a full frame has been received into a stripe buffer.
 * `stripe_rgb` points to EB_STRIPE_BYTES of RGB888; `extra` is the decoded
 * 32-byte side-channel for that chunk.
 */
typedef void (*eb_chunk_cb_t)(int chunk_index,
			      const uint8_t *stripe_rgb,
			      const struct eb_extra_data *extra);

/* Provide the telemetry the device returns on each IN request (measured
 * joints). The transport copies 32 bytes from here when the host reads. */
typedef void (*eb_fill_telemetry_cb_t)(uint8_t out_buf32[EB_EXTRA_BYTES]);

int eb_usb_init(eb_chunk_cb_t on_chunk, eb_fill_telemetry_cb_t fill_telemetry);

/* True once the host has configured the device. */
int eb_usb_ready(void);

#endif /* ELECTRONBOT_USB_APP_H_ */
