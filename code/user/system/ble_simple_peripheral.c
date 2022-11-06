/**
 * Copyright (c) 2019, Freqchip
 * 
 * All rights reserved.
 * 
 * 
 */
 
 /*
 * INCLUDES (����ͷ�ļ�)
 */
#include <stdbool.h>
#include "os_timer.h"
#include "gap_api.h"
#include "gatt_api.h"
#include "driver_gpio.h"
#include "simple_gatt_service.h"
#include "ble_simple_peripheral.h"

#include "sys_utils.h"
#include "flash_usage_config.h"
/*
 * MACROS (�궨��)
 */

/*
 * CONSTANTS (��������)
 */

// GAP - Advertisement data (max size = 31 bytes, though this is
// best kept short to conserve power while advertisting)
// GAP-�㲥��������,�31���ֽ�.��һ������ݿ��Խ�ʡ�㲥ʱ��ϵͳ����.
static uint8_t adv_data[] =
{
  // service UUID, to notify central devices what services are included
  // in this peripheral. ����central������ʲô����, ��������ֻ��һ����Ҫ��.
  0x03,   // length of this data
  GAP_ADVTYPE_16BIT_MORE,      // some of the UUID's, but not all
  0xFF,
  0xFE,
};

// GAP - Scan response data (max size = 31 bytes, though this is
// best kept short to conserve power while advertisting)
// GAP-Scan response����,�31���ֽ�.��һ������ݿ��Խ�ʡ�㲥ʱ��ϵͳ����.
static uint8_t scan_rsp_data[] =
{
  // complete name �豸����
  0x12,   // length of this data
  GAP_ADVTYPE_LOCAL_NAME_COMPLETE,
  'S','i','m','p','l','e','_','P','e','r','i','p','h','e','r','a','l',

  // Tx power level ���书��
  0x02,   // length of this data
  GAP_ADVTYPE_POWER_LEVEL,
  0,	   // 0dBm
};

/*
 * TYPEDEFS (���Ͷ���)
 */

/*
 * GLOBAL VARIABLES (ȫ�ֱ���)
 */

/*
 * LOCAL VARIABLES (���ر���)
 */
os_timer_t update_param_timer;


 
/*
 * LOCAL FUNCTIONS (���غ���)
 */
static void sp_start_adv(void);
/*
 * EXTERN FUNCTIONS (�ⲿ����)
 */

/*
 * PUBLIC FUNCTIONS (ȫ�ֺ���)
 */

/** @function group ble peripheral device APIs (ble������ص�API)
 * @{
 */

void param_timer_func(void *arg)
{
    co_printf("param_timer_func\r\n");
    gap_conn_param_update(0, 12, 12, 55, 600);
}
/*********************************************************************
 * @fn      app_gap_evt_cb
 *
 * @brief   Application layer GAP event callback function. Handles GAP evnets.
 *
 * @param   p_event - GAP events from BLE stack.
 *       
 *
 * @return  None.
 */
void app_gap_evt_cb(gap_event_t *p_event)
{
    switch(p_event->type)
    {
        case GAP_EVT_ADV_END:
        {
            co_printf("adv_end,status:0x%02x\r\n",p_event->param.adv_end.status);
            
        }
        break;
        
        case GAP_EVT_ALL_SVC_ADDED:
        {
            co_printf("All service added\r\n");
            sp_start_adv();
        }
        break;

        case GAP_EVT_SLAVE_CONNECT:
        {
            co_printf("slave[%d],connect. link_num:%d\r\n",p_event->param.slave_connect.conidx,gap_get_connect_num());
            os_timer_start(&update_param_timer,4000,0);
            //gap_security_req(p_event->param.slave_connect.conidx);
        }
        break;

        case GAP_EVT_DISCONNECT:
        {
            co_printf("Link[%d] disconnect,reason:0x%02X\r\n",p_event->param.disconnect.conidx
                      ,p_event->param.disconnect.reason);
					  os_timer_stop(&update_param_timer);
            gap_start_advertising(0);
        }
        break;

        case GAP_EVT_LINK_PARAM_REJECT:
            co_printf("Link[%d]param reject,status:0x%02x\r\n"
                      ,p_event->param.link_reject.conidx,p_event->param.link_reject.status);
            break;

        case GAP_EVT_LINK_PARAM_UPDATE:
            co_printf("Link[%d]param update,interval:%d,latency:%d,timeout:%d\r\n",p_event->param.link_update.conidx
                      ,p_event->param.link_update.con_interval,p_event->param.link_update.con_latency,p_event->param.link_update.sup_to);
            break;

        case GAP_EVT_PEER_FEATURE:
            co_printf("peer[%d] feats ind\r\n",p_event->param.peer_feature.conidx);
            //show_reg((uint8_t *)&(p_event->param.peer_feature.features),8,1);
            break;

        case GAP_EVT_MTU:
            co_printf("mtu update,conidx=%d,mtu=%d\r\n"
                      ,p_event->param.mtu.conidx,p_event->param.mtu.value);
            break;
        
        case GAP_EVT_LINK_RSSI:
            co_printf("link rssi %d\r\n",p_event->param.link_rssi);
            break;
                
        case GAP_SEC_EVT_SLAVE_ENCRYPT:
            co_printf("slave[%d]_encrypted\r\n",p_event->param.slave_encrypt_conidx);
						os_timer_start(&update_param_timer,4000,0);
				
            break;

        default:
            break;
    }
}

/*********************************************************************
 * @fn      sp_start_adv
 *
 * @brief   Set advertising data & scan response & advertising parameters and start advertising
 *
 * @param   None. 
 *       
 *
 * @return  None.
 */
static void sp_start_adv(void)
{
    // Set advertising parameters
    gap_adv_param_t adv_param;
    adv_param.adv_mode = GAP_ADV_MODE_UNDIRECT;
    adv_param.adv_addr_type = GAP_ADDR_TYPE_PUBLIC;
    adv_param.adv_chnl_map = GAP_ADV_CHAN_ALL;
    adv_param.adv_filt_policy = GAP_ADV_ALLOW_SCAN_ANY_CON_ANY;
    adv_param.adv_intv_min = 600;
    adv_param.adv_intv_max = 600;
        
    gap_set_advertising_param(&adv_param);
    
    // Set advertising data & scan response data
	gap_set_advertising_data(adv_data, sizeof(adv_data));
	gap_set_advertising_rsp_data(scan_rsp_data, sizeof(scan_rsp_data));
    // Start advertising
	co_printf("Start advertising...\r\n");
	gap_start_advertising(0);
}


/*********************************************************************
 * @fn      simple_peripheral_init
 *
 * @brief   Initialize simple peripheral profile, BLE related parameters.
 *
 * @param   None. 
 *       
 *
 * @return  None.
 */
void simple_peripheral_init(void)
{
    // set local device name
    uint8_t local_name[] = "Simple Peripheral";
    gap_set_dev_name(local_name, sizeof(local_name));
    os_timer_init( &update_param_timer,param_timer_func,NULL);

#if 0		//security encryption
    gap_security_param_t param =
    {
        .mitm = true,		
        .ble_secure_conn = true,		//not enable security encryption
        .io_cap = GAP_IO_CAP_NO_INPUT_NO_OUTPUT,		//ble device has input ability, will input pin code. 
        .pair_init_mode = GAP_PAIRING_MODE_WAIT_FOR_REQ,	//need bond
        .bond_auth = true,	//need bond auth
    };
#endif 
#if 0
    gap_security_param_t param =
    {
        .mitm = true,		// use PIN code during bonding
        .ble_secure_conn = false,		//not enable security encryption
        .io_cap = GAP_IO_CAP_KEYBOARD_ONLY,		//ble device has input ability, will input pin code. 
        .pair_init_mode = GAP_PAIRING_MODE_WAIT_FOR_REQ,	//need bond
        .bond_auth = true,	//need bond auth
    };
#endif
#if 0
    gap_security_param_t param =
    {
        .mitm = true,		// use PIN code during bonding
        .ble_secure_conn = false,		//not enable security encryption
        .io_cap = GAP_IO_CAP_DISPLAY_ONLY,	//ble device has output ability, will show pin code. 
        .pair_init_mode = GAP_PAIRING_MODE_WAIT_FOR_REQ, //need bond
        .bond_auth = true,	//need bond auth
        .password = 123456,	//set PIN code, it is a dec num between 100000 ~ 999999
    };
#endif
#if 1
    gap_security_param_t param =
    {
        .mitm = false,	// dont use PIN code during bonding
        .ble_secure_conn = false,	//not enable security encryption
        .io_cap = GAP_IO_CAP_NO_INPUT_NO_OUTPUT, //ble device has neither output nor input ability, 
        .pair_init_mode = GAP_PAIRING_MODE_WAIT_FOR_REQ,		//need bond
        .bond_auth = true,	//need bond auth
        .password = 0,
    };
#endif
		// Initialize security related settings.
    gap_security_param_init(&param);
    
    gap_set_cb_func(app_gap_evt_cb);

		//enable bond manage module, which will record bond key and peer service info into flash. 
		//and read these info from flash when func: "gap_bond_manager_init" executes.
    gap_bond_manager_init(BLE_BONDING_INFO_SAVE_ADDR, BLE_REMOTE_SERVICE_SAVE_ADDR, 8, true);
    //gap_bond_manager_delete_all();
    
    mac_addr_t addr;
    gap_address_get(&addr);
    co_printf("Local BDADDR: 0x%2X%2X%2X%2X%2X%2X\r\n", addr.addr[0], addr.addr[1], addr.addr[2], addr.addr[3], addr.addr[4], addr.addr[5]);
    
    // Adding services to database
    sp_gatt_add_service();  
}


