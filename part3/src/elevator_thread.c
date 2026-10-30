#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/err.h>
#include "elevator.h"

#define MOVE_SECS 2         // time to move one floor
#define LOAD_SECS 1         // time to load/unload at a floor
#define POLL_MS 100         // how often to check for work while offline or idle

/*
 * Main elevator loop. The lock is held while reading or changing shared data
 * and released before every sleep, so system calls and proc reads are never
 * blocked while the elevator is moving or loading.
 */
static int elevator_run(void *data)
{
    int dir;

    while (!kthread_should_stop()) {
        mutex_lock(&elevator_lock);

        if (elevator.state == OFFLINE) {
            mutex_unlock(&elevator_lock);
            msleep(POLL_MS);
            continue;
        }

        if (scheduler_should_stop()) {
            elevator.state = LOADING;
            mutex_unlock(&elevator_lock);
            ssleep(LOAD_SECS);
            mutex_lock(&elevator_lock);

            pets_unload();
            if (!elevator.deactivating)
                pets_load();
        }

        // stop_elevator finishes once every pet on board is dropped off
        if (elevator.deactivating && elevator.num_pets == 0) {
            elevator.deactivating = false;
            elevator.state = OFFLINE;
            mutex_unlock(&elevator_lock);
            continue;
        }

        dir = scheduler_next_direction();
        if (dir == 0) {
            elevator.state = IDLE;
            mutex_unlock(&elevator_lock);
            msleep(POLL_MS);
            continue;
        }

        // the elevator only ever moves one floor at a time
        elevator.direction = dir;
        elevator.state = (dir > 0) ? UP : DOWN;
        mutex_unlock(&elevator_lock);
        ssleep(MOVE_SECS);

        mutex_lock(&elevator_lock);
        elevator.current_floor += dir;
        mutex_unlock(&elevator_lock);
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
