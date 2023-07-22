# Scheduling and Threading

Here, I made two primary notifications to the xv6 kernel. First,
I extended xv6's scheduler to support multiple new schedulers.
Second, I constructed a kernel-space threading library.

## Scheduling

#### Background
The xv6 scheduler is found in `kernel/src/proc.c`, in the `scheduler()` function, and by
default implements a round robin (RR) scheduler. After each CPU is setup, all
eventually reach `mpmain()`, where `scheduler()` is called for the first time.
`scheduler()` loops over the process table in order looking for `RUNNABLE`
processes. When a `RUNNABLE` process is found, the kernel switches to that
process. It executes until it finishes or until a timer tick interrupts it and
causes the process to `yield()`.

The timer is a hardware interrupt. The code for handling this is
found in `kernel/src/trap.c`, around line 105.

```
// Force process to give up CPU on clock tick.
// If interrupts were on while locks held, would need to check nlock.
if (proc && proc->state == RUNNING && tf->trapno == T_IRQ0+IRQ_TIMER)
  yield();
```

`yield()` sets the current process to `RUNNABLE` and then switches directly to the
kernel scheduler.

I enabled the user-space to specify
their scheduler policy by:

- Enabling the user-space to select their scheduling policy (`SCHED_RR`, or
  `SCHED_FIFO`)
- Enabling the user-space process to set a priority.
- Implementing two new schedulers: Round Robin with Priority and FIFO with Priority


#### Added System Call

To enable the user-space to set their scheduler policy, I also added one
system-call to the kernel:

```
int setscheduler(int pid, int policy, int priority);

Arguments:
pid - the pid of the process to change priority (a process may only change the
scheduler of themselves, or their direct children)
policy - the scheduler policy (SCHED_RR, SCHED_FIFO)
priority - the priority value to be set (any non-negative int value is
legal)

setscheduler should be declared in a program by including "user.h" and
should be defined within the user-space ulib library (ulib_SOURCES
in user/Sources.cmake).
```

A user-space program should also be able to use the macros `SCHED_RR` and
`SCHED_FIFO` by including the file `include/sched.h` (typically through the
pre-processor directive `#include "sched.h"`).

## Kernel Threading

I also built a kernel threading library.  Like
Linux, I treated threads as processes, however they share memory with
their neighboring threads.  Additionally, I provided kernel support to
help build threading primitives on top of the threading library.

### The Kernel Threading

The threading library has two parts, a kernel implementation and a
user-space implementation. 

First, I created a new thread (a process that shares address
space with its parent).  For this lab, we'll be accomplishing this with our
version of the classic system call `clone()`:

```
int clone(void *stack, int stack_size)

Arguments:
  stack -- a pointer to the beginning of a memory region of size stack_size, to
           be used as the new thread's stack
  stack_size -- the size of the new thread's stack in bytes

Return:
  As with fork, clone returns twice on success (in the child and the parent).
  The returned values are:
    Parent - the pid of the child.
    Child - 0

  On error clone returns exactly once, with the value -1 (no child is created).

Behavior:
  On success clone creates a new process which shares its address space with its
  parent.  Additionally, clone sets up the child's stack to be logically
  equivalent to the parent's stack.  On clone the child's register state is
  equivalent to that of the parent, with the exception of registers used for 
  the return value from clone (recall eax is the return value of a system call), 
  or holding information about the stack.
```

`clone()` creates a new process, and adds it to the caller's "thread group".
Processes within a "thread group" all share the same address space.  Thread
groups are created via either the `fork` or `exec` system calls.  The first
process within a thread group (the `fork`d or `exec`d process) is the thread
group's owner.  If the owner of a thread group terminates before the other
threads in the group, the behavior for those threads is undefined.

Clone should additionally follow these rules:

- Clone should fail cleanly on errors. If clone cannot run (for instance, if its
  passed a stack that's too small), it should return with an error.

- Cloned processes share several resources with their parent, namely:
   - Virtual address space (shared memory)
   - File descriptor table
   - Current working directory

When any thread makes a change to a shared resource (such as writing to memory,
allocating new memory, or changing the directory) that change should be visible
to all threads in that thread group. 

### The Thread Library

I also created a (very simple) user-space threading
library to accompany the kernel `clone` functionality.  The library will include two funcitons:

```c
Function: thread_create
Arguments:
  - start_routine -- A function pointer to the routine that the child thread will run
  - arg -- the argument passed to start_routine
Return Value:
  - -1 on failure, pid of the created thread on success
Description:
Creates a new child thread.  That thread will immediately begin running start_routine,
as though invoked with start_routine(arg).

Definition:
int thread_create(void *(*start_routine)(void *), void *arg);


Function: thread_wait
Return Value:
  - -1 on failure, pid of the joined thread on success
Description:
Waits for a child thread to finish.

Definition:
int thread_wait();
```

These functions are declared in `user/include/user.h`. 
