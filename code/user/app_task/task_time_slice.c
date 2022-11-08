#include "task_time_slice.h"
#include <stdint.h>
#include <stddef.h>
#include "os_mem.h"
#include "co_printf.h"
#include "comm_interface.h"

task_t *task_head = NULL;

// task_t *task_add(task_attr_t attr)
// {
//     task_t* task = NULL;
//     task = (task_t*)os_malloc(sizeof(task_t));
//     task->attr = attr;
//     task->next = NULL;

//     return task;
// }

void task_register(task_t* task)
{
    task_t* temp = task_head;
    
    if (task_head == NULL)
    {
        task_head = task;
    }
    else
    {
        while(temp->next != NULL)
        {
            temp = temp->next;
        }

        temp->next = task;
    }
}

void task_unregister(task_t* task)
{
    task_t* temp = task_head;

    COMM_ASSERT(task == NULL);
    COMM_ASSERT(temp == NULL);
      
    if (task_head == task)
    {
        task_head = task_head->next;
    }
    else
    {
        while(temp->next != task)
        {
            temp = temp->next;
        }

        temp->next = temp->next->next;
        os_free(temp->next);
    }
}


void task_run(void)
{
    task_t* temp = task_head;
    static uint32_t tick = 0;

    if (comm_time_is_up(&tick, 1000))
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
    task_t* temp = task_head;
    static uint32_t tick = 0;

    co_printf("\r\n========== task_print ==========\r\n");
    while (temp != NULL)
    {
        co_printf("id: %d\r\n" ,temp->id);
        co_printf("interval: %d\r\n" ,temp->interval);
        co_printf("interval_cnt: %d\r\n" ,temp->interval_cnt);
        temp = temp->next;
    }
    co_printf("=============================\r\n");
}
