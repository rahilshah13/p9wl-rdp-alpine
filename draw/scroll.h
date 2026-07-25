/* scroll.h - Scroll detection and FreeRDP surface update command generation */

#ifndef SCROLL_H
#define SCROLL_H

#include <stdint.h>
#include <stddef.h>

struct server;

/* Minimum scroll amount to detect (pixels) */
#define MIN_SCROLL_PIXELS 1

/* Detect scrolling in all regions of the frame */
void detect_scroll(struct server *s, uint32_t *send_buf);

/* Apply detected scroll to prev_framebuf */
int apply_scroll_to_prevbuf(struct server *s);

/* Write scroll copy commands to batch buffer */
int write_scroll_commands(struct server *s, uint8_t *batch, size_t max_size);

/* Timing statistics from the last detect_scroll() call */
struct scroll_timing {
    double total_us;
    int regions_processed;
    int regions_detected;
};

/* Get timing statistics from the last detect_scroll() call */
const struct scroll_timing *scroll_get_timing(void);

/* Initialize scroll detection resources */
void scroll_init(void);

/* Cleanup scroll detection resources */
void scroll_cleanup(void);

#endif /* SCROLL_H */