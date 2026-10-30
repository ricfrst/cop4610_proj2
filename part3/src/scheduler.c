#include "elevator.h"

/*
 * Scheduling policy (LOOK):
 * - Stop at a floor if a pet on board gets off there, or if the first pet
 *   waiting there fits (pets board FIFO, so nobody skips the line).
 * - Keep moving in the current direction while any floor ahead has work,
 *   then turn around. Go idle when no floor has work.
 * - While stopping, waiting pets are ignored so only pets on board are served.
 * Every floor with work is reached within one sweep, so no pet waits forever.
 */

// true if the elevator should stop at the current floor to load/unload
bool scheduler_should_stop(void)
{
    if (pets_to_unload())
        return true;
    return !elevator.deactivating && pets_can_board();
}

// true if any floor past the current one in direction dir needs a visit
static bool work_in_direction(int dir)
{
    int f;

    for (f = elevator.current_floor + dir; f >= MIN_FLOOR && f <= MAX_FLOOR; f += dir) {
        if (pets_work_on_floor(f))
            return true;
    }
    return false;
}

// returns 1 to move up, -1 to move down, or 0 if there is nothing to do
int scheduler_next_direction(void)
{
    if (work_in_direction(elevator.direction))
        return elevator.direction;
    if (work_in_direction(-elevator.direction))
        return -elevator.direction;
    return 0;
}
