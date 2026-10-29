#include <linux/slab.h>
#include "elevator.h"

// weight in lbs, indexed by pet type
static const int pet_weights[NUM_PET_TYPES] = {
    [CHIHUAHUA] = 3,
    [PUG]       = 14,
    [PUGHUAHUA] = 10,
    [DACHSHUND] = 16,
};

struct floor floors[NUM_FLOORS];
int pets_serviced = 0;

void pets_init(void)
{
    int i;

    for (i = 0; i < NUM_FLOORS; i++) {
        floors[i].num_waiting = 0;
        INIT_LIST_HEAD(&floors[i].waiting);
    }
    INIT_LIST_HEAD(&elevator.pets);
    pets_serviced = 0;
}

// allocate a pet and add it to the back of its start floor's queue
int pets_add_request(int start_floor, int dest_floor, int type)
{
    struct pet *p;

    p = kmalloc(sizeof(struct pet), GFP_KERNEL);
    if (p == NULL)
        return -ENOMEM;

    p->type = type;
    p->weight = pet_weights[type];
    p->start_floor = start_floor;
    p->dest_floor = dest_floor;

    list_add_tail(&p->list, &FLOOR(start_floor)->waiting);
    FLOOR(start_floor)->num_waiting++;

    return 0;
}

// true if any pet on board wants to get off at the current floor
bool pets_to_unload(void)
{
    struct pet *p;

    list_for_each_entry(p, &elevator.pets, list) {
        if (p->dest_floor == elevator.current_floor)
            return true;
    }
    return false;
}

// true if the first pet waiting on the current floor fits on the elevator;
// pets board in FIFO order, so later pets never skip ahead of the first one
bool pets_can_board(void)
{
    struct floor *f = FLOOR(elevator.current_floor);
    struct pet *first;

    if (list_empty(&f->waiting) || elevator.num_pets >= MAX_PETS)
        return false;

    first = list_first_entry(&f->waiting, struct pet, list);
    return elevator.load + first->weight <= MAX_WEIGHT;
}

// remove and free every pet on board whose destination is the current floor
void pets_unload(void)
{
    struct pet *p;
    struct pet *tmp;

    list_for_each_entry_safe(p, tmp, &elevator.pets, list) {
        if (p->dest_floor == elevator.current_floor) {
            list_del(&p->list);
            elevator.num_pets--;
            elevator.load -= p->weight;
            pets_serviced++;
            kfree(p);
        }
    }
}

// move pets from the current floor's queue onto the elevator while they fit
void pets_load(void)
{
    struct floor *f = FLOOR(elevator.current_floor);
    struct pet *p;

    while (pets_can_board()) {
        p = list_first_entry(&f->waiting, struct pet, list);
        list_move_tail(&p->list, &elevator.pets);
        f->num_waiting--;
        elevator.num_pets++;
        elevator.load += p->weight;
    }
}

// true if the elevator has a reason to visit this floor: a pet on board is
// going there, or (unless the elevator is stopping) pets are waiting there
bool pets_work_on_floor(int floor)
{
    struct pet *p;

    list_for_each_entry(p, &elevator.pets, list) {
        if (p->dest_floor == floor)
            return true;
    }
    return !elevator.deactivating && FLOOR(floor)->num_waiting > 0;
}

int pets_waiting_total(void)
{
    int i;
    int total = 0;

    for (i = 0; i < NUM_FLOORS; i++)
        total += floors[i].num_waiting;
    return total;
}

static void free_pet_list(struct list_head *head)
{
    struct pet *p;
    struct pet *tmp;

    list_for_each_entry_safe(p, tmp, head, list) {
        list_del(&p->list);
        kfree(p);
    }
}

// free every pet on every floor and on the elevator (module unload)
void pets_free_all(void)
{
    int i;

    for (i = 0; i < NUM_FLOORS; i++) {
        free_pet_list(&floors[i].waiting);
        floors[i].num_waiting = 0;
    }
    free_pet_list(&elevator.pets);
    elevator.num_pets = 0;
    elevator.load = 0;
}
