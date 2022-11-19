#include <stdio.h>
#include <string.h>
#include "co_printf.h"
#include "gap_api.h"
#include "gatt_api.h"
#include "app_init.h"
#include "os_task.h"
#include "driver_system.h"
#include "user_task.h"
#include "app_common.h"
#include "os_mem.h"
#include "time.h"
#include "app_task.h"
#include "ring_buffer.h"
#include "ble_service.h"
#include "ble_protocol.h"
#include "uart_protocol.h"

#define LOG_TAG "app_init"

void task_idle(void)
{
    APP_COMM_PRINTF("free heap %d\r\n", os_get_free_heap_size());
}

void app_loop(void)
{
    task_run();
}

void app_init(void)
{
    uart_protocol_init();

    APP_COMM_PRINTF("app init start...\r\n");
    system_sleep_disable();
    user_task_init();
    ble_service_init();
    ble_protocol_init();
    system_set_tx_power(RF_TX_POWER_POS_10dBm);
    os_user_loop_event_set(app_loop);
    task_register(task_idle, 5000);
    APP_COMM_PRINTF("app init end.\r\n");
}
