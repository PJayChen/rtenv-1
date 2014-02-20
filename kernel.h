#ifndef __KERNEL_H
#define __KERNEL_H

#define TASK_LIMIT 8  /* Max number of tasks we can handle */

/* Task Control Block */
struct task_control_block {
    struct user_thread_stack *stack;
    int pid;
    int status;
    int priority;
    struct task_control_block **prev;
    struct task_control_block  *next;
};
extern struct task_control_block tasks[TASK_LIMIT];

#endif