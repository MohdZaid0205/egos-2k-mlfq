#pragma once

#include "egos.h"
#include "syscall.h"
#include <sys/queue.h>

enum proc_status {
    PROC_LOADING,
    PROC_READY,
    PROC_RUNNING,
    PROC_RUNNABLE,
    PROC_PENDING_SYSCALL
};

struct process {
    int pid;
    struct syscall syscall;
    enum proc_status status;
    uint mepc, saved_registers[32];
    /* Student's code goes here (Preemptive Scheduler | System Call). */

    /* Add new fields for lifecycle statistics, MLFQ or process sleep. */

    /* Student's code ends here. */
    SLIST_ENTRY(process) next; /* Next pointer for the singly-linked list. */
};

ulonglong mtime_get();

int proc_alloc();
void proc_free(int);
void proc_set_ready(int);
void proc_set_runnable(int);

void mlfq_reset_level();
void mlfq_update_level(struct process* p, ulonglong runtime);
void proc_sleep(int pid, uint usec);
void proc_coresinfo();
