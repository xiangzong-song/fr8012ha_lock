#ifndef _APP_COMMON_H_
#define _APP_COMMON_H_
#include "co_printf.h"
#include <stdbool.h>

#define RET_SUCCESS 0
#define RET_FAIL    1

#define APP_COMM_ASSERT(x) \
    if ((x) == true)\
    {\
        co_printf("Error: %s, %d \r\n",__FILE__,__LINE__);\
        while(1);\
    }
    
#define APP_COMM_PRINTF(str, arg...) \
    co_printf("["LOG_TAG"] "str, ##arg);

uint8_t app_comm_time_is_up(uint32_t *tick, uint32_t interval);
void app_comm_print_buff(uint8_t *data, uint32_t size);
uint8_t app_comm_checksum(uint8_t* data, uint32_t size);
uint8_t app_comm_check_crc8(uint8_t* data, uint32_t size);

#endif
