/*
 * clipboard.h - FreeRDP cliprdr <-> Plan 9 /dev/snarf integration
 *
 * Provides bidirectional clipboard synchronization:
 *   - FreeRDP client copies -> writes to /dev/snarf
 *   - FreeRDP client pastes -> reads from /dev/snarf
 *   - Plan 9 snarf changes  -> detected via qid.vers polling and pushed to RDP
 *
 * Design:
 *
 *   Snarf is treated as the single source of truth for
 *   clipboard contents.
 *
 *   Maximum clipboard size: 1MB (SNARF_MAX_SIZE)
 *
 * FreeRDP -> Snarf (Copy):
 *
 *   When the FreeRDP client copies text:
 *     1. rdp_to_snarf_write() handler receives client data
 *     2. Data written to /dev/snarf via p9_write_file()
 *
 * Snarf -> FreeRDP (Paste / Push):
 *
 *   Snarf Version Polling:
 *
 *   Rio's /dev/snarf exposes a version counter via qid.vers that
 *   increments on each write. A dedicated thread polls this with
 *   Tstat every 500ms to detect Plan 9-side clipboard changes
 *   (e.g., user copies text in a rio window). When a change is
 *   detected, the background thread reads fresh from /dev/snarf and
 *   updates the FreeRDP clipboard via freerdp_clipboard_update().
 *
 *   The blocking Tstat and read RPCs run entirely in the poll thread,
 *   so the main event loop is never stalled by 9P I/O.
 *
 * Usage:
 *
 *   Initialize during server setup (after p9_snarf and cliprdr are ready):
 *
 *     if (clipboard_init(server) < 0) {
 *         // handle error
 *     }
 *
 *   Clean up during shutdown:
 *
 *     clipboard_cleanup(server);
 */

#ifndef P9WL_CLIPBOARD_H
#define P9WL_CLIPBOARD_H

#include <stddef.h>
#include <stdint.h>

struct server;

/* ============== Initialization ============== */

/*
 * Initialize clipboard handling.
 *
 * Sets up:
 *   - Snarf version polling thread (500ms interval) to detect Plan 9 clipboard changes
 *
 * The snarf poll walks to /dev/snarf once (without opening) and
 * keeps the fid for periodic Tstat calls in a background thread.
 * If the initial stat fails, polling is disabled.
 *
 * s: server instance (must have p9_snarf initialized)
 *
 * Returns 0 on success, -1 on failure.
 */
int clipboard_init(struct server *s);

/*
 * Write clipboard data received from the FreeRDP client into Plan 9 /dev/snarf.
 *
 * s:    server instance
 * data: pointer to clipboard bytes
 * size: number of bytes
 */
void rdp_to_snarf_write(struct server *s, const void *data, size_t size);

/*
 * Clean up clipboard resources.
 *
 * Stops the snarf polling thread and clunks the stat fid.
 *
 * s: server instance
 */
void clipboard_cleanup(struct server *s);

#endif /* P9WL_CLIPBOARD_H */