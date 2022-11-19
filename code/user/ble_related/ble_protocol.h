#ifndef _BLE_PTOTOCOL_H_
#define _BLE_PTOTOCOL_H_
#include "ring_buffer.h"

void ble_protocol_init(void);
ring_buffer_t* ble_protocol_buffer_get(void);

#endif

