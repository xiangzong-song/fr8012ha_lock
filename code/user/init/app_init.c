#include <stdio.h>
#include <string.h>
#include "co_printf.h"
#include "gap_api.h"
#include "gatt_api.h"
#include "app_init.h"
#include "os_task.h"
#include "driver_system.h"
#include "user_task.h"
#include "ble_simple_peripheral.h"
#include "comm_interface.h"
#include "os_mem.h"
#include "task_time_slice.h"


void task_test1(void)
{
    co_printf("task1\r\n");
}

void task_test2(void)
{
    co_printf("task2\r\n");
}

void task_test3(void)
{
    co_printf("task3\r\n");
}


task_t test1 =
{
    .id = 0,
    .interval = 1,
    .interval_cnt = 0,
    .task_process = task_test1,
    .next = NULL
};

task_t test2 =
{
    .id = 1,
    .interval = 2,
    .interval_cnt = 0,
    .task_process = task_test2,
    .next = NULL
};

task_t test3 =
{
    .id = 2,
    .interval = 3,
    .interval_cnt = 0,
    .task_process = task_test3,
    .next = NULL
};

void app_loop(void)
{
    static uint32_t tick = 0;
    static uint32_t tick1 = 0;
    static uint32_t tick2 = 0;
    static uint32_t tick3 = 0;

    if (comm_time_is_up(&tick, 5000))
    {
        co_printf("free_heap: %d\r\n", os_get_free_heap_size());
    }

    task_run();

    if (comm_time_is_up(&tick1, 5000))
    {
        co_printf("task1_unregister\r\n");
        task_unregister(&test2);
        task_print();
    }
    // if (comm_time_is_up(&tick2, 10000))
    // {
    //     co_printf("task2_unregister\r\n");
    //     task_unregister(&test2);
    //     task_print();
    // }
    // if (comm_time_is_up(&tick3, 15000))
    // {
    //     co_printf("task3_unregister\r\n");
    //     task_unregister(&test3);
    //     task_print();
    // }
}

void app_init(void)
{
    co_printf("\r\napp init start...\r\n");

    system_sleep_disable();
    user_task_init();
    simple_peripheral_init();
    system_set_tx_power(RF_TX_POWER_POS_10dBm);
    os_user_loop_event_set(app_loop);

    task_print();
    task_register(&test1);
    task_print();
    task_register(&test2);
    task_print();
    task_register(&test3);
    task_print();

    co_printf("app init end.\r\n");
}
