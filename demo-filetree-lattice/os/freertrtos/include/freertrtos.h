#ifndef FREERTRTOS_FREERTRTOS_H
#define FREERTRTOS_FREERTRTOS_H

/**
 * Umbrella header for the freertrtos adaptation layer.
 *
 * Flight code should include the specific header it needs instead of this
 * umbrella, but this is convenient for board and test code.
 */

#include <freertrtos/critical.h>
#include <freertrtos/fault.h>
#include <freertrtos/memory.h>
#include <freertrtos/queue.h>
#include <freertrtos/semphr.h>
#include <freertrtos/task.h>
#include <freertrtos/time.h>

/** Initialise board-independent kernel hooks. Returns 0 on success. */
int frt_init(void);

/** Start the scheduler. Does not return on success. */
void frt_start_scheduler(void);

#endif /* FREERTRTOS_FREERTRTOS_H */
