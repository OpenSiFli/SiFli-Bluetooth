/*
 * SPDX-FileCopyrightText: 2019-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _BT_CONNECTION_MANAGER_H_
#define _BT_CONNECTION_MANAGER_H_

#ifdef BSP_BT_CONNECTION_MANAGER
#include "config.h"
#include "bts2_bt.h"
#include "bts2_app_demo.h"
#include "drivers/bt_device.h"

#ifdef CFG_MAX_BT_ACL_NUM
    #define BT_CM_DEVICE_MAX_CONN CFG_MAX_BT_ACL_NUM
#else
    #define BT_CM_DEVICE_MAX_CONN 1
#endif
#ifdef CFG_MAX_BT_BOND_NUM
    #define BT_CM_MAX_BOND CFG_MAX_BT_BOND_NUM
#else
    #define BT_CM_MAX_BOND 2
#endif

#define BT_CM_INVALID_CONN_INDEX (0xFF)
#define BT_CM_DEFAULT_MASTER_BIT (BT_CM_A2DP|BT_CM_AVRCP)


#ifdef CFG_OPEN_AVSNK
    #define BT_CM_DEFAULT_SLAVE_BIT  (BT_CM_HFP | BT_CM_A2DP)
#else
    #define BT_CM_DEFAULT_SLAVE_BIT  (BT_CM_HFP)
#endif

#ifdef CFG_HID
    #define BT_CM_DEFAULT_SLAVE_BIT_EXT  (BT_CM_DEFAULT_SLAVE_BIT | BT_CM_HID)
#else
    #define BT_CM_DEFAULT_SLAVE_BIT_EXT  BT_CM_DEFAULT_SLAVE_BIT
#endif


#define BT_CM_HFP       (0x01 << 0)
#define BT_CM_A2DP      (0x01 << 1)
#define BT_CM_AVRCP     (0x01 << 2)
#define BT_CM_HID       (0x01 << 3)
#define BT_CM_PAN       (0x01 << 4)

#define BT_BASIC_PROFILE (BT_CM_HFP | BT_CM_A2DP | BT_CM_AVRCP)

// Master device means audio provider, not BT's role
typedef enum
{
    BT_LINK_MASTER,
    BT_LINK_SLAVE
} bt_cm_link_role_t;

typedef enum
{
    BT_LINK_PHONE = 0x00,
    BT_LINK_EARPHONE
} bt_cm_link_type_t;

// Master device means audio provider, not BT's role
typedef enum
{
    BT_CM_LINK_INCOMING,
    BT_CM_LINK_OUTGOING
} bt_cm_link_dir_t;

typedef enum
{
    BT_CM_ERR_NO_ERR = 0,
    BT_CM_ERR_INVALID_PARA = 1,
    BT_CM_ERR_UNSUPPORTED = 2,
    BT_CM_ERR_GENERAL_ERR = 3,
    BT_CM_ERR_CONN_EXISTED = 4,
    BT_CM_ERR_RESOURCE_NOT_ENOUGH = 5,
} bt_cm_err_t;

typedef enum
{
    BT_CM_CLOSED,
    BT_CM_OPENING,
    BT_CM_OPENED,
    BT_CM_CLOSING,
} bt_cm_state_t;

typedef enum
{
    BT_CM_ACL_STATE_DISCONNECTED,
    BT_CM_ACL_STATE_CONNECTING,
    BT_CM_ACL_STATE_CONNECTED,
    BT_CM_ACL_STATE_DISCONNECTING,
} bt_cm_acl_state_t;

typedef enum
{
    BT_CM_SUB_STATE_IDLE,
    BT_CM_SUB_PROFILING_CONNECTING,
} bt_cm_profile_state_t;


typedef struct
{
    BTS2S_BD_ADDR bd_addr;
    uint32_t dev_cls;   // device of class
    bt_cm_link_role_t role;
    uint8_t link_type;// phone or earphone
    uint8_t is_reconn;
    uint8_t is_use;
} bt_cm_dev_info_t;

typedef struct
{
    bt_cm_dev_info_t info;
    uint8_t is_use;
    uint32_t profiles_bit_mask;
    uint32_t profiles_role_bit_mask;
    rt_timer_t profile_timer_hdl;
    uint16_t conn_hdl;
    // Curretly connect as master or slave;
    uint8_t link_dir;
    uint8_t state;
    uint8_t sub_state;
    uint8_t link_status_update; // link mode update flag
    uint8_t link_mode;
    float   link_interval;
} bt_cm_dev_acl_info_t;

typedef struct
{
    bt_cm_dev_info_t info[BT_CM_MAX_BOND];
    uint8_t last_bond_idx;
} bt_cm_bonded_dev_t;

// Added reconnect flag and addr
typedef struct
{
    bt_cm_dev_acl_info_t bt_devices[BT_CM_DEVICE_MAX_CONN];
    bt_cm_state_t close_process;
} bt_cm_device_manager_t;


void init_bt_cm();
bt_cm_device_manager_t *bt_cm_get_env();
bt_cm_bonded_dev_t *bt_cm_get_bonded_dev(void);


void bt_cm_change_inquiryscan_activity(uint8_t is_high);
void bt_cm_change_page_activity(uint8_t is_high);

bt_cm_err_t bt_cm_connect_req(BTS2S_BD_ADDR *bd_addr, bt_cm_link_type_t link_type);
void bt_cm_disconnect_req(void);

void bt_cm_set_profile_target(uint32_t setProfile, bt_cm_link_type_t link_type, uint8_t addFlag);

bt_cm_err_t bt_cm_profile_connect(uint32_t profile_bit, BTS2S_BD_ADDR *bd_addr, bt_cm_link_type_t link_type);

void bt_cm_add_bonded_dev(bt_cm_dev_info_t *dev, uint8_t force);
void bt_cm_delete_bonded_devs(void);
void bt_cm_delete_bonded_devs_and_linkkey(uint8_t *addr);
bt_cm_dev_info_t *bt_cm_get_bonded_dev_by_addr(uint8_t *addr);

bt_cm_dev_acl_info_t *bt_cm_conn_alloc(bt_cm_device_manager_t *env, BTS2S_BD_ADDR *bd_addr, uint8_t link_type);
bt_cm_dev_acl_info_t *bt_cm_get_conn_by_addr(bt_cm_device_manager_t *env, BTS2S_BD_ADDR *bd_addr);
uint8_t bt_cm_get_bond_index_by_addr(uint8_t *addr);
uint8_t bt_cm_get_addr_by_bond_index(uint8_t idx, BTS2S_BD_ADDR *addr);

#endif // BSP_BT_CONNECTION_MANAGER
#endif // _BT_CONNECTION_MANAGER_H_
