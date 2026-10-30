#include <linux/init.h>
#include <linux/module.h>
#include "elevator.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ricardo Furst, Kyan Carrigan");
MODULE_DESCRIPTION("Pet elevator kernel module");

// stubs defined in the kernel (syscalls/syscalls.c)
extern int (*STUB_start_elevator)(void);
extern int (*STUB_issue_request)(int, int, int);
extern int (*STUB_stop_elevator)(void);

DEFINE_MUTEX(elevator_lock);
struct elevator elevator;

static int start_elevator(void)
{
    int ret = 0;

    mutex_lock(&elevator_lock);
    if (elevator.state != OFFLINE)
        ret = 1;
    else
        elevator.state = IDLE;
    mutex_unlock(&elevator_lock);

    return ret;
}

static int issue_request(int start_floor, int destination_floor, int type)
{
    int ret;

    if (start_floor < MIN_FLOOR || start_floor > MAX_FLOOR)
        return 1;
    if (destination_floor < MIN_FLOOR || destination_floor > MAX_FLOOR)
        return 1;
    if (start_floor == destination_floor)
        return 1;
    if (type < 0 || type >= NUM_PET_TYPES)
        return 1;

    mutex_lock(&elevator_lock);
    ret = pets_add_request(start_floor, destination_floor, type);
    mutex_unlock(&elevator_lock);

    return ret;
}

static int stop_elevator(void)
{
    int ret = 0;

    mutex_lock(&elevator_lock);
    if (elevator.deactivating)
        ret = 1;
    else if (elevator.state != OFFLINE)
        elevator.deactivating = true;
    mutex_unlock(&elevator_lock);

    return ret;
}

static int __init elevator_init(void)
{
    int ret;

    elevator.state = OFFLINE;
    elevator.current_floor = MIN_FLOOR;
    elevator.direction = 1;
    elevator.num_pets = 0;
    elevator.load = 0;
    elevator.deactivating = false;
    pets_init();

    ret = elevator_proc_init();
    if (ret)
        return ret;

    ret = elevator_thread_start();
    if (ret) {
        elevator_proc_exit();
        return ret;
    }

    STUB_start_elevator = start_elevator;
    STUB_issue_request = issue_request;
    STUB_stop_elevator = stop_elevator;
    return 0;
}

static void __exit elevator_exit(void)
{
    STUB_start_elevator = NULL;
    STUB_issue_request = NULL;
    STUB_stop_elevator = NULL;

    elevator_thread_stop();
    elevator_proc_exit();

    mutex_lock(&elevator_lock);
    pets_free_all();
    mutex_unlock(&elevator_lock);
}

module_init(elevator_init);
module_exit(elevator_exit);
