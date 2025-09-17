/*
 * (C) 2025, Cornell University
 * All rights reserved.
 *
 * Description: helper functions for process management
 */

#include "process.h"
#include <stdlib.h>

#define MLFQ_NLEVELS          5
#define MLFQ_RESET_PERIOD     10000000         /* 10 seconds */
#define MLFQ_LEVEL_RUNTIME(x) (x + 1) * 100000 /* e.g., 100ms for level 0 */

extern SLIST_HEAD(PCB, process) runnable;
extern struct process* running_proc[NCORES + 1];

static void proc_set_status(int pid, enum proc_status status) {
    for (uint i = 0; i <= NCORES; i++)
        if (running_proc[i] && running_proc[i]->pid == pid)
            running_proc[i]->status = status;
    struct process* p;
    SLIST_FOREACH(p, &runnable, next) if (p->pid == pid) p->status = status;
}

void proc_set_ready(int pid) { proc_set_status(pid, PROC_READY); }
void proc_set_runnable(int pid) { proc_set_status(pid, PROC_RUNNABLE); }

int proc_alloc() {
    struct process* p    = malloc(sizeof(struct process));
    static uint curr_pid = 0;
    p->pid               = ++curr_pid;

    if (curr_pid == GPID_PROCESS) {
        uint core;
        asm("csrr %0, mhartid" : "=r"(core));
        running_proc[core]         = p;
        running_proc[core]->status = PROC_RUNNING;
    } else {
        p->status = PROC_LOADING;
        SLIST_INSERT_HEAD(&runnable, p, next);
    }

    return curr_pid;
}

void proc_free(int pid) {
    /* Student's code goes here (Preemptive Scheduler). */

    /* Print the lifecycle statistics of the terminated process or processes. */
    if (pid != GPID_ALL) {
        earth->mmu_free(pid);
        struct process* p;
        SLIST_FOREACH(p, &runnable, next)
        if (p->pid == pid) {
            SLIST_REMOVE(&runnable, p, process, next);
            free(p);
        }
    }
    /* Student's code ends here. */
}

void mlfq_update_level(struct process* p, ulonglong runtime) {
    /* Student's code goes here (Preemptive Scheduler). */

    /* Update the MLFQ-related fields in struct process* p after this
     * process has run on the CPU for another runtime microseconds. */

    /* Student's code ends here. */
}

void mlfq_reset_level() {
    /* Student's code goes here (Preemptive Scheduler). */
    if (!earth->tty_input_empty()) {
        /* Reset the level of GPID_SHELL if there is pending keyboard input. */
    }

    static ulonglong MLFQ_last_reset_time = 0;
    /* Reset the level of all processes every MLFQ_RESET_PERIOD microseconds. */

    /* Student's code ends here. */
}

void proc_sleep(int pid, uint usec) {
    /* Student's code goes here (System Call & Protection). */

    /* Update the sleep-related fields in the struct process for process pid. */

    /* Student's code ends here. */
}

void proc_coresinfo() {
    /* Student's code goes here (Multicore & Locks). */

    /* Print out the pid of the process running on each CPU core. */

    /* Student's code ends here. */
}
