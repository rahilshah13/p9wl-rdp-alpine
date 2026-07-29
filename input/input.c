/*
 * input.c - Input handling (FreeRDP keyboard and mouse events mapped to input queue)
 */

#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>

#include <freerdp/log.h>
#include <freerdp/input.h>

#include "input.h"

#define TAG FREERDP_TAG("p9wl.input")

/* ============== Input Queue ============== */

void input_queue_init(struct input_queue *q) {
    q->head = q->tail = 0;
    pthread_mutex_init(&q->lock, NULL);
    if (pipe(q->pipe_fd) < 0) {
        WLog_ERR(TAG, "pipe failed: %s", strerror(errno));
        q->pipe_fd[0] = q->pipe_fd[1] = -1;
    }
    fcntl(q->pipe_fd[0], F_SETFL, O_NONBLOCK);
}

void input_queue_push(struct input_queue *q, struct input_event *ev) {
    pthread_mutex_lock(&q->lock);
    int next = (q->tail + 1) % INPUT_QUEUE_SIZE;
    if (next != q->head) {
        q->events[q->tail] = *ev;
        q->tail = next;
        char c = 1;
        if (write(q->pipe_fd[1], &c, 1) < 0) { /* ignore */ }
    }
    pthread_mutex_unlock(&q->lock);
}

int input_queue_pop(struct input_queue *q, struct input_event *ev) {
    pthread_mutex_lock(&q->lock);
    if (q->head == q->tail) {
        pthread_mutex_unlock(&q->lock);
        return 0;
    }
    *ev = q->events[q->head];
    q->head = (q->head + 1) % INPUT_QUEUE_SIZE;
    pthread_mutex_unlock(&q->lock);
    return 1;
}

/* ============== FreeRDP Input Callbacks ============== */

void rdp_handle_mouse(struct server *s, uint16_t flags, uint16_t x, uint16_t y) {
    int buttons = 0;
    
    if (flags & PTR_FLAGS_BUTTON1) buttons |= 1;
    if (flags & PTR_FLAGS_BUTTON2) buttons |= 4;
    if (flags & PTR_FLAGS_BUTTON3) buttons |= 2;

    struct input_event ev = {
        .type = INPUT_MOUSE,
        .mouse = { .x = x, .y = y, .buttons = buttons }
    };
    input_queue_push(&s->input_queue, &ev);
}

void rdp_handle_keyboard(struct server *s, uint16_t flags, uint8_t keycode) {
    int pressed = (flags & KBD_FLAGS_DOWN) ? 1 : 0;

    struct input_event ev = {
        .type = INPUT_KEY,
        .key = { .rune = keycode, .pressed = pressed }
    };
    input_queue_push(&s->input_queue, &ev);
}


void *mouse_thread_func(void *arg) {
    (void)arg;
    return NULL;
}

void *kbd_thread_func(void *arg) {
    (void)arg;
    return NULL;
}