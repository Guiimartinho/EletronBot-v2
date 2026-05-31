/*
 * USB bulk transport (Zephyr) implementing the ElectronBot frame handshake.
 *
 * Per chunk (the device side mirrors the host's SyncTask):
 *   1. The device makes 32 bytes of telemetry available on EP IN; the host
 *      reads it (this is the "request"/handshake + measured-joints channel).
 *   2. The host writes 84 x 512 bytes of image to EP OUT -> accumulated.
 *   3. The host writes a 224-byte tail (192 image + 32 extra) -> completes the
 *      stripe and triggers the chunk callback; the 224-byte short packet is the
 *      delimiter that advances/wraps the ping-pong buffer.
 *
 * This file targets Zephyr's USB device stack. The exact descriptor/class glue
 * differs between the legacy `usb_device` stack and `usb_device_next`; the
 * structure here is stack-agnostic and the integration points are marked TODO.
 * The framing/state machine itself is final and matches the host byte-for-byte.
 */
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <string.h>

#include "usb_app.h"
#include "protocol.h"

LOG_MODULE_REGISTER(eb_usb, LOG_LEVEL_INF);

static eb_chunk_cb_t          s_on_chunk;
static eb_fill_telemetry_cb_t s_fill_telemetry;
static volatile int           s_ready;

/* Ping-pong stripe buffers: [body 43008][tail-image 192][extra 32]. */
static uint8_t s_rx[2][EB_RX_BUF_BYTES];
static int     s_rx_index;
static size_t  s_rx_off;
static int     s_chunk;

/* Called by the OUT endpoint handler with each received bulk packet.
 * Aggregates by length: 512-byte packets are body data; a 224-byte packet is
 * the chunk terminator. */
static void on_out_packet(const uint8_t *data, size_t len)
{
	uint8_t *buf = s_rx[s_rx_index];

	if (len == EB_PACKET_SIZE) {
		if (s_rx_off + len <= EB_CHUNK_BODY) {
			memcpy(buf + s_rx_off, data, len);
			s_rx_off += len;
		}
		return;
	}

	if (len == EB_TAIL_SIZE) {
		/* 192 image bytes complete the stripe, 32 bytes are extra-data. */
		memcpy(buf + EB_CHUNK_BODY, data, EB_TAIL_IMAGE_BYTES);

		struct eb_extra_data extra;
		eb_extra_decode(data + EB_TAIL_IMAGE_BYTES, &extra);

		if (s_on_chunk) {
			s_on_chunk(s_chunk, buf, &extra);
		}

		/* advance chunk; wrap frame after 4 chunks, flip ping-pong. */
		s_chunk = (s_chunk + 1) % EB_CHUNKS;
		s_rx_off = 0;
		s_rx_index ^= 1;
		return;
	}

	LOG_WARN("unexpected OUT packet len=%u", (unsigned)len);
}

/* Called when the host issues an IN read: hand back 32 telemetry bytes. */
static void on_in_request(uint8_t *out, size_t *out_len)
{
	if (s_fill_telemetry) {
		s_fill_telemetry(out);
	} else {
		memset(out, 0, EB_EXTRA_BYTES);
	}
	*out_len = EB_EXTRA_BYTES;
}

int eb_usb_init(eb_chunk_cb_t on_chunk, eb_fill_telemetry_cb_t fill_telemetry)
{
	s_on_chunk = on_chunk;
	s_fill_telemetry = fill_telemetry;
	s_rx_index = 0;
	s_rx_off = 0;
	s_chunk = 0;

	/*
	 * TODO (stack integration):
	 *  - Register a vendor/bulk class with VID=EB_USB_VID, PID=EB_USB_PID,
	 *    manufacturer/product strings, one OUT (EB_EP_OUT) and one IN
	 *    (EB_EP_IN) endpoint, wMaxPacketSize 512 (HS) / 64 (FS).
	 *  - Wire the OUT endpoint completion to on_out_packet(data, len).
	 *  - Wire the IN endpoint request to on_in_request(buf, &len) and submit.
	 *  - Set s_ready = 1 on the SET_CONFIGURATION event.
	 *  - usb_enable().
	 *
	 * The two functions above (on_out_packet / on_in_request) are the only
	 * coupling points; the protocol logic is complete.
	 */
	(void)on_in_request; /* referenced by the endpoint glue once wired */
	LOG_INF("eb_usb framing ready (endpoint glue is the integration TODO)");
	return 0;
}

int eb_usb_ready(void)
{
	return s_ready;
}
