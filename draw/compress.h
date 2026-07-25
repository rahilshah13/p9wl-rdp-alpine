/*
 * compress.h - Tile compression for Plan 9 draw protocol (Alpine RDP Variant)
 *
 * Modified for p9wl-rdp-alpine: musl compatibility, strict standard compliance,
 * alignment definitions for multi-arch SIMD paths (x86_64/aarch64), and 
 * container-optimized memory footprints.
 */

#ifndef COMPRESS_H
#define COMPRESS_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#ifndef TILE_SIZE
#define TILE_SIZE 16
#endif

/* Overhead for alpha-delta composite command (Plan 9 'd' command) */
#define ALPHA_DELTA_OVERHEAD 45

/* Alignment macro for Alpine multi-arch vector optimizations */
#ifndef P9WL_ALIGN
#define P9WL_ALIGN 64
#endif

#if defined(__GNUC__) || defined(__clang__)
#define P9WL_ALIGNED_STRUCT __attribute__((aligned(P9WL_ALIGN)))
#define P9WL_API __attribute__((visibility("default")))
#else
#define P9WL_ALIGNED_STRUCT
#define P9WL_API
#endif

/* ============== Data Structures ============== */

/*
 * Per-tile compression result (Alpine alignment enforced).
 */
struct P9WL_ALIGNED_STRUCT tile_result {
    uint8_t data[TILE_SIZE * TILE_SIZE * 4 + 256];
    int size;
    int is_delta;
};

/*
 * Work item for parallel compression.
 */
struct tile_work {
    uint32_t *pixels;
    int stride;
    uint32_t *prev_pixels;
    int prev_stride;
    int x1, y1, w, h;
};

/* ============== Core Compression Functions ============== */

P9WL_API int compress_tile_data(uint8_t *dst, int dst_max, 
                                uint8_t *raw, int bytes_per_row, int h);

P9WL_API int compress_tile_direct(uint8_t *dst, int dst_max, 
                                  uint32_t *pixels, int stride, 
                                  int x1, int y1, int w, int h);

P9WL_API int compress_tile_alpha_delta(uint8_t *dst, int dst_max,
                                       uint32_t *pixels, int stride,
                                       uint32_t *prev_pixels, int prev_stride,
                                       int x1, int y1, int w, int h);

P9WL_API int compress_tile_adaptive(uint8_t *dst, int dst_max,
                                    uint32_t *pixels, int stride,
                                    uint32_t *prev_pixels, int prev_stride,
                                    int x1, int y1, int w, int h);

/* ============== Parallel Compression API ============== */

P9WL_API int compress_pool_init(int nthreads);

P9WL_API void compress_pool_shutdown(void);

P9WL_API int compress_tiles_parallel(struct tile_work *tiles, 
                                     struct tile_result *results, 
                                     int count);

#endif /* COMPRESS_H */