#ifndef _TASK_TIME_SLICE_H_
#define _TASK_TIME_SLICE_H_
#include <stdint.h>

typedef void (*task_process_t)(void);

typedef struct task
{
    uint32_t id;
    uint32_t interval;
    uint32_t interval_cnt;
    task_process_t task_process;
    struct task *next;
}task_t;

extern void task_run(void);
extern void task_register(task_t* task);
extern void task_unregister(task_t* task);
extern void task_print(void);

#endif

