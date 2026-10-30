#ifndef ELEVATOR_H
#define ELEVATOR_H

#include <linux/types.h>
#include <linux/list.h>
#include <linux/sched.h>
#include <linux/mutex.h>

#define MIN_FLOOR 1
#define MAX_FLOOR 5
#define NUM_FLOORS (MAX_FLOOR - MIN_FLOOR + 1)

#define MAX_PETS 5
#define MAX_WEIGHT 50       // lbs
#define NUM_PET_TYPES 4

enum pet_type { CHIHUAHUA, PUG, PUGHUAHUA, DACHSHUND };
enum elevator_state { OFFLINE, IDLE, LOADING, UP, DOWN };

struct pet {
    int type;
    int weight;
    int start_floor;
    int dest_floor;
    struct list_head list;
};

struct floor {
    int num_waiting;
    struct list_head waiting;   // pets waiting on this floor, in arrival order
};

struct elevator {
    int state;
    int current_floor;
    int direction;              // last direction moved: 1 = up, -1 = down
    int num_pets;
    int load;                   // total weight of pets on board, in lbs
    bool deactivating;          // set by stop_elevator until the elevator is empty
    struct list_head pets;      // pets on board
    struct task_struct *kthread;
};

/*
 * elevator_lock protects everything below it: the elevator, every floor, and
 * pets_serviced. It is shared by the kthread, the system calls, and the proc
 * file. Functions in pets.c and scheduler.c expect the caller to hold it.
 */
extern struct mutex elevator_lock;
extern struct elevator elevator;
extern struct floor floors[NUM_FLOORS];
extern int pets_serviced;

// floors[] is indexed from 0, floor numbers start at MIN_FLOOR
#define FLOOR(n) (&floors[(n) - MIN_FLOOR])

/* pets.c - linked lists of pets on each floor and on the elevator */
void pets_init(void);
int pets_add_request(int start_floor, int dest_floor, int type);
bool pets_to_unload(void);
bool pets_can_board(void);
void pets_unload(void);
void pets_load(void);
bool pets_work_on_floor(int floor);
int pets_waiting_total(void);
char pets_type_char(int type);
void pets_free_all(void);

/* scheduler.c - decides where the elevator stops and which way it moves */
bool scheduler_should_stop(void);
int scheduler_next_direction(void);

/* elevator_thread.c - kthread that moves the elevator */
int elevator_thread_start(void);
void elevator_thread_stop(void);

/* elevator_proc.c - /proc/elevator */
int elevator_proc_init(void);
void elevator_proc_exit(void);

#endif
