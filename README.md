# Elevator

A Linux kernel project in three parts: tracing system calls with `strace`, a `my_timer` kernel
module that reports the time through `/proc/timer`, and a pet elevator kernel module controlled by
three new system calls (`start_elevator`, `issue_request`, `stop_elevator`) added to Linux 6.16.12.
The elevator runs in a kthread, stores pets in linked lists, protects shared data with a mutex, and
reports its status through `/proc/elevator`.

## Group Members
- **Ricardo Furst**: rcf22b@fsu.edu
- **Kyan Carrigan**: kmc22t@fsu.edu

## Division of Labor

### Part 1: System Call Tracing
- **Responsibilities**: Write an empty C program and a copy with exactly five added system
  calls, then use `strace` to produce and compare `empty.trace` and `part1.trace`.
- **Assigned to**: Ricardo Furst and Kyan Carrigan

### Part 2: Timer Kernel Module
- **Responsibilities**: Write the `my_timer` kernel module that creates `/proc/timer` and uses
  `ktime_get_real_ts64()` to print the current time and the time elapsed since the last read.
- **Assigned to**: Ricardo Furst and Kyan Carrigan

### Part 3a: Adding System Calls
- **Responsibilities**: Add `start_elevator` (548), `issue_request` (549), and `stop_elevator`
  (550) to the kernel source, with stubs the elevator module hooks into when loaded.
- **Assigned to**: Ricardo Furst and Kyan Carrigan

### Part 3b: Kernel Compilation
- **Responsibilities**: Configure, compile, and install Linux kernel 6.16.x with the new system
  calls, then verify them with the provided system-call tests.
- **Assigned to**: Ricardo Furst and Kyan Carrigan

### Part 3c: Threads
- **Responsibilities**: Use a kthread to run the elevator loop: moving between floors, loading
  and unloading pets, and stopping cleanly when the module is removed.
- **Assigned to**: Ricardo Furst and Kyan Carrigan

### Part 3d: Linked List
- **Responsibilities**: Store the pets waiting on each floor and the pets on the elevator in
  linked lists, allocating each pet with `kmalloc` and freeing it when it is serviced.
- **Assigned to**: Ricardo Furst and Kyan Carrigan

### Part 3e: Mutexes
- **Responsibilities**: Protect the elevator and floor data shared by the kthread, the system
  calls, and `/proc/elevator` with a mutex to prevent race conditions.
- **Assigned to**: Ricardo Furst and Kyan Carrigan

### Part 3f: Scheduling Algorithm
- **Responsibilities**: Decide which direction the elevator moves and which pets board,
  following the 5-pet / 50 lb limit, FIFO boarding, and the required move/load delays.
- **Assigned to**: Ricardo Furst and Kyan Carrigan

## File Listing
```
cop4610_proj2/
├── Makefile
├── README.md
├── part1/
│   ├── empty.c
│   ├── empty.trace
│   ├── part1.c
│   ├── part1.trace
│   └── Makefile
├── part2/
│   ├── src/
│   │   └── my_timer.c
│   └── Makefile
└── part3/
    ├── src/
    │   ├── elevator.h
    │   ├── elevator_module.c
    │   ├── elevator_thread.c
    │   ├── elevator_proc.c
    │   ├── pets.c
    │   └── scheduler.c
    ├── tests/
    ├── syscalls.c
    ├── kernel_setup.sh
    └── Makefile
```

# How to Compile & Execute

### Requirements
- **Compiler**: `gcc` and `make` (`sudo apt install build-essential`)
- **Kernel**: Linux 6.16.12 built with the system calls in `part3/syscalls.c` (see Part 3)
- **Tools**: `strace` for Part 1 and kernel headers for the running kernel for Parts 2 and 3

Running `make` in the top-level directory builds all three partsand `make clean` removes the
build output.

## Part 1

### Compilation
```bash
cd part1
make
```
This builds the `empty` and `part1` executables in `part1/`.
### Execution
```bash
make trace
```
This runs both programs under `strace` and writes `empty.trace` and `part1.trace`.
`part1.trace` contains exactly five more system calls than `empty.trace`.

## Part 2

### Compilation
```bash
cd part2
make
```
This builds the kernel module `my_timer.ko` in `part2/`.
### Execution
```bash
sudo insmod my_timer.ko
cat /proc/timer
sleep 1
cat /proc/timer
sudo rmmod my_timer
```
Each read of `/proc/timer` prints the current time and, after the first read, the time elapsed
since the previous read. Removing the module removes `/proc/timer`.

## Part 3

### Kernel (system calls)
```bash
bash part3/kernel_setup.sh ~/linux-6.16.12
cd ~/linux-6.16.12
make menuconfig
make -j$(nproc)
sudo make modules_install INSTALL_MOD_STRIP=1
sudo make install
sudo reboot
```
`kernel_setup.sh` adds `syscalls/syscalls.c` and `syscalls/Makefile` to the kernel source and
updates `syscall_64.tbl`, `include/linux/syscalls.h`, and the top-level `Makefile`. To check the
system calls after rebooting into 6.16.12, run `make`, `sudo insmod syscheck.ko`, and `make run` in
`part3/tests/system-calls-test`.

### Compilation
```bash
cd part3
make
```
This builds the kernel module `elevator.ko` in `part3/`.
### Execution
```bash
sudo insmod elevator.ko
cd tests/elevator-test && make
./consumer --start
./producer 20
watch -n1 cat /proc/elevator
./consumer --stop
cd ../.. && sudo rmmod elevator
```
`consumer --start` / `--stop` start and stop the elevator, `producer N` issues N random pet
requests, and `/proc/elevator` shows the elevator state, current floor, load, pets on board, the
pets waiting on each floor, and the waiting and serviced totals.

## Development Log
Each member records their contributions here.

### Ricardo Furst

| Date       | Work Completed / Notes |
|------------|------------------------|
| 2026-10-20 | Part 1: wrote `empty.c` and `part1.c` +Makefile with `trace` target that runs `strace` on both programs. 
| 2026-10-20 | Part 2: wrote the `my_timer` module and  `/proc/timer` prints the current time and the elapsed time since the previous read. |
| 2026-10-25 | Part 3a: wrote `syscalls.c` with system calls 548-550 and their module stubs, plus `kernel_setup.sh` to add them to the 6.16 kernel source and prepare the build config. |
| 2026-10-25 | Part 3c: added the elevator kthread that moves one floor at a time (2 s per floor, 1 s per load/unload stop) and handles start/stop. |
| 2026-10-29 | Part 3d: added the linked lists for pets waiting on each floor and pets on the elevator, with `kmalloc`/`kfree` for each pet and FIFO boarding under the 5-pet / 50 lb limits. |
| 2026-10-29 | Part 3b: built and installed kernel 6.16.12 with the new system calls on the Proxmox VM and confirmed with `uname -r` and the provided system-call test (all three calls exist). |
| 2026-10-29 | Tested Part 1 (`part1.trace` has exactly 5 more system calls than `empty.trace`) and Part 2 (`/proc/timer` elapsed times of ~1 s and ~3 s and entry removed on `rmmod`). |
| 2026-10-29 | Tested the elevator module with the provided producer/consumer: kthread runs, start/stop/issue_request return the expected values, and the module unloads cleanly with no kernel errors. |
| 2026-10-30 | Part 3e: added a mutex around the elevator, floor queues, and counters, shared by the kthread, the system calls, and the proc file the kthread releases it before each sleep. |
| 2026-10-30 | Added `/proc/elevator` showing the elevator state, floor, load, pets on board, each floor's waiting pets, and the waiting/serviced totals. |
| 2026-10-30 | Part  3f: moved scheduling into `scheduler.c` (LOOK: continue while work is ahead, then reverse idle when no work).

### Kyan Carrigan

| Date       | Work Completed / Notes |
|------------|------------------------|
| YYYY-MM-DD | [Description of task]  |
| YYYY-MM-DD | [Description of task]  |
| YYYY-MM-DD | [Description of task]  |


## Meetings
Meetings were held as Discord calls, covering what was done so far and how to approach the next parts.

| Date       | Attendees                      | Topics Discussed | Outcomes / Decisions |
|------------|--------------------------------|------------------|-----------------------|
| 2026-10-26 | Ricardo Furst, Kyan Carrigan   | How to approach Part 1 (system-call tracing), Part 2 (timer module), and Part 3a (adding the system calls) | Plan for writing Parts 1, 2, and 3a |
| 2026-10-28 | Ricardo Furst, Kyan Carrigan   | How to test on vm and how project would proceed with both of us unavailable over the weekend. |
| 2026-10-30 | Ricardo Furst, Kyan Carrigan   | How to approach rest of project, specifically Part 3d (linked lists), Part 3e (mutex), and Part 3f (scheduling) | Plan for finishing the elevator module and testing it |

## Bugs
- **Bug 1 (fixed)**: Running `make` in `part3/tests/system-calls-test` did not produce
  `syscheck.ko` modpost failed with `"STUB_start_elevator" [syscheck.ko] undefined!` (and the same
  for the other two stubs), so `insmod` then failed with "No such file or directory". The VM was
  still booted into the stock Ubuntu 7.0 kernel, which does not define the stubs.
- **Bug 2 (fixed)**: Preparing the 6.16.12 kernel build failed with `Too many levels of symbolic
  links`. The source was linked with a relative path (`ln -s linux-6.16.12 ~/linux-6.16.12`), so the
  link pointed to itself.

## Considerations
- **Scheduling**: the elevator uses LOOK. It keeps moving in its current direction while any floor
  ahead has work (a pet on board going there, or pets waiting there), then turns around, and goes
  idle when there is no work. Every waiting pet is reached within one sweep, so none wait forever.
- **FIFO boarding**: only the first pet in a floor's queue may board and if it would exceed 5 pets or
  50 lbs, the pets behind it wait too.
- **Locking**: one mutex protects the elevator, the floor queues, and the counters. The kthread
  releases it before every sleep, so system calls and `/proc/elevator` reads never wait on the 1 s
  or 2 s delays.
- **Stopping**: `stop_elevator` stops picking up waiting pets and the elevator drops off everyone on
  board and then goes `OFFLINE`. Waiting pets stay queued until the elevator is started again.
- **Invalid requests**: floors outside 1-5, a start floor equal to the destination, or a pet type
  outside 0-3 make `issue_request` return 1 without creating a pet.
- **Module removal**: `rmmod` stops the kthread and frees every remaining pet.
