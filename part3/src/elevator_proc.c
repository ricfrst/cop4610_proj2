#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include "elevator.h"

#define ENTRY_NAME "elevator"
#define PERMS 0444
#define PARENT NULL
#define BUF_SIZE 10000

static struct proc_dir_entry* elevator_entry;

static char buf[BUF_SIZE];
static int len = 0;

static const char *state_names[] = {
    [OFFLINE] = "OFFLINE",
    [IDLE]    = "IDLE",
    [LOADING] = "LOADING",
    [UP]      = "UP",
    [DOWN]    = "DOWN",
};

// fill buf with the current elevator status; caller holds elevator_lock
static void build_status(void)
{
    struct pet *p;
    int f;

    len = 0;
    len += scnprintf(buf + len, BUF_SIZE - len, "Elevator state: %s\n",
                     state_names[elevator.state]);
    len += scnprintf(buf + len, BUF_SIZE - len, "Current floor: %d\n",
                     elevator.current_floor);
    len += scnprintf(buf + len, BUF_SIZE - len, "Current load: %d lbs\n", elevator.load);

    len += scnprintf(buf + len, BUF_SIZE - len, "Elevator status:");
    list_for_each_entry(p, &elevator.pets, list) {
        len += scnprintf(buf + len, BUF_SIZE - len, " %c%d",
                         pets_type_char(p->type), p->dest_floor);
    }
    len += scnprintf(buf + len, BUF_SIZE - len, "\n\n");

    for (f = MAX_FLOOR; f >= MIN_FLOOR; f--) {
        len += scnprintf(buf + len, BUF_SIZE - len, "[%c] Floor %d: %d",
                         (f == elevator.current_floor) ? '*' : ' ',
                         f, FLOOR(f)->num_waiting);
        list_for_each_entry(p, &FLOOR(f)->waiting, list) {
            len += scnprintf(buf + len, BUF_SIZE - len, " %c%d",
                             pets_type_char(p->type), p->dest_floor);
        }
        len += scnprintf(buf + len, BUF_SIZE - len, "\n");
    }

    len += scnprintf(buf + len, BUF_SIZE - len, "\nNumber of pets: %d\n", elevator.num_pets);
    len += scnprintf(buf + len, BUF_SIZE - len, "Number of pets waiting: %d\n",
                     pets_waiting_total());
    len += scnprintf(buf + len, BUF_SIZE - len, "Number of pets serviced: %d\n",
                     pets_serviced);
}

static ssize_t elevator_read(struct file *file, char __user *ubuf, size_t count, loff_t *ppos)
{
    ssize_t ret;

    mutex_lock(&elevator_lock);

    // cat keeps calling read until it gets 0 bytes, so only build the
    // status on the first call of each read
    if (*ppos == 0)
        build_status();

    ret = simple_read_from_buffer(ubuf, count, ppos, buf, len);

    mutex_unlock(&elevator_lock);
    return ret;
}

static const struct proc_ops elevator_fops = {
    .proc_read = elevator_read,
};

int elevator_proc_init(void)
{
    elevator_entry = proc_create(ENTRY_NAME, PERMS, PARENT, &elevator_fops);
    if (!elevator_entry)
        return -ENOMEM;
    return 0;
}

void elevator_proc_exit(void)
{
    proc_remove(elevator_entry);
}
