/*
 * (C) 2025, Cornell University
 * All rights reserved.
 *
 * Description: helper functions for process management
 */

#include "process.h"

#define MLFQ_NLEVELS          5
#define MLFQ_RESET_PERIOD     10000000         /* 10 seconds */
#define MLFQ_LEVEL_RUNTIME(x) (x + 1) * 100000 /* e.g., 100ms for level 0 */
extern struct process proc_set[MAX_NPROCESS + 1];

static void proc_set_status(int pid, enum proc_status status) {
    for (uint i = 0; i < MAX_NPROCESS; i++)
        if (proc_set[i].pid == pid) proc_set[i].status = status;
}

void proc_set_ready(int pid) { proc_set_status(pid, PROC_READY); }
void proc_set_running(int pid) { proc_set_status(pid, PROC_RUNNING); }
void proc_set_runnable(int pid) { proc_set_status(pid, PROC_RUNNABLE); }
void proc_set_pending(int pid) { proc_set_status(pid, PROC_PENDING_SYSCALL); }

int proc_alloc() {
    static uint curr_pid = 0;
    for (uint i = 1; i <= MAX_NPROCESS; i++)
        if (proc_set[i].status == PROC_UNUSED) {
            proc_set[i].pid    = ++curr_pid;
            proc_set[i].status = PROC_LOADING;
            /* Student's code goes here (Preemptive Scheduler | System Call). */
            proc_set[i].nint = 0;

            proc_set[i].turn_time = mtime_get();
            proc_set[i].resp_time = mtime_get();
            proc_set[i].acpu_time = 0;

            proc_set[i].mlfq_priority = 0;
            proc_set[i].remaining_time= MLFQ_LEVEL_RUNTIME(0);
            /* Student's code ends here. */
            return curr_pid;
        }

    FATAL("proc_alloc: reach the limit of %d processes", MAX_NPROCESS);
}

void proc_free(int pid) {
    /* Student's code goes here (Preemptive Scheduler). */
    
    for (int i = 0; i < MAX_NPROCESS+1; i++)
        if (proc_set[i].pid == pid){
            proc_set[i].turn_time = mtime_get() - proc_set[i].turn_time;
            printf("[PID]=%d nint=%d turn=%dms resp=%dms acpu=%dms\n", 
                   proc_set[i].pid, proc_set[i].nint,
                   (int)(proc_set[i].turn_time/1000),
                   (int)(proc_set[i].resp_time/1000),
                   (int)(proc_set[i].acpu_time/1000)
            );
        }

    /* Print the lifecycle statistics of the terminated process or processes. */
    if (pid != GPID_ALL) {
        earth->mmu_free(pid);
        proc_set_status(pid, PROC_UNUSED);
    } else {
        /* Free all user processes. */
        for (uint i = 0; i < MAX_NPROCESS; i++)
            if (proc_set[i].pid >= GPID_USER_START &&
                proc_set[i].status != PROC_UNUSED) {
                earth->mmu_free(proc_set[i].pid);
                proc_set[i].status = PROC_UNUSED;
            }
    }
    /* Student's code ends here. */
}

void mlfq_update_level(struct process* p, ulonglong runtime) {
    /* Student's code goes here (Preemptive Scheduler). */
    p->acpu_time += runtime;
    if (p->remaining_time -= runtime <= 0 && p->mlfq_priority < MLFQ_NLEVELS-1){
        p->mlfq_priority += 1;
        p->remaining_time = MLFQ_LEVEL_RUNTIME(p->mlfq_priority);
        INFO("[PID]=%d PRIORITY chaged to LEVEL=%d\n", p->pid, p->mlfq_priority);
    }
    /* Student's code ends here. */
}

void mlfq_reset_level() {
    /* Student's code goes here (Preemptive Scheduler). */
    if (!earth->tty_input_empty()) {
        /* Reset the level of GPID_SHELL if there is pending keyboard input. */
    }

    static ulonglong MLFQ_last_reset_time = 0;
    for (int i = 0; i < MAX_NPROCESS+1; i++){
        proc_set[i].mlfq_priority = 0;
        INFO("[PID]=%d PRIORITY VOLUNTARY swith=%d\n",
             proc_set[i].pid, proc_set[i].mlfq_priority);
    }
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
