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

struct elevator elevator;

static int start_elevator(void)
{
    if (elevator.state != OFFLINE)
        return 1;

    elevator.state = IDLE;
    return 0;
}

static int issue_request(int start_floor, int destination_floor, int type)
{
    if (start_floor < MIN_FLOOR || start_floor > MAX_FLOOR)
        return 1;
    if (destination_floor < MIN_FLOOR || destination_floor > MAX_FLOOR)
        return 1;
    if (start_floor == destination_floor)
        return 1;
    if (type < 0 || type >= NUM_PET_TYPES)
        return 1;

    return pets_add_request(start_floor, destination_floor, type);
}

static int stop_elevator(void)
{
    if (elevator.deactivating)
        return 1;
    if (elevator.state != OFFLINE)
        elevator.deactivating = true;
    return 0;
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

    ret = elevator_thread_start();
    if (ret)
        return ret;

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
    pets_free_all();
}

module_init(elevator_init);
module_exit(elevator_exit);
