#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include "app_common.h"
#include "driver_system.h"

#define LOG_TAG "app_com"

uint8_t app_comm_time_is_up(uint32_t *tick, uint32_t interval)
{
    uint32_t ms;
    uint32_t delta;

    APP_COMM_ASSERT(tick == NULL);

    ms = system_get_curr_time();
    if (*tick == 0)
    {
        *tick = ms;
        return 0;
    }

    if (ms >= *tick)
    {
        delta = ms - *tick;
    }
    else
    {
        delta = 0x4FFFFFF + ms - *tick;
    }

    if (delta >= interval)
    {
        *tick = ms;
        return 1;
    }

    return 0;
}

void app_comm_print_buff(uint8_t *data, uint32_t size)
{
    for (int i = 0; i < size; i++)
    {
        if (i > 0  && i % 20 == 0)
        {
            co_printf("\r\n");
        }
        co_printf("%02x ", data[i]);
    }
    co_printf("\r\n");
}

uint8_t app_comm_checksum(uint8_t* data, uint32_t size)
{
    uint8_t checksum = 0;

    for (int i = 0; i < size; i++)
    {
        checksum += data[i];
    }

    return checksum;  
}
