#include <stdint.h>
#include "app_common.h"
#include "ble_protocol.h"
#include "ring_buffer.h"
#include "ble_service.h"
#include "ring_buffer.h"
#include "app_task.h"

#define LOG_TAG "ble_prot"

void ble_protocol_process(void)
{
    uint32_t size;
    uint8_t recv_data[256];

    ring_buffer_t* buffer = ble_service_buffer_get();
    
    size = ring_buffer_valid_size(buffer);
    if (size > 0)
    {
        ring_buffer_read(buffer, recv_data, size);
        app_comm_print_buff(recv_data, size);
    }
}

void ble_protocol_init(void)
{
    task_register(ble_protocol_process, 20);
}



