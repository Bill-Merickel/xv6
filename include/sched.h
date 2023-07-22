#ifndef __INCLUDE_sched_h
#define __INCLUDE_sched_h

extern int setscheduler(int pid, int policy, int prio);

#define SCHED_FIFO 0
#define SCHED_RR 1
// SCHED_OTHER is SCHED_RR in our kernel
#define SCHED_OTHER 1

#endif

