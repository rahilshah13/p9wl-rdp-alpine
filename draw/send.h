/*
 * send.h - Frame sending and send thread (FreeRDP channel encoder backend)
 *
 * Handles queuing frames from the compositor thread and sending them
 * over RDP surface updates using FreeRDP channel encoders.
 *
 * Architecture Overview:
 *
 *   The send system uses a producer-consumer model with three threads:
 *
 *     Compositor Thread (producer):
 *       - Renders frames to s->framebuf
 *       - Calls send_frame() to queue for transmission
 *       - Never blocks waiting for network I/O
 *
 *     Send Thread (consumer):
 *       - Waits for frames via s->send_cond
 *       - Detects changed tiles and scrolling
 *       - Encodes surfaces via FreeRDP/RFX
 *       - Batches commands and streams over RDP channels
 *       - Updates s->prev_framebuf for reference
 *
 *     Drain Thread (I/O helper):
 *       - Reads incoming RDP client channel events asynchronously
 *       - Allows pipelined writes without blocking
 *       - Handles error detection and recovery
 *       - Signals done_cond after each completed response to wake
 *         rdp_drain_throttle() and rdp_drain_pause() without polling
 *
 * Double Buffering:
 *
 *   Uses two send buffers (s->send_buf[0], s->send_buf[1]) to decouple
 *   the compositor from network transmission:
 *
 *     s->pending_buf: Buffer just queued, waiting for send thread
 *     s->active_buf:  Buffer currently being transmitted
 *
 *   send_frame() finds a free buffer (neither pending nor active),
 *   swaps the framebuffer pointer with the free send buffer (zero-copy),
 *   copies the dirty tile bitmap from staging, and signals the send
 *   thread. All three buffers (framebuf, send_buf[0], send_buf[1])
 *   must be allocated at the padded size (TILE_ALIGN_UP of visible
 *   dimensions) since their pointers are swapped.
 *
 * Damage-Based Dirty Tile Tracking:
 *
 *   To avoid scanning every tile for changes, the output thread
 *   extracts compositor damage into a per-tile bitmap:
 *
 *     s->dirty_staging:       Written by output thread (no lock needed)
 *     s->dirty_staging_valid: Set when staging has valid damage data
 *     s->dirty_tiles[N]:      Per-send-buffer bitmap (copied under lock)
 *     s->dirty_valid[N]:      Whether bitmap is valid for buffer N
 *
 *   The bitmap size is tiles_x × tiles_y where tiles_x = width/TILE_SIZE
 *   and tiles_y = height/TILE_SIZE, using the padded dimensions.
 *   Tiles in the padding region are never marked dirty by the compositor.
 *
 * Pipelined I/O:
 *
 *   To maximize throughput, writes are pipelined through FreeRDP channel streams:
 *
 *     1. Send thread formats surface update commands
 *     2. rdp_drain_notify() signals drain thread
 *     3. Drain thread processes asynchronous channel events
 *     4. Drain thread signals done_cond after completions
 *     5. Send thread continues with next batch
 *
 * Thread Safety:
 *
 *   s->send_lock protects:
 *     - s->pending_buf, s->active_buf
 *     - s->send_full
 *     - s->resize_pending
 *     - s->framebuf, s->send_buf[] pointer swaps
 *     - s->dirty_tiles[], s->dirty_valid[] (per-buffer bitmaps)
 *     - s->dirty_staging_valid (read and cleared by send_frame)
 */

#ifndef SEND_H
#define SEND_H

#include <stdint.h>

/* Forward declarations */
struct server;

/* Tile size for change detection and compression */
#ifndef TILE_SIZE
#define TILE_SIZE 16
#endif

/* ============== Frame Queueing ============== */

/*
 * Queue a frame for sending.
 *
 * Swaps the current framebuffer pointer (s->framebuf) with a free send
 * buffer and signals the send thread. This is a zero-copy handoff: the
 * send thread gets the just-rendered frame and the compositor gets a
 * recycled buffer for the next frame. Also copies the dirty tile bitmap
 * from staging (s->dirty_staging) into the per-buffer slot so the send
 * thread knows which tiles changed without pixel scanning.
 *
 * Buffer selection:
 *   - Finds a buffer that is neither pending nor active
 *   - If no buffer is free, the frame is dropped (throttling)
 *
 * Flags copied:
 *   - If s->force_full_frame is set, s->send_full is set
 *   - If s->dirty_staging_valid is set, dirty_tiles[buf] is populated
 *
 * This function returns immediately after the swap; actual transmission
 * happens asynchronously in the send thread.
 *
 * Thread-safe: acquires s->send_lock internally.
 *
 * s: server state containing framebuffer and send thread context
 *
 * Preconditions:
 *   - s->framebuf must be valid
 *   - framebuf and send_buf[] must be allocated at the same padded size
 *     (width × height × 4 bytes) since their pointers are swapped
 */
void send_frame(struct server *s);

/* ============== Timer Integration ============== */

/*
 * Timer callback for throttled frame sending.
 *
 * Called by the Wayland event loop at the configured frame rate
 * (typically 30-60 Hz). Checks if there are pending changes and
 * triggers send_frame() if so.
 *
 * This provides frame rate limiting - even if the compositor renders
 * faster, frames are only sent at the timer rate.
 *
 * data: pointer to struct server
 *
 * Returns 0 (Wayland event loop convention for success).
 */
int send_timer_callback(void *data);

/* ============== Send Thread ============== */

/*
 * Send thread main function.
 *
 * Runs in a dedicated thread, processing queued frames. Starts the
 * RDP channel drain thread for asynchronous I/O and initializes
 * the FreeRDP Surface/RFX context.
 *
 * Main loop behavior:
 *
 *   1. Wait for frame via pthread_cond_wait on s->send_cond
 *      (signaled by send_frame and input events on resize)
 *   2. Handle errors and window changes
 *   3. Detect and apply scroll transformations
 *   4. Identify changed tiles by comparing with prev_framebuf
 *   5. Encode tiles via FreeRDP/RFX surface update routines
 *   6. Stream surface updates over the RDP channel
 *   7. Update prev_framebuf for next frame
 *
 * Exit conditions:
 *   - s->running becomes false
 *   - Fatal RDP connection disconnection
 *
 * Cleanup on exit:
 *   - Stops RDP drain thread
 *   - Frees work arrays and buffers
 *
 * arg: pointer to struct server
 *
 * Returns NULL on thread exit.
 */
void *send_thread_func(void *arg);

#endif /* SEND_H */