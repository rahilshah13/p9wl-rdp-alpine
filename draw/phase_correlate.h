/* phase_correlate.h - FFT-based scroll detection */

#ifndef PHASE_CORRELATE_H
#define PHASE_CORRELATE_H

#include <stdint.h>

/* FFT size for phase correlation */
#define FFT_SIZE 256

/* Maximum detectable scroll distance */
#define MAX_SCROLL_DETECT (FFT_SIZE / 2)

/* Result of phase correlation detection */
struct phase_result {
    int dx;
    int dy;
    int valid;
};

/* Detect scroll between current and previous frame regions */
struct phase_result phase_correlate_detect(
    uint32_t *curr_buf, uint32_t *prev_buf, int buf_width,
    int rx1, int ry1, int rx2, int ry2,
    int max_shift
);

/* Free global FFT resources */
void phase_correlate_cleanup(void);

#endif /* PHASE_CORRELATE_H */