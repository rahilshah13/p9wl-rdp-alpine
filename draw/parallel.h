/* parallel.h - Simple parallel for loop with persistent workers */

#ifndef PARALLEL_H
#define PARALLEL_H

/* Maximum number of worker threads */
#define MAX_WORKERS 8

/* Function signature for parallel work items */
typedef void (*parallel_fn)(void *ctx, int idx);

/* Execute a function in parallel for indices 0 to count-1 */
void parallel_for(int count, parallel_fn fn, void *ctx);

/* Shutdown the thread pool and release resources */
void parallel_cleanup(void);

#endif /* PARALLEL_H */