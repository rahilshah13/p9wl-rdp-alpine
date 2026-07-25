/*
 * rdp_cmd.h - Rdp Surface Update & Encoding Helpers
 *
 * Inline helpers for structuring FreeRDP / RFX surface update headers
 * and command blocks for headless RDP compositing on Alpine Linux.
 *
 * Architecture Overview:
 *
 *   Replaces legacy protocol command framing with standard FreeRDP surface
 *   commands and bitmap update headers, optimized for musl/Alpine builds.
 */

#ifndef RDP_CMD_H
#define RDP_CMD_H

#include <stdint.h>
#include <string.h>

/* ============== Byte Order Macros ============== */

/* Write 32-bit little-endian value to buffer (Safe for Musl/Alpine alignment) */
#ifndef PUT32
#define PUT32(p, v) do { \
    uint32_t _v = (v); \
    (p)[0] = (uint8_t)(_v); \
    (p)[1] = (uint8_t)(_v >> 8); \
    (p)[2] = (uint8_t)(_v >> 16); \
    (p)[3] = (uint8_t)(_v >> 24); \
} while(0)
#endif

/* Write 16-bit little-endian value to buffer */
#ifndef PUT16
#define PUT16(p, v) do { \
    uint16_t _v = (v); \
    (p)[0] = (uint8_t)(_v); \
    (p)[1] = (uint8_t)(_v >> 8); \
} while(0)
#endif

/* ============== RDP Surface Commands ============== */

/*
 * Emit RDP surface command header for frame updates or bitmap regions.
 *
 * buf:     output buffer (must have 12 bytes available)
 * x1, y1:  region top-left coordinates
 * x2, y2:  region bottom-right coordinates
 *
 * Returns bytes written (always 12).
 */
static inline int rdp_surface_command_header(uint8_t *buf, int x1, int y1, int x2, int y2) {
    int off = 0;
    PUT16(buf + off, (uint16_t)x1); off += 2;
    PUT16(buf + off, (uint16_t)y1); off += 2;
    PUT16(buf + off, (uint16_t)(x2 - x1)); off += 2;
    PUT16(buf + off, (uint16_t)(y2 - y1)); off += 2;
    return off;  /* 8 bytes base rect */
}

/* ============== Pixel Format Constants ============== */

/*
 * Pixel channel format descriptors for RDP surface mapping.
 */
#define RDP_PIXEL_FORMAT_BGRA32  0x48081828  /* a8b8g8r8 - 32bpp surface mapping */
#define RDP_PIXEL_FORMAT_XRGB32  0x68081828  /* x8r8g8b8 - 32bpp with padding */

#endif /* RDP_CMD_H */