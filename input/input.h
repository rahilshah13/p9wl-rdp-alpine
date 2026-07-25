/*
 * input.h - Input handling for FreeRDP to Wayland bridge (Alpine headless RDP)
 *
 * This module handles:
 *   - Keyboard input: Processing FreeRDP scancodes and action flags
 *   - Mouse input: Processing FreeRDP mouse coordinates and buttons
 *   - Input queue: Thread-safe queue for passing events to main loop
 *
 * Usage:
 *
 *   Initialize the input queue before handling events:
 *
 *     input_queue_init(&server->input_queue);
 *
 *   Handle FreeRDP inputs via callbacks:
 *
 *     rdp_handle_mouse(server, flags, x, y);
 *     rdp_handle_keyboard(server, flags, keycode);
 */

#ifndef P9WL_INPUT_H
#define P9WL_INPUT_H

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

#include "../types.h"       /* For struct server, input_queue, input_event */

/* ============== Input Queue ============== */

/*
 * Initialize an input queue.
 *
 * Initializes the mutex and creates a notification pipe (read end
 * set non-blocking). The caller is responsible for adding the pipe
 * read fd to the event loop.
 *
 * q: queue to initialize
 */
void input_queue_init(struct input_queue *q);

/*
 * Push an event onto the input queue.
 *
 * Thread-safe. Events are silently dropped if queue is full.
 *
 * q:  queue to push to
 * ev: event to copy into queue
 */
void input_queue_push(struct input_queue *q, struct input_event *ev);

/*
 * Pop an event from the input queue.
 *
 * Thread-safe. Non-blocking - returns immediately if empty.
 *
 * q:  queue to pop from
 * ev: output - event copied from queue
 *
 * Returns 1 if event was popped, 0 if queue was empty.
 */
int input_queue_pop(struct input_queue *q, struct input_event *ev);

/* ============== FreeRDP Input Callbacks ============== */

/*
 * Handle incoming mouse events from FreeRDP.
 *
 * s:     server instance
 * flags: mouse action flags (e.g. PTR_FLAGS_BUTTON1)
 * x:     x coordinate
 * y:     y coordinate
 */
void rdp_handle_mouse(struct server *s, uint16_t flags, uint16_t x, uint16_t y);

/*
 * Handle incoming keyboard events from FreeRDP.
 *
 * s:       server instance
 * flags:   keyboard action flags (e.g. KBD_FLAGS_DOWN)
 * keycode: raw scancode from client
 */
void rdp_handle_keyboard(struct server *s, uint16_t flags, uint8_t keycode);

#endif /* P9WL_INPUT_H */