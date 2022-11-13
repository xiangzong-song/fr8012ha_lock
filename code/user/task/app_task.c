#include <stdint.h>
#include <stddef.h>
#include "app_task.h"
#include "os_mem.h"
#include "co_printf.h"
#include "app_common.h"

#define LOG_TAG "A_T"

task_t *task_head = NULL;
uint32_t task_id = 0;

task_t *task_new(task_process_t func, uint32_t interval)
{
    task_t* task = NULL;
    task = (task_t*)os_malloc(sizeof(task_t));
    task->interval = interval;
    task->interval_cnt = 0;
    task->task_process = func;
    task->next = NULL;

    return task;
}

void task_register(task_process_t func, uint32_t interval)
{
    task_t* temp = task_head;
    task_t* new;
    
    new = task_new(func, interval);

    if (task_head == NULL)
    {
        task_head = new;
    }
    else
    {
        while(temp->next != NULL)
        {
            temp = temp->next;
        }

        temp->next = new;
    }
}

void task_unregister(task_process_t func)
{
    task_t* temp = task_head;
    task_t* find;

    APP_COMM_ASSERT(func == NULL);
    APP_COMM_ASSERT(temp == NULL);
      
    if (task_head->task_process == func)
    {
        find = task_head;
        task_head = task_head->next;
        os_free(find);
    }
    else
    {
        while(temp->next != NULL && temp->next->task_process != func)
        {
            temp = temp->next;
        }
        if (temp->next != NULL)
        {
            find = temp->next;
            temp->next = find->next;
            os_free(find);
        }
        else
        {
            APP_COMM_PRINTF("task not find!\r\n");
        }
    }
}

void task_run(void)
{
    task_t* temp = task_head;
    static uint32_t tick = 0;

    if (app_comm_time_is_up(&tick, 1))
    {
        while (temp != NULL)
        {
            if (++(temp->interval_cnt) >= temp->interval)
            {
                temp->interval_cnt = 0;
                temp->task_process();
            }
            temp = temp->next;
        }
    }
}

void task_print(void)
{
    task_t* task = task_head;
    static uint32_t tick = 0;

    APP_COMM_PRINTF("\r\n========== task_print ==========\r\n");
    while (task != NULL)
    {
        co_printf("task: 0x%x\r\n" ,task);
        co_printf("interval: %d\r\n" ,task->interval);
        co_printf("interval_cnt: %d\r\n" ,task->interval_cnt);
        co_printf("task_process: 0x%x\r\n" ,task->task_process);
        task = task->next;
    }
    APP_COMM_PRINTF("=============================\r\n");
}
