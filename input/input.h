#ifndef P9WL_INPUT_H
#define P9WL_INPUT_H

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

#include "../types.h"       /* For struct server, input_queue, input_event */

/* ============== Input Queue ============== */

void input_queue_init(struct input_queue *q);
void input_queue_push(struct input_queue *q, struct input_event *ev);
int input_queue_pop(struct input_queue *q, struct input_event *ev);

/* ============== FreeRDP Input Callbacks ============== */

void rdp_handle_mouse(struct server *s, uint16_t flags, uint16_t x, uint16_t y);
void rdp_handle_keyboard(struct server *s, uint16_t flags, uint8_t keycode);

/* ============== Keymap Handling ============== */

struct key_map {
    uint32_t keycode;
    uint8_t shift;
    uint8_t ctrl;
};

uint32_t keymapmod(uint32_t rune);
const struct key_map *keymap_lookup(uint32_t rune);

#endif /* P9WL_INPUT_H */