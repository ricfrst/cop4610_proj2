# Elevator

[Description]

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
elevator/
├── Makefile
├── part1/
│   ├── empty.c
│   ├── empty.trace
│   ├── part1.c
│   ├── part1.trace
│   └── Makefile
├── part2/
│   ├── src/
│   └── Makefile
├── part3/
│   ├── src/
│   ├── tests/
│   ├── Makefile
│   └── sys_call.c
├── Makefile
└── README.md

```
# How to Compile & Execute

### Requirements
- **Compiler**: e.g., `gcc` for C/C++, `rustc` for Rust.
- **Dependencies**: List any libraries or frameworks necessary (rust only).

## Part 1

### Compilation
For a C/C++ example:
```bash
make
```
This will build the executable in ...
### Execution
```bash
make run
```
This will run the program ...

## Part 2

### Compilation
For a C/C++ example:
```bash
make
```
This will build the executable in ...
### Execution
```bash
make run
```
This will run the program ...


## Part 3

### Compilation
For a C/C++ example:
```bash
make
```
This will build the executable in ...
### Execution
```bash
make run
```
This will run the program ...

## Development Log
Each member records their contributions here.

### [Member 1]

| Date       | Work Completed / Notes |
|------------|------------------------|
| YYYY-MM-DD | [Description of task]  |
| YYYY-MM-DD | [Description of task]  |
| YYYY-MM-DD | [Description of task]  |

### [Member 2]

| Date       | Work Completed / Notes |
|------------|------------------------|
| YYYY-MM-DD | [Description of task]  |
| YYYY-MM-DD | [Description of task]  |
| YYYY-MM-DD | [Description of task]  |


### [Member 3]

| Date       | Work Completed / Notes |
|------------|------------------------|
| YYYY-MM-DD | [Description of task]  |
| YYYY-MM-DD | [Description of task]  |
| YYYY-MM-DD | [Description of task]  |


## Meetings
Document in-person meetings, their purpose, and what was discussed.

| Date       | Attendees            | Topics Discussed | Outcomes / Decisions |
|------------|----------------------|------------------|-----------------------|
| YYYY-MM-DD | [Names]              | [Agenda items]   | [Actions/Next steps]  |
| YYYY-MM-DD | [Names]              | [Agenda items]   | [Actions/Next steps]  |
| YYYY-MM-DD | [Names]              | [Agenda items]   | [Actions/Next steps]  |



## Bugs
- **Bug 1**: This is bug 1.
- **Bug 2**: This is bug 2.
- **Bug 3**: This is bug 3.

## Considerations
