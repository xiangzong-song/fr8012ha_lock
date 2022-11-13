#ifndef _APP_TASK_H_
#define _APP_TASK_H_
#include <stdint.h>

typedef void (*task_process_t)(void);

typedef struct task
{
    uint32_t interval;
    uint32_t interval_cnt;
    task_process_t task_process;
    struct task *next;
}task_t;

void task_run(void);
void task_register(task_process_t func, uint32_t interval);
void task_unregister(task_process_t func);
void task_print(void);

#endif

