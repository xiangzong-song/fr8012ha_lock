#include <stdint.h>
#include "uart_protocol.h"
#include "ring_buffer.h"
#include <stddef.h>
#include "driver_plf.h"
#include "driver_uart.h"
#include "driver_gpio.h"
#include "app_common.h"
#include "app_task.h"
#include <string.h>

#define LOG_TAG "uart_pro"

#define UART_BUFFER_SIZE                512
#define UART_PRO_HEAD_START             0x55
#define UART_PRO_HEAD_VER               0x01
#define UART_PRO_MAX_PAYLOAD_SIZE       256

static ring_buffer_t *uart_buffer = NULL;

typedef struct 
{
    uint8_t start;
    uint8_t version;
    uint32_t payload_size;
    uint8_t cmd;
}__attribute__ ((packed)) uart_pro_head_t;

ring_buffer_t* uart_protocol_buffer_get(void)
{
    return uart_buffer;
}

__attribute__((section("ram_code"))) void uart1_isr_ram(void)
{
    uint8_t int_id;
    volatile struct uart_reg_t * const uart_reg_ram = (volatile struct uart_reg_t *)UART1_BASE;
    int_id = uart_reg_ram->u3.iir.int_id;
    uint8_t data;

    if(int_id == 0x04 || int_id == 0x0c )   /* Receiver data available or Character time-out indication */
    {
        while(uart_reg_ram->lsr & 0x01)
        {
            data = uart_reg_ram->u1.data;
            if (uart_buffer != NULL)
            {
                ring_buffer_write(uart_buffer, &data, 1);
            }
        }  
    }
    else if(int_id == 0x06)
    {
        uart_reg_ram->lsr = uart_reg_ram->lsr;
    }
}

void uart_protocol_execute(uint8_t cmd, uint8_t *payload, uint32_t size)
{
    APP_COMM_PRINTF("cmd: %02x\r\n", cmd);
    app_comm_print_buff(payload, size);
}

uint8_t uart_protocol_check_head(uart_pro_head_t* head)
{
    if (head->start == UART_PRO_HEAD_START && \
        head->version !=0 && head->payload_size <= UART_PRO_MAX_PAYLOAD_SIZE)
    {
        return 1;
    }
    else
    {
        APP_COMM_PRINTF("head error: %02x %02x %02x\r\n", \
                        head->start, head->version, head->payload_size);
    }

    return 0;
}

void uart_protocol_process(void)
{
    static uint8_t read_step = 0;
    static uart_pro_head_t head;

    uint8_t recv_checksum;
    uint8_t cal_checksum;
    uint32_t head_size = sizeof(uart_pro_head_t);
    uint8_t recv_data[head_size + UART_PRO_MAX_PAYLOAD_SIZE + 1];
    
    memset(recv_data, 0, sizeof(recv_data));
    //find head.
    if (read_step == 0)
    {
        if (ring_buffer_valid_size(uart_buffer) < head_size)
        { 
            return;
        }
        ring_buffer_read(uart_buffer, recv_data, head_size, 0);

        while(!uart_protocol_check_head((uart_pro_head_t*)recv_data))
        {
            ring_buffer_update_read_idx(uart_buffer, 1);
            if (ring_buffer_valid_size(uart_buffer) < head_size)
            {
                return;
            }
            ring_buffer_read(uart_buffer, recv_data, head_size, 0);
        }

        ring_buffer_update_read_idx(uart_buffer, head_size);
        memcpy(&head, recv_data, head_size);
        read_step++;
    }
    //get payload.
    else if (read_step == 1)
    {
        memcpy(recv_data, &head, head_size);
        if (ring_buffer_valid_size(uart_buffer) < head.payload_size + 1)
        {
            return;
        }
        ring_buffer_read(uart_buffer, recv_data + head_size, head.payload_size + 1, 1);
    
        recv_checksum = recv_data[head_size + head.payload_size];
        cal_checksum = app_comm_checksum(recv_data, head_size + head.payload_size);
    
        if (cal_checksum == recv_checksum)
        {
            uart_protocol_execute(head.cmd, &recv_data[head_size], head.payload_size);
        }
        else
        {
            APP_COMM_PRINTF("checksum error: %02x != %02x\r\n", recv_checksum, cal_checksum);
        } 

        read_step = 0;
    }
}

void uart_protocol_init(void)
{
    system_set_port_pull(GPIO_PA2, true);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_2, PORTA2_FUNC_UART1_RXD);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_3, PORTA3_FUNC_UART1_TXD);
    uart_init(UART1, BAUD_RATE_921600);  
    APP_COMM_PRINTF("uart print test!\r\n");
    NVIC_EnableIRQ(UART1_IRQn);

    uart_buffer = ring_buffer_init(UART_BUFFER_SIZE);

    if (uart_buffer == NULL)
    {
        APP_COMM_PRINTF("uart_buffer error!\r\n");
    }

    task_register(uart_protocol_process, 0);
}


