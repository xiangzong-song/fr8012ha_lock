#include <stdint.h>
#include "ring_buffer.h"
#include "co_printf.h"
#include "os_mem.h"
#include "app_common.h"

#define LOG_TAG "R_B"

ring_buffer_t* ring_buffer_init(uint32_t size)
{
    ring_buffer_t *ring_buffer;

    ring_buffer = (ring_buffer_t*)os_malloc(sizeof(ring_buffer_t));
    ring_buffer->buffer = os_malloc(size);
    ring_buffer->size = size;
    ring_buffer->write_idx = 0;
    ring_buffer->read_idx = 0;

    return ring_buffer;
}

void ring_buffer_deinit(ring_buffer_t *ring_buffer)
{
    if (ring_buffer->buffer)
    {
        os_free(ring_buffer->buffer);
        ring_buffer->buffer = NULL;
    }

    if (ring_buffer)
    {
        os_free(ring_buffer);
        ring_buffer = NULL;
    }
}

__attribute__((section("ram_code"))) uint32_t ring_buffer_valid_size(ring_buffer_t* ring_buffer)
{
    uint32_t left = 0;

    if (ring_buffer->write_idx >= ring_buffer->read_idx)
    {
       left = ring_buffer->write_idx - ring_buffer->read_idx;
    }
    else
    {
        left = ring_buffer->size + ring_buffer->write_idx - ring_buffer->read_idx;
    }

    return left;
}

__attribute__((section("ram_code"))) uint32_t ring_buffer_free_size(ring_buffer_t* ring_buffer)
{
    uint32_t consume = 0;

    consume = ring_buffer->size - ring_buffer_valid_size(ring_buffer);

    return consume;
}

__attribute__((section("ram_code"))) void ring_buffer_write(ring_buffer_t* ring_buffer, uint8_t *write_data, uint32_t write_size)
{
    uint32_t free_size = 0;
    uint32_t end_left = 0;
    uint32_t size_first = 0;
    uint32_t size_left = 0;

    APP_COMM_ASSERT(ring_buffer == NULL);
    APP_COMM_ASSERT(write_data == NULL);
    
    free_size = ring_buffer_free_size(ring_buffer);
    end_left = ring_buffer->size - ring_buffer->write_idx;
    if (free_size < write_size)
    {
        APP_COMM_PRINTF("size error! free %d < write %d\r\n", free_size, write_size);
        return;
    }

    if(end_left >= write_size)
    {
        size_first = write_size;
        size_left = 0;
    }
    else
    {
        size_first = end_left;
        size_left = write_size - size_first;
    }
    
    if (size_first)
    {
        memcpy(ring_buffer->buffer + ring_buffer->write_idx, write_data, size_first);
        ring_buffer->write_idx += size_first;
    }
    
    if(size_left)
    {
        memcpy(ring_buffer->buffer, write_data + size_first, size_left);
        ring_buffer->write_idx = size_left;
    }
}

__attribute__((section("ram_code"))) void ring_buffer_read(ring_buffer_t* ring_buffer, uint8_t *read_data, uint32_t read_size)
{
    uint32_t valid_size = 0;
    uint32_t end_left = 0;
    uint32_t size_first = 0;
    uint32_t size_left = 0;

    APP_COMM_ASSERT(ring_buffer == NULL);
    APP_COMM_ASSERT(read_data == NULL);
    
    valid_size = ring_buffer_valid_size(ring_buffer);
    end_left = ring_buffer->size - ring_buffer->read_idx;
    if (valid_size < read_size)
    {
        APP_COMM_PRINTF("size error! valid %d < read %d\r\n", valid_size, read_size);
        return;
    }

    if(end_left >= read_size)
    {
        size_first = read_size;
        size_left = 0;
    }
    else
    {
        size_first = end_left;
        size_left = read_size - size_first;
    }
    
    if (size_first)
    {
        memcpy(read_data, ring_buffer->buffer + ring_buffer->read_idx, size_first);
        ring_buffer->read_idx += size_first;
    }
    
    if(size_left)
    {
        memcpy(read_data + size_first, ring_buffer->buffer, size_left);
        ring_buffer->read_idx = size_left;
    }
}

