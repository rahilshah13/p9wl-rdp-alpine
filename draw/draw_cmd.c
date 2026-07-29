#include <stdint.h>
#include "draw_cmd.h"

int cmd_copy(uint8_t *cmd, uint32_t dstid, uint32_t srcid,
             int r_minx, int r_miny, int r_maxx, int r_maxy,
             int dx, int dy) {
    cmd[0] = 'c';
    *(uint32_t *)(cmd + 1) = dstid;
    *(uint32_t *)(cmd + 5) = srcid;
    *(uint32_t *)(cmd + 9)  = (uint32_t)r_minx;
    *(uint32_t *)(cmd + 13) = (uint32_t)r_miny;
    *(uint32_t *)(cmd + 17) = (uint32_t)r_maxx;
    *(uint32_t *)(cmd + 21) = (uint32_t)r_maxy;
    *(uint32_t *)(cmd + 25) = (uint32_t)dx;
    *(uint32_t *)(cmd + 29) = (uint32_t)dy;
    return 33;
}