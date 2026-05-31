/*
 * ElectronBot USB wire protocol - byte-compatible with the original host SDK.
 *
 * This is the keystone of the Zephyr port: the host (ElectronBotSDK-LowLevel)
 * is unchanged, so the firmware MUST reproduce this framing exactly.
 *
 * One face frame = 240 x 240 x 3 (RGB888) = 172800 bytes, sent in 4 chunks.
 * Per chunk, the host:
 *   1. reads 32 bytes on EP IN  (the device "request" + telemetry channel),
 *   2. writes 84 x 512 = 43008 bytes of image on EP OUT,
 *   3. writes a 224-byte tail (192 image bytes + 32 extra-data bytes).
 * 4 * (43008 + 192) = 172800 = one full frame.
 *
 * The 224-byte short packet is the end-of-frame delimiter the firmware keys on
 * to swap its receive ping-pong buffer.
 *
 * The 32-byte extra-data block (host -> device, in the tail; device -> host, in
 * the IN request) carries servo data:
 *   byte[0]      = enable flag (1 = drive servos)
 *   byte[1..24]  = 6 IEEE-754 little-endian float joint set-points / measured
 *   byte[25..31] = reserved
 */
#ifndef ELECTRONBOT_PROTOCOL_H_
#define ELECTRONBOT_PROTOCOL_H_

#include <stdint.h>

/* USB identity - must match the host defaults (electron_low_level.h). */
#define EB_USB_VID            0x1001
#define EB_USB_PID            0x8023
#define EB_USB_MANUFACTURER   "Pengzhihui"
#define EB_USB_PRODUCT        "ElectronBot@PZH"

/* Bulk endpoints (host view): OUT 0x01 (host->dev), IN 0x81 (dev->host). */
#define EB_EP_OUT             0x01
#define EB_EP_IN              0x81

/* Display geometry. */
#define EB_LCD_W              240
#define EB_LCD_H              240
#define EB_BYTES_PER_PIXEL    3
#define EB_FRAME_BYTES        (EB_LCD_W * EB_LCD_H * EB_BYTES_PER_PIXEL) /* 172800 */

/* Framing. */
#define EB_CHUNKS             4
#define EB_CHUNK_PACKETS      84
#define EB_PACKET_SIZE        512
#define EB_CHUNK_BODY         (EB_CHUNK_PACKETS * EB_PACKET_SIZE)        /* 43008 */
#define EB_TAIL_SIZE          224
#define EB_TAIL_IMAGE_BYTES   192
#define EB_EXTRA_BYTES        32
#define EB_CHUNK_IMAGE_BYTES  (EB_CHUNK_BODY + EB_TAIL_IMAGE_BYTES)     /* 43200 */

/* One LCD stripe rendered per chunk: 60 rows x 240 x 3 = 43200 bytes. */
#define EB_STRIPE_ROWS        (EB_LCD_H / EB_CHUNKS)                    /* 60 */
#define EB_STRIPE_BYTES       (EB_STRIPE_ROWS * EB_LCD_W * EB_BYTES_PER_PIXEL)

/* Servo channel layout inside the 32-byte extra-data block. */
#define EB_JOINT_COUNT        6
#define EB_EXTRA_ENABLE_OFF   0
#define EB_EXTRA_JOINTS_OFF   1   /* 6 * float32, little-endian */

/* Decoded extra-data block. */
struct eb_extra_data {
	uint8_t enable;
	float   joints[EB_JOINT_COUNT];
};

/* Receive buffer: one full frame plus the trailing 32-byte control block.
 * Matches the host/firmware rxData[2][60*240*3 + 32] ping-pong sizing.
 */
#define EB_RX_BUF_BYTES       (EB_STRIPE_BYTES + EB_EXTRA_BYTES) /* 43232 */

/* Pack/unpack helpers (implemented in protocol.c). */
void eb_extra_decode(const uint8_t *buf32, struct eb_extra_data *out);
void eb_extra_encode(const struct eb_extra_data *in, uint8_t *buf32);

#endif /* ELECTRONBOT_PROTOCOL_H_ */
