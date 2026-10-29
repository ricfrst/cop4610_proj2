#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/err.h>
#include "elevator.h"

#define MOVE_SECS 2         // time to move one floor
#define LOAD_SECS 1         // time to load/unload at a floor
#define POLL_MS 100         // how often to check for work while offline or idle

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

// basic direction choice: keep going while there is work ahead, otherwise
// turn around; returns 0 when there is nothing left to do
// (scheduling policy to be finalized in part 3f)
static int next_direction(void)
{
    if (work_in_direction(elevator.direction))
        return elevator.direction;
    if (work_in_direction(-elevator.direction))
        return -elevator.direction;
    return 0;
}

static int elevator_run(void *data)
{
    int dir;

    while (!kthread_should_stop()) {
        if (elevator.state == OFFLINE) {
            msleep(POLL_MS);
            continue;
        }

        // stop at this floor if anyone is getting off or can get on
        if (pets_to_unload() || (!elevator.deactivating && pets_can_board())) {
            elevator.state = LOADING;
            ssleep(LOAD_SECS);
            pets_unload();
            if (!elevator.deactivating)
                pets_load();
        }

        // stop_elevator finishes once every pet on board is dropped off
        if (elevator.deactivating && elevator.num_pets == 0) {
            elevator.deactivating = false;
            elevator.state = OFFLINE;
            continue;
        }

        dir = next_direction();
        if (dir == 0) {
            elevator.state = IDLE;
            msleep(POLL_MS);
            continue;
        }

        // the elevator only ever moves one floor at a time
        elevator.direction = dir;
        elevator.state = (dir > 0) ? UP : DOWN;
        ssleep(MOVE_SECS);
        elevator.current_floor += dir;
    }

    return 0;
}

int elevator_thread_start(void)
{
    elevator.kthread = kthread_run(elevator_run, NULL, "elevator");
    if (IS_ERR(elevator.kthread))
        return PTR_ERR(elevator.kthread);
    return 0;
}

void elevator_thread_stop(void)
{
    kthread_stop(elevator.kthread);
}
