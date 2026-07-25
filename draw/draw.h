/*
 * draw.h - Headless RDP Surface Initialization and Management
 */

#ifndef P9WL_DRAW_H
#define P9WL_DRAW_H

#include "../types.h"

/* Surface initialization */
int init_draw(struct server *s);

/* RDP surface layout and session management */
int relookup_window(struct server *s);
void delete_rio_window(void *p9);

#endif /* P9WL_DRAW_H */