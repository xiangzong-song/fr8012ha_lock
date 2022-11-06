#include <stdint.h>
#include <stddef.h>
#include "driver_system.h"
#include "comm_interface.h"

uint8_t comm_time_is_up(uint32_t *tick, uint32_t interval)
{
    uint32_t ms;
    uint32_t delta;

    COMM_ASSERT(tick == NULL);

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