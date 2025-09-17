/*
 * (C) 2025, Cornell University
 * All rights reserved.
 *
 * Description: kernel ≈ 2 handlers
 *   intr_entry() handles timer and device interrupts.
 *   excp_entry() handles system calls and faults (e.g., invalid memory access).
 */

#include "process.h"
#include <string.h>

uint core_in_kernel;
SLIST_HEAD(PCB, process) runnable;
struct process* running_proc[NCORES + 1];
/* QEMU has cores with ID #1 .. #NCORES. */
/* Arty has cores with ID #0 .. #NCORES-1. */

#define curr_pid     running_proc[core_in_kernel]->pid
#define curr_mepc    running_proc[core_in_kernel]->mepc
#define curr_status  running_proc[core_in_kernel]->status
#define curr_syscall running_proc[core_in_kernel]->syscall
#define curr_saved   running_proc[core_in_kernel]->saved_registers

static void intr_entry(uint);
static void excp_entry(uint);

void kernel_entry() {
    /* With the kernel lock, only one core can enter this point at any time. */
    asm("csrr %0, mhartid" : "=r"(core_in_kernel));

/* Save the process context. */
#define SAVED_REGISTER_ADDR (void*)(EGOS_STACK_TOP - 32 * sizeof(uint))
    asm("csrr %0, mepc" : "=r"(curr_mepc));
    memcpy(curr_saved, SAVED_REGISTER_ADDR, 32 * sizeof(uint));

    uint mcause;
    asm("csrr %0, mcause" : "=r"(mcause));
    (mcause & (1 << 31)) ? intr_entry(mcause & 0x3FF) : excp_entry(mcause);

    /* Restore the process context. */
    asm("csrw mepc, %0" ::"r"(curr_mepc));
    memcpy(SAVED_REGISTER_ADDR, curr_saved, 32 * sizeof(uint));
}

#define INTR_ID_TIMER   7
#define EXCP_ID_ECALL_U 8
#define EXCP_ID_ECALL_M 11
static void proc_yield();
static void proc_try_syscall(struct process* proc);

static void excp_entry(uint id) {
    if (id >= EXCP_ID_ECALL_U && id <= EXCP_ID_ECALL_M) {
        /* Copy the system call arguments from user space to the kernel. */
        uint syscall_paddr = earth->mmu_translate(curr_pid, SYSCALL_ARG);
        memcpy(&curr_syscall, (void*)syscall_paddr, sizeof(struct syscall));
        curr_syscall.status = PENDING;
        curr_mepc           = curr_mepc + 4;
        curr_status         = PROC_PENDING_SYSCALL;
        proc_try_syscall(running_proc[core_in_kernel]);
        proc_yield();
        return;
    }
    /* Student's code goes here (System Call & Protection | Virtual Memory). */

    /* Kill the current process if curr_pid is a user application. */

    /* Student's code ends here. */
    FATAL("excp_entry: kernel got exception %d", id);
}

static void intr_entry(uint id) {
    if (id != INTR_ID_TIMER) FATAL("excp_entry: kernel got interrupt %d", id);
    /* Student's code goes here (Preemptive Scheduler). */

    /* Update the process lifecycle statistics. */

    /* Student's code ends here. */
    proc_yield();
}

static void proc_yield() {
    if (curr_status == PROC_RUNNING) curr_status = PROC_RUNNABLE;

    /* Student's code goes here (Multiple Projects). */

    /* [Preemptive Scheduler]
     * Measure and record lifecycle statistics for the *current* process.
     * Modify the loop below to find the next process to schedule with MLFQ.
     * [System Call & Protection]
     * Do not schedule a process that should still be sleeping at this time. */

    struct process* next;
    SLIST_FOREACH(next, &runnable, next) {
        if (next->status == PROC_PENDING_SYSCALL) proc_try_syscall(next);

        if (next->status == PROC_READY || next->status == PROC_RUNNABLE) {
            SLIST_REMOVE(&runnable, next, process, next);
            break;
        }
    }

    if (next) {
        SLIST_INSERT_HEAD(&runnable, running_proc[core_in_kernel], next);
        running_proc[core_in_kernel] = next;
        /* [Preemptive Scheduler]
         * Measure and record lifecycle statistics for the *next* process.
         * [System Call & Protection | Multicore & Locks]
         * Modify mstatus.MPP to enter machine or user mode after mret. */

    } else {
        /* [Multicore & Locks]
         * Release the kernel lock.
         * [Multicore & Locks | System Call & Protection]
         * Set curr_proc_idx to MAX_NPROCESS; Reset the timer;
         * Enable interrupts by setting the mstatus.MIE bit to 1;
         * Wait for the next interrupt using the wfi instruction. */

        if (curr_status == PROC_PENDING_SYSCALL)
            proc_try_syscall(running_proc[core_in_kernel]);
        if (curr_status != PROC_READY && curr_status != PROC_RUNNABLE)
            FATAL("proc_yield: no process to run on core %d", core_in_kernel);
    }
    /* Student's code ends here. */

    earth->mmu_switch(curr_pid);
    earth->mmu_flush_cache();
    if (curr_status == PROC_READY) {
        /* Setup argc, argv and program counter for a newly created process. */
        curr_saved[0] = APPS_ARG;
        curr_saved[1] = APPS_ARG + 4;
        curr_mepc     = APPS_ENTRY;
    }
    curr_status = PROC_RUNNING;
    earth->timer_reset(core_in_kernel);
}

static void proc_try_send(struct process* sender) {
    struct process* dst;
    SLIST_FOREACH(dst, &runnable, next) {
        if (dst->pid == sender->syscall.receiver) {
            /* Return if dst is not receiving or not taking msg from sender. */
            if (!(dst->syscall.type == SYS_RECV &&
                  dst->syscall.status == PENDING) ||
                !(dst->syscall.sender == GPID_ALL ||
                  dst->syscall.sender == sender->pid))
                return;

            dst->syscall.status = DONE;
            dst->syscall.sender = sender->pid;
            /* Copy the system call arguments within the kernel PCB. */
            memcpy(dst->syscall.content, sender->syscall.content,
                   SYSCALL_MSG_LEN);
            return;
        }
    }
    FATAL("proc_try_send: unknown receiver pid=%d", sender->syscall.receiver);
}

static void proc_try_recv(struct process* receiver) {
    if (receiver->syscall.status == PENDING) return;

    /* Copy the system call struct from the kernel back to user space. */
    uint syscall_paddr = earth->mmu_translate(receiver->pid, SYSCALL_ARG);
    memcpy((void*)syscall_paddr, &receiver->syscall, sizeof(struct syscall));

    /* Set the receiver and sender back to RUNNABLE. */
    receiver->status = PROC_RUNNABLE;
    proc_set_runnable(receiver->syscall.sender);
}

static void proc_try_syscall(struct process* proc) {
    SLIST_INSERT_HEAD(&runnable, running_proc[core_in_kernel], next);
    switch (proc->syscall.type) {
    case SYS_RECV:
        proc_try_recv(proc);
        break;
    case SYS_SEND:
        proc_try_send(proc);
        break;
    default:
        FATAL("proc_try_syscall: unknown syscall type=%d", proc->syscall.type);
    }
    SLIST_REMOVE(&runnable, running_proc[core_in_kernel], process, next);
}
