#include <linux/linkage.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/syscalls.h>

// sys call stubs, set by the elevator module when it is loaded
int (*STUB_start_elevator)(void) = NULL;
int (*STUB_issue_request)(int, int, int) = NULL;
int (*STUB_stop_elevator)(void) = NULL;
EXPORT_SYMBOL(STUB_start_elevator);
EXPORT_SYMBOL(STUB_issue_request);
EXPORT_SYMBOL(STUB_stop_elevator);

// sys call wrappers (548, 549, 550)
SYSCALL_DEFINE0(start_elevator) {
    if (STUB_start_elevator != NULL)
        return STUB_start_elevator();
    else
        return -ENOSYS;
}

SYSCALL_DEFINE3(issue_request, int, start_floor, int, destination_floor, int, type) {
    if (STUB_issue_request != NULL)
        return STUB_issue_request(start_floor, destination_floor, type);
    else
        return -ENOSYS;
}

SYSCALL_DEFINE0(stop_elevator) {
    if (STUB_stop_elevator != NULL)
        return STUB_stop_elevator();
    else
        return -ENOSYS;
}
