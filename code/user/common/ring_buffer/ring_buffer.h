#ifndef _RING_BUFFER_H_
#define _RING_BUFFER_H_
#include <stdbool.h>

typedef struct
{
    uint32_t size;
    uint32_t write_idx;
    uint32_t read_idx;
    void *buffer;
}ring_buffer_t;

ring_buffer_t* ring_buffer_init(uint32_t size);
void ring_buffer_deinit(ring_buffer_t *ring_buffer);
uint32_t ring_buffer_valid_size(ring_buffer_t* ring_buffer);
uint32_t ring_buffer_free_size(ring_buffer_t* ring_buffer);
void ring_buffer_write(ring_buffer_t* ring_buffer, uint8_t *write_data, uint32_t write_size);
void ring_buffer_read(ring_buffer_t* ring_buffer, uint8_t *read_data, uint32_t read_size, uint8_t update_idx);
void ring_buffer_update_read_idx(ring_buffer_t* ring_buffer, uint8_t increase_value);

#endif


