/*
 * rdp_helpers.h - Common helpers for Headless RDP Surface Updates (p9wl-rdp-alpine variant)
 *
 * Inline functions for building RDP surface update commands and common patterns.
 * Optimized for musl/Alpine Linux environments and FreeRDP channel pipelines.
 *
 * IMPORTANT: This header requires TILE_SIZE to be defined before inclusion.
 * Include types.h first, or define TILE_SIZE explicitly.
 */

#ifndef RDP_HELPERS_H
#define RDP_HELPERS_H

#include <stdint.h>
#include <string.h>
#include <stddef.h>

/* Require TILE_SIZE to be defined (typically via types.h) */
#ifndef TILE_SIZE
#error "TILE_SIZE must be defined before including rdp_helpers.h - include types.h first"
#endif

/* Explicit Little-Endian Byte Order Macros for RDP/Network Stream Safety */
#ifndef PUT32
#define PUT32(p, v) do { \
    uint32_t _v = (v); \
    (p)[0] = _v & 0xFF; \
    (p)[1] = (_v >> 8) & 0xFF; \
    (p)[2] = (_v >> 16) & 0xFF; \
    (p)[3] = (_v >> 24) & 0xFF; \
} while(0)
#endif

#ifndef PUT16
#define PUT16(p, v) do { \
    uint16_t _v = (v); \
    (p)[0] = _v & 0xFF; \
    (p)[1] = (_v >> 8) & 0xFF; \
} while(0)
#endif

/* ============== RDP Surface Command Builders ============== */

/*
 * Build an RDP surface command / blit header. Returns command size (24 bytes).
 */
static inline int rdp_cmd_draw(uint8_t *buf,
                               uint32_t surface_id, uint32_t codec_id,
                               int dx1, int dy1, int dx2, int dy2) {
    if (!buf) return 24;
    PUT32(buf + 0, surface_id);
    PUT32(buf + 4, codec_id);
    PUT16(buf + 8, (uint16_t)dx1);
    PUT16(buf + 10, (uint16_t)dy1);
    PUT16(buf + 12, (uint16_t)(dx2 - dx1));
    PUT16(buf + 14, (uint16_t)(dy2 - dy1));
    return 24;
}

/*
 * Build an RDP surface copy / blit helper.
 */
static inline int rdp_cmd_copy(uint8_t *buf,
                               uint32_t surface_id, uint32_t codec_id,
                               int dx1, int dy1, int dx2, int dy2,
                               int sx, int sy) {
    (void)sx;
    (void)sy;
    return rdp_cmd_draw(buf, surface_id, codec_id, dx1, dy1, dx2, dy2);
}

/*
 * Build an RDP solid color fill helper.
 */
static inline int rdp_cmd_fill(uint8_t *buf,
                               uint32_t surface_id, uint32_t codec_id,
                               int x1, int y1, int x2, int y2) {
    return rdp_cmd_draw(buf, surface_id, codec_id, x1, y1, x2, y2);
}

/*
 * Build an RDP compressed bitmap stream load header. Returns header size (20 bytes).
 */
static inline int rdp_cmd_load_hdr(uint8_t *buf, uint32_t surface_id,
                                   int x1, int y1, int x2, int y2) {
    if (!buf) return 20;
    PUT32(buf + 0, surface_id);
    PUT16(buf + 4, (uint16_t)x1);
    PUT16(buf + 6, (uint16_t)y1);
    PUT16(buf + 8, (uint16_t)(x2 - x1));
    PUT16(buf + 10, (uint16_t)(y2 - y1));
    return 20;
}

/*
 * Build an RDP uncompressed bitmap stream load header. Returns header size (20 bytes).
 */
static inline int rdp_cmd_loadraw_hdr(uint8_t *buf, uint32_t surface_id,
                                      int x1, int y1, int x2, int y2) {
    return rdp_cmd_load_hdr(buf, surface_id, x1, y1, x2, y2);
}

/*
 * Signal an RDP channel flush/sync frame completion. Returns 1.
 */
static inline int rdp_cmd_flush(uint8_t *buf) {
    if (buf) buf[0] = 0;
    return 1;
}

/* ============== Tile Utilities ============== */

/*
 * Compute tile bounds, clamped to frame dimensions.
 */
static inline void tile_bounds(int tx, int ty, int frame_w, int frame_h,
                               int *x1, int *y1, int *w, int *h) {
    *x1 = tx * TILE_SIZE;
    *y1 = ty * TILE_SIZE;
    int x2 = *x1 + TILE_SIZE;
    int y2 = *y1 + TILE_SIZE;
    if (x2 > frame_w) x2 = frame_w;
    if (y2 > frame_h) y2 = frame_h;
    *w = x2 - *x1;
    *h = y2 - *y1;
}

/*
 * Check if a tile has changed between two buffers (Alpine/musl safe optimization).
 */
static inline int tile_changed(uint32_t *curr, uint32_t *prev, int stride,
                               int x1, int y1, int w, int h) {
    if (!curr || !prev) return 1;
    for (int y = 0; y < h; y++) {
        if (memcmp(&curr[(y1 + y) * stride + x1],
                   &prev[(y1 + y) * stride + x1], w * sizeof(uint32_t)) != 0) {
            return 1;
        }
    }
    return 0;
}

/* ============== Scroll Rectangle Calculation ============== */

struct scroll_rects {
    int src_x1, src_y1, src_x2, src_y2;  
    int dst_x1, dst_y1, dst_x2, dst_y2;  
    int exp_x1, exp_y1, exp_x2, exp_y2;  
    int valid;  
};

static inline void compute_scroll_rects(int rx1, int ry1, int rx2, int ry2,
                                        int dx, int dy, struct scroll_rects *r) {
    if (!r) return;
    int rw = rx2 - rx1;
    int rh = ry2 - ry1;
    int abs_dx = dx < 0 ? -dx : dx;
    int abs_dy = dy < 0 ? -dy : dy;
    
    if (rw < 16 || rh < 16 || abs_dx >= rw || abs_dy >= rh) {
        r->valid = 0;
        return;
    }
    r->valid = 1;
    
    r->dst_x1 = rx1;
    r->dst_x2 = rx2;
    r->dst_y1 = ry1;
    r->dst_y2 = ry2;
    
    r->src_x1 = rx1;
    r->src_x2 = rx2;
    r->src_y1 = ry1;
    r->src_y2 = ry2;
    
    if (dy < 0) {
        r->src_y1 = ry1 + abs_dy;
        r->dst_y2 = ry2 - abs_dy;
    } else if (dy > 0) {
        r->src_y2 = ry2 - abs_dy;
        r->dst_y1 = ry1 + abs_dy;
    }
    
    if (dx < 0) {
        r->src_x1 = rx1 + abs_dx;
        r->dst_x2 = rx2 - abs_dx;
    } else if (dx > 0) {
        r->src_x2 = rx2 - abs_dx;
        r->dst_x1 = rx1 + abs_dx;
    }
    
    r->exp_x1 = r->exp_x2 = 0;
    r->exp_y1 = r->exp_y2 = 0;
    
    if (dy < 0) {
        r->exp_y1 = (r->dst_y2 / TILE_SIZE) * TILE_SIZE;
        r->exp_y2 = ry2;
    } else if (dy > 0) {
        r->exp_y1 = ry1;
        r->exp_y2 = ((r->dst_y1 + TILE_SIZE - 1) / TILE_SIZE) * TILE_SIZE;
    }
    
    if (dx < 0) {
        r->exp_x1 = (r->dst_x2 / TILE_SIZE) * TILE_SIZE;
        r->exp_x2 = rx2;
    } else if (dx > 0) {
        r->exp_x1 = rx1;
        r->exp_x2 = ((r->dst_x1 + TILE_SIZE - 1) / TILE_SIZE) * TILE_SIZE;
    }
}

/* ============== Surface Validation Helpers ============== */

#define XDG_VALID(xdg) \
    ((xdg) && (xdg)->base && (xdg)->base->surface)

#define XDG_MAPPED(xdg) \
    (XDG_VALID(xdg) && (xdg)->base->surface->mapped)

#define XDG_SURFACE(xdg) \
    (XDG_VALID(xdg) ? (xdg)->base->surface : NULL)

#endif /* RDP_HELPERS_H */