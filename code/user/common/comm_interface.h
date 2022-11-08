#ifndef _COMM_INTERFACE_H_
#define _COMM_INTERFACE_H_
#include "co_printf.h"
#include <stdbool.h>

#define COMM_ASSERT(x) \
    if ((x) == true)\
    {\
        co_printf("Error: %s, %d \r\n",__FILE__,__LINE__);\
    }

extern uint8_t comm_time_is_up(uint32_t *tick, uint32_t interval);

#endif

