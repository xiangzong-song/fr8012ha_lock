#include <stdint.h>
#include "app_common.h"
#include "ble_protocol.h"
#include "ring_buffer.h"
#include "ble_service.h"
#include "ring_buffer.h"
#include "app_task.h"
#include <stddef.h>
#include <string.h>

#define LOG_TAG "ble_pro"
#define BLE_BUFFER_SIZE                 512
#define BLE_PRO_HEAD_STX                0xA3A4
#define BLE_PRO_HEAD_VER                0x01
#define BLE_PRO_MAX_PAYLOAD_SIZE        256

#define BLE_PRO_VERIFY_KEY              0x01
#define BLE_PRO_ERROR_PROMPT            0x10
#define BLE_PRO_OPENING_INSTRUCTION     0x05
#define BLE_PRO_LOCK_INFORMATION        0x31
#define BLE_PRO_MODIFY_DEVICE_KEY       0x33
#define BLE_PRO_LOW_BAT_UNLOCK          0x35
#define BLE_PRO_BLE_CONFIGURATION       0x13
typedef struct 
{
    uint16_t stx;
    uint8_t len;
    uint8_t rand;
    uint8_t key_once;
    uint8_t cmd;
}__attribute__ ((packed)) ble_pro_head_t;

typedef void (*cmd_fun_t)(uint8_t *payload, uint32_t size);
typedef struct 
{
    uint8_t cmd;
    cmd_fun_t cmd_fun;
}ble_cmd_fun_t;

static ring_buffer_t *ble_buffer = NULL;

void ble_pro_response(uint8_t cmd, uint8_t *payload, uint32_t size)
{
    uint8_t response[256] = {0};
    uint8_t head_size = sizeof(ble_pro_head_t);
    ble_pro_head_t *head = (ble_pro_head_t*)response;
    uint32_t crc8_idx = head_size + size;

    head->stx = BLE_PRO_HEAD_STX;
    head->len = size;
    head->rand = 0;
    head->key_once = 0;
    head->cmd = cmd;
    memcpy(response + head_size, payload, size);
    response[crc8_idx] = app_comm_check_crc8(response, sizeof(ble_pro_head_t) + size);

    ble_service_write_data(response, sizeof(ble_pro_head_t) + size + 1);
}

void ble_pro_verify_key_fun(uint8_t *payload, uint32_t size)
{

}

void ble_pro_error_prompt_fun(uint8_t *payload, uint32_t size)
{

}

void ble_pro_opening_instruction_fun(uint8_t *payload, uint32_t size)
{

}

void ble_pro_lock_information_fun(uint8_t *payload, uint32_t size)
{

}

void ble_pro_modify_device_key_fun(uint8_t *payload, uint32_t size)
{

}

void ble_pro_low_bat_unlock_fun(uint8_t *payload, uint32_t size)
{

}

void ble_pro_ble_configuration_fun(uint8_t *payload, uint32_t size)
{

}

ble_cmd_fun_t ble_cmd_fun_tab[]=
{
    {BLE_PRO_VERIFY_KEY,              ble_pro_verify_key_fun},
    {BLE_PRO_ERROR_PROMPT,            ble_pro_error_prompt_fun},
    {BLE_PRO_OPENING_INSTRUCTION,     ble_pro_opening_instruction_fun},
    {BLE_PRO_LOCK_INFORMATION,        ble_pro_lock_information_fun},
    {BLE_PRO_MODIFY_DEVICE_KEY,       ble_pro_modify_device_key_fun},
    {BLE_PRO_LOW_BAT_UNLOCK,          ble_pro_low_bat_unlock_fun},
    {BLE_PRO_BLE_CONFIGURATION,       ble_pro_ble_configuration_fun},
};

void ble_protocol_cmd_func(uint8_t cmd, uint8_t *payload, uint32_t size)
{
    uint32_t len = sizeof(ble_cmd_fun_tab) / sizeof(ble_cmd_fun_t);
    
    for (int i = 0; i < len; i++)
    {
        if (ble_cmd_fun_tab[i].cmd == cmd)
        {
            ble_cmd_fun_tab[i].cmd_fun(payload, size);
            return;
        }
    }

    APP_COMM_PRINTF("unknow ble cmd %02x\r\n", cmd);
}

ring_buffer_t* ble_protocol_buffer_get(void)
{
    return ble_buffer;
}

uint8_t ble_protocol_check_head(ble_pro_head_t* head)
{
    if (head->stx == BLE_PRO_HEAD_STX && \
        head->len <= BLE_PRO_MAX_PAYLOAD_SIZE)
    {
        return 1;
    }
    else
    {
        APP_COMM_PRINTF("head error: %04x %02x\r\n", \
                        head->stx, head->len);
    }

    return 0;
}

void ble_protocol_process(void)
{
    static uint8_t read_step = 0;
    static ble_pro_head_t head;

    uint8_t recv_checksum;
    uint8_t cal_checksum;
    uint32_t head_size = sizeof(ble_pro_head_t);
    uint8_t recv_data[head_size + BLE_PRO_MAX_PAYLOAD_SIZE + 1];
    
    memset(recv_data, 0, sizeof(recv_data));
    //find head.
    if (read_step == 0)
    {
        if (ring_buffer_valid_size(ble_buffer) < head_size)
        { 
            return;
        }
        ring_buffer_read(ble_buffer, recv_data, head_size, 0);

        while(!ble_protocol_check_head((ble_pro_head_t*)recv_data))
        {
            ring_buffer_update_read_idx(ble_buffer, 1);
            if (ring_buffer_valid_size(ble_buffer) < head_size)
            {
                return;
            }
            ring_buffer_read(ble_buffer, recv_data, head_size, 0);
        }

        ring_buffer_update_read_idx(ble_buffer, head_size);
        memcpy(&head, recv_data, head_size);
        read_step++;
    }
    //get payload.
    else if (read_step == 1)
    {
        memcpy(recv_data, &head, head_size);
        if (ring_buffer_valid_size(ble_buffer) < head.len + 1)
        {
            return;
        }
        ring_buffer_read(ble_buffer, recv_data + head_size, head.len + 1, 1);
    
        recv_checksum = recv_data[head_size + head.len];
        cal_checksum = app_comm_check_crc8(recv_data, head_size + head.len);
    
        if (cal_checksum == recv_checksum)
        {
            ble_protocol_cmd_func(head.cmd, &recv_data[head_size], head.len);
        }
        else
        {
            APP_COMM_PRINTF("checksum error: %02x != %02x\r\n", recv_checksum, cal_checksum);
        } 

        read_step = 0;
    }
}

void ble_protocol_init(void)
{
    ble_buffer = ring_buffer_init(BLE_BUFFER_SIZE);
    if (ble_buffer == NULL)
    {
        APP_COMM_PRINTF("ble_buffer error!\r\n");
    }

    task_register(ble_protocol_process, 0);
}



