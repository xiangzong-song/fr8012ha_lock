#include <stdint.h>
#include "app_common.h"
#include "ble_protocol.h"
#include "ring_buffer.h"
#include "ble_service.h"
#include "ring_buffer.h"
#include "app_task.h"
#include <stddef.h>

#define LOG_TAG "ble_pro"
#define BLE_BUFFER_SIZE                     512

static ring_buffer_t *ble_buffer = NULL;

ring_buffer_t* ble_protocol_buffer_get(void)
{
    return ble_buffer;
}

void ble_protocol_process(void)
{
    uint32_t size;
    uint8_t recv_data[256];

    size = ring_buffer_valid_size(ble_buffer);
    if (size > 0)
    {
        ring_buffer_read(ble_buffer, recv_data, size, 1);
        app_comm_print_buff(recv_data, size);
    }
}

void ble_protocol_init(void)
{
    ble_buffer = ring_buffer_init(BLE_BUFFER_SIZE);
    if (ble_buffer == NULL)
    {
        APP_COMM_PRINTF("ble_buffer error!\r\n");
    }

    task_register(ble_protocol_process, 20);
}



