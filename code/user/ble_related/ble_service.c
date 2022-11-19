#include <stdbool.h>
#include "gap_api.h"
#include "gatt_api.h"
#include "simple_gatt_service.h"
#include "sys_utils.h"
#include "flash_usage_config.h"
#include "ring_buffer.h"
#include "app_common.h"
#include "ble_protocol.h"

#define LOG_TAG "ble_ser"

#define ADV_DATA_LEN        (27+1)
#define RESP_DATA_LEN       18
#define ADV_NAME_LEN        16

uint8_t adv_data[ADV_DATA_LEN] = {  0x11, 0x09, 't', 'e', 's', 't', '_', '_', 'h', '9', '9', '9', '9', '_', '0', '0', '0', '0',\
                                    0x09, 0xff, 0x02, 0x88, 0xec, 0x00, 0x00, 0x00, 0x01, 0x00};
uint8_t adv_resp[RESP_DATA_LEN] = {0x11, 0x09, 't', 'e', 's', 't', '_', '_', 'h', '9', '9', '9', '9', '_', '0', '0', '0', '0'};
uint8_t dev_name[ADV_NAME_LEN] = {'t', 'e', 's', 't', '_', '_', 'h', '9', '9', '9', '9', '_', '0', '0', '0', '0'};
      
#define BLE_ADV_BUFFER_LEN                  32
#define BLE_DEV_BUFFER_LEN                  32
#define GATT_CHAR1_DESC_LEN                 20
#define GATT_CHAR2_DESC_LEN                 20
#define GATT_CHAR1_VALUE_LEN                300
#define GATT_CHAR2_VALUE_LEN                300
#define GATT_SVC1_TX_UUID_128               "\x10\x2B\x0D\x0C\x0B\x0A\x09\x08\x07\x06\x05\x04\x03\x02\x01\x00"
#define GATT_SVC1_RX_UUID_128               "\x11\x2B\x0D\x0C\x0B\x0A\x09\x08\x07\x06\x05\x04\x03\x02\x01\x00"

typedef enum
{
    GATT_IDX_SERVICE = 0,
    GATT_IDX_CHAR1_DECLARATION,
    GATT_IDX_CHAR1_VALUE,
    GATT_IDX_CHAR1_CFG,
    GATT_IDX_CHAR1_USER_DESCRIPTION,
    GATT_IDX_CHAR2_DECLARATION,
    GATT_IDX_CHAR2_VALUE,
    GATT_IDX_CHAR2_CFG,
    GATT_IDX_CHAR2_USER_DESCRIPTION,
    GATT_IDX_NB,
} GATT_INDEX_E;

typedef enum
{
    BLE_STATE_DISCONNECT = 0,
    BLE_STATE_CONNECTED
} ble_state_e;

typedef struct
{
    uint8_t *adv_data;
    uint16_t adv_len;
    uint8_t *rsp_data;
    uint16_t rsp_len;
    uint8_t *dev_data;
    uint8_t dev_len;
    uint16_t adv_intv;
} ble_service_t;

static uint8_t gatt_service_id = 0;
static gatt_service_t govee_gatt_service;
static ble_service_t ble_service;
static uint8_t adv_status = 0;
static uint8_t phone_link_conidx = 0xff;
static ble_state_e ble_conn_state = BLE_STATE_DISCONNECT;

static const uint8_t gatt_char1_desc[GATT_CHAR1_DESC_LEN] = "for gatt Read";
static const uint8_t gatt_char2_desc[GATT_CHAR2_DESC_LEN] = "for gatt Write";
static const uint8_t server_uuid[16] = {0x10, 0x19, 0x0D, 0x0C, 0x0B, 0x0A, 0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00};
static const gatt_attribute_t gatt_profile_att_table[GATT_IDX_NB] =
{
    // Simple gatt Service Declaration
    [GATT_IDX_SERVICE] =
    {
        {UUID_SIZE_2, UUID16_ARR(GATT_PRIMARY_SERVICE_UUID)},       /* UUID */
        GATT_PROP_WRITE_REQ | GATT_PROP_READ | GATT_PROP_NOTI,      /* Permissions */
        UUID_SIZE_16,                                               /* Max size of the value */     /* Service UUID size in service declaration */
        (uint8_t*)server_uuid,                                    /* Value of the attribute */    /* Service UUID value in service declaration */
    },

    //Write
    // Characteristic 1 Declaration
    [GATT_IDX_CHAR1_DECLARATION] =
    {
        {UUID_SIZE_2, UUID16_ARR(GATT_CHARACTER_UUID)},             /* UUID */
        GATT_PROP_READ,                                             /* Permissions */
        0,                                                          /* Max size of the value */
        NULL,                                                       /* Value of the attribute */
    },
    // Characteristic 1 Value
    [GATT_IDX_CHAR1_VALUE] =
    {
        {UUID_SIZE_16, GATT_SVC1_TX_UUID_128},                /* UUID */
        GATT_PROP_READ | GATT_PROP_NOTI,                            /* Permissions */
        GATT_CHAR1_VALUE_LEN,                                       /* Max size of the value */
        NULL,                                                       /* Value of the attribute */    /* Can assign a buffer here, or can be assigned in the application by user */
    },

    // Characteristic 1 client characteristic configuration
    [GATT_IDX_CHAR1_CFG]  =
    {
        {UUID_SIZE_2, UUID16_ARR(GATT_CLIENT_CHAR_CFG_UUID)},       /* UUID */
        GATT_PROP_READ | GATT_PROP_WRITE,                           /* Permissions */
        2,                                                          /* Max size of the value */
        NULL,                                                       /* Value of the attribute */    /* Can assign a buffer here, or can be assigned in the application by user */
    },
    // Characteristic 1 User Description
    [GATT_IDX_CHAR1_USER_DESCRIPTION] =
    {
        {UUID_SIZE_2, UUID16_ARR(GATT_CHAR_USER_DESC_UUID)},        /* UUID */
        GATT_PROP_READ,                                             /* Permissions */
        GATT_CHAR1_DESC_LEN,                                        /* Max size of the value */
        (uint8_t*)gatt_char1_desc,                                  /* Value of the attribute */
    },

    //Read
    // Characteristic 2 Declaration
    [GATT_IDX_CHAR2_DECLARATION] =
    {
        {UUID_SIZE_2, UUID16_ARR(GATT_CHARACTER_UUID)},             /* UUID */
        GATT_PROP_READ,                                             /* Permissions */
        0,                                                          /* Max size of the value */
        NULL,                                                       /* Value of the attribute */
    },
    // Characteristic 2 Value
    [GATT_IDX_CHAR2_VALUE] =
    {
        {UUID_SIZE_16, GATT_SVC1_RX_UUID_128},                /* UUID */
        GATT_PROP_WRITE | GATT_PROP_READ | GATT_PROP_NOTI,          /* Permissions */
        GATT_CHAR1_VALUE_LEN,                                       /* Max size of the value */
        NULL,                                                       /* Value of the attribute */    /* Can assign a buffer here, or can be assigned in the application by user */
    },
    // Characteristic 2 client characteristic configuration
    [GATT_IDX_CHAR2_CFG] =
    {
        {UUID_SIZE_2, UUID16_ARR(GATT_CLIENT_CHAR_CFG_UUID)},       /* UUID */
        GATT_PROP_READ | GATT_PROP_WRITE,                           /* Permissions */
        2,                                                          /* Max size of the value */
        NULL,                                                       /* Value of the attribute */    /* Can assign a buffer here, or can be assigned in the application by user */
    },
    // Characteristic 2 User Description
    [GATT_IDX_CHAR2_USER_DESCRIPTION] =
    {
        {UUID_SIZE_2, UUID16_ARR(GATT_CHAR_USER_DESC_UUID)},        /* UUID */
        GATT_PROP_READ,                                             /* Permissions */
        GATT_CHAR1_DESC_LEN,                                        /* Max size of the value */
        (uint8_t*)gatt_char2_desc,                                  /* Value of the attribute */
    },
};



static void ble_service_adv_start(uint8_t force)
{
    if (adv_status == 1 && force == 0)
    {
        return;
    }

    gap_adv_param_t adv_param;
    memset(&adv_param, 0, sizeof(gap_adv_param_t));
    adv_param.adv_mode = GAP_ADV_MODE_UNDIRECT;
    adv_param.disc_mode = GAP_ADV_DISC_MODE_GEN_DISC;
    adv_param.adv_addr_type = GAP_ADDR_TYPE_PUBLIC;
    adv_param.adv_chnl_map = GAP_ADV_CHAN_ALL;
    adv_param.adv_filt_policy = GAP_ADV_ALLOW_SCAN_ANY_CON_ANY;
    adv_param.adv_intv_min = ble_service.adv_intv;
    adv_param.adv_intv_max = ble_service.adv_intv;
    gap_set_advertising_param(&adv_param);
    gap_set_advertising_data(ble_service.adv_data, ble_service.adv_len);
    gap_set_advertising_rsp_data(ble_service.rsp_data, ble_service.rsp_len);

    APP_COMM_PRINTF("start_advertising...\r\n");
    gap_start_advertising(0);
    adv_status = 1;
}

static void ble_service_adv_stop(void)
{
    if (adv_status == 0)
    {
        return;
    }

    APP_COMM_PRINTF("stop_advertising...\r\n");
    gap_stop_advertising();
    adv_status = 0;
}

static void ble_gap_event_callback(gap_event_t* p_event)
{
    switch (p_event->type)
    {
        case GAP_EVT_ADV_END:
        {

        }
        break;

        case GAP_EVT_ALL_SVC_ADDED:
        {
            APP_COMM_PRINTF("All service added.\r\n");
            ble_service_adv_start(0);
        }
        break;

        case GAP_EVT_SLAVE_CONNECT:
        {
            phone_link_conidx = p_event->param.slave_connect.conidx;
            gap_conn_param_update(p_event->param.link_update.conidx, 20, 50, 0, 300);
            ble_conn_state = BLE_STATE_CONNECTED;
            APP_COMM_PRINTF("slave[%d],connect. link_num:%d\r\n", p_event->param.slave_connect.conidx, gap_get_connect_num());

        }
        break;

        case GAP_EVT_DISCONNECT:
        {
            if (phone_link_conidx == p_event->param.disconnect.conidx)
            {
                phone_link_conidx = 0xff;
                ble_conn_state = BLE_STATE_DISCONNECT;
                ble_service_adv_start(1);
            }
        }
        break;

        case GAP_EVT_LINK_PARAM_REJECT:
            APP_COMM_PRINTF("Link[%d]param reject,status:0x%02x\r\n", p_event->param.link_reject.conidx, p_event->param.link_reject.status);
            break;

        case GAP_EVT_LINK_PARAM_UPDATE:
            APP_COMM_PRINTF("Link[%d]param update,interval:%d,latency:%d,timeout:%d\r\n", p_event->param.link_update.conidx
                      , p_event->param.link_update.con_interval, p_event->param.link_update.con_latency, p_event->param.link_update.sup_to);
            break;

        case GAP_EVT_PEER_FEATURE:
            APP_COMM_PRINTF("peer[%d] feats ind\r\n", p_event->param.peer_feature.conidx);
            break;

        case GAP_EVT_MTU:
            APP_COMM_PRINTF("mtu update,conidx=%d,mtu=%d\r\n", p_event->param.mtu.conidx, p_event->param.mtu.value);
            break;

        case GAP_EVT_LINK_RSSI:
            APP_COMM_PRINTF("link rssi %d\r\n", p_event->param.link_rssi);
            break;

        default:
            break;
    }
}


static void ble_service_read_callback(uint8_t* p_read, uint16_t* len, uint16_t att_idx)
{
    //do nothing
}

static void ble_service_write_callback(uint8_t* write_buf, uint16_t len, uint16_t att_idx)
{
    if (NULL == write_buf)
    {
        APP_COMM_PRINTF("BLE read data invalid.\r\n");
        return;
    }

    ring_buffer_write(ble_protocol_buffer_get(), write_buf, len);
}

static uint16_t ble_service_msg_handle(gatt_msg_t* p_msg)
{
    switch (p_msg->msg_evt)
    {
        case GATTC_MSG_READ_REQ:
            ble_service_read_callback((uint8_t*)(p_msg->param.msg.p_msg_data), &(p_msg->param.msg.msg_len), p_msg->att_idx);
            break;

        case GATTC_MSG_WRITE_REQ:
            if (p_msg->att_idx == GATT_IDX_CHAR1_CFG)
            {
                ble_conn_state = BLE_STATE_CONNECTED;
                phone_link_conidx = p_msg->conn_idx;
            }

            ble_service_write_callback((uint8_t*)(p_msg->param.msg.p_msg_data), (p_msg->param.msg.msg_len), p_msg->att_idx);
            break;

        default:
            break;
    }

    return p_msg->param.msg.msg_len;
}

static uint8_t ble_service_add(gatt_service_t* pt_service)
{
    pt_service->p_att_tb = gatt_profile_att_table;
    pt_service->att_nb = GATT_IDX_NB;
    pt_service->gatt_msg_handler = ble_service_msg_handle;
    return gatt_add_service(pt_service);
}

void ble_service_init(void)
{
    mac_addr_t addr;

    gap_security_param_t param =
    {
        .mitm = false,
        .ble_secure_conn = false,
        .io_cap = GAP_IO_CAP_NO_INPUT_NO_OUTPUT,
        .pair_init_mode = GAP_PAIRING_MODE_WAIT_FOR_REQ,
        .bond_auth = true,
        .password = 0,
    };  
    gap_security_param_init(&param);

    ble_service.adv_data = adv_data;
    ble_service.adv_len = sizeof(adv_data);
    ble_service.rsp_data = adv_resp;
    ble_service.rsp_len = sizeof(adv_resp);
    ble_service.dev_data = dev_name;
    ble_service.dev_len = sizeof(dev_name);
    ble_service.adv_intv = 1600;

    gap_set_dev_name(ble_service.dev_data, ble_service.dev_len);
    gap_set_cb_func(ble_gap_event_callback);

    gap_bond_manager_init(BLE_BONDING_INFO_SAVE_ADDR, BLE_REMOTE_SERVICE_SAVE_ADDR, 8, true);
    gap_bond_manager_delete_all();

    gap_address_get(&addr);
    APP_COMM_PRINTF("mac addr[%02x:%02x:%02x:%02x:%02x:%02x]\r\n", addr.addr[0], addr.addr[1], addr.addr[2], addr.addr[3], addr.addr[4], addr.addr[5]);
    
    gatt_service_id = ble_service_add(&govee_gatt_service);
}
