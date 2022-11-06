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

void app_loop(void)
{
    static uint32_t tick = 0;

    if (comm_time_is_up(&tick, 5000))
    {
        co_printf("free_heap: %d\r\n", os_get_free_heap_size());
    }
}

void app_init(void)
{
    co_printf("\r\napp init start...\r\n");

    system_sleep_disable();
    user_task_init();
    simple_peripheral_init();
    system_set_tx_power(RF_TX_POWER_POS_10dBm);
    os_user_loop_event_set(app_loop);

    co_printf("app init end.\r\n");
}
