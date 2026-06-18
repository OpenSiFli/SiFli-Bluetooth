/*
 * SPDX-FileCopyrightText: 2019-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "bts2_app_inc.h"
#ifdef  CFG_MS
    #include <Windows.h>
#endif

#ifdef RT_USING_BT
    #include "bt_rt_device.h"
#endif

#include "drivers/bt_device.h"

#ifdef CFG_HID

#define LOG_TAG         "btsapp_hid"
//#define DBG_LVL          LOG_LVL_INFO
#include "log.h"

#define  POWER_OFF_TIMES      5
#define  MOUSE_SPEED          50
#define  POWER_OFF_DELAY      2

extern bts2_app_stru *bts2g_app_p;
uint8_t   bts2s_hid_openFlag;
static U8 time_count_power_onoff = 0;

//*/************************************************hid descriptor***************************************************************/
const U8 bts2s_sds_hid_device_svc_record_mid_mouse[] =
{
    /* HIDDescriptorList */
    0x09, 0x02, 0x06, /* HIDDescriptorList */
    0x35, 0x49,
    0x35, 0x47,
    0x08, 0x22,
    0x25, 0x43,


    0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
    0x09, 0x02,        // Usage (Mouse)
    0xA1, 0x01,        // Collection (Application)
    0x85, HID_MOUSE_REPORT_ID,        //   Report ID (2)
    0x09, 0x01,        //   Usage (Pointer)
    0xA1, 0x00,        //   Collection (Physical)
    0x05, 0x09,        //     Usage Page (Button)
    0x19, 0x01,        //     Usage Minimum (0x01)
    0x29, 0x08,        //     Usage Maximum (0x08)
    0x15, 0x00,        //     Logical Minimum (0)
    0x25, 0x01,        //     Logical Maximum (1)
    0x95, 0x08,        //     Report Count (8)
    0x75, 0x01,        //     Report Size (1)
    0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x05, 0x01,        //     Usage Page (Generic Desktop Ctrls)
    0x09, 0x30,        //     Usage (X)
    0x09, 0x31,        //     Usage (Y)
    0x16, 0x01, 0xF8,  //     Logical Minimum (-2047)
    0x26, 0xFF, 0x07,  //     Logical Maximum (2047)    XL8 YL4/XH4 YL8
    0x75, 0x0C,        //     Report Size (8)
    0x95, 0x02,        //     Report Count (2)
    0x81, 0x06,        //     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
    0x09, 0x38,        //     Usage (Wheel)
    0x15, 0x81,        //     Logical Minimum (-127)
    0x25, 0x7F,        //     Logical Maximum (127)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x01,        //     Report Count (1)
    0x81, 0x06,        //     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
    0x05, 0x0C,        //     Usage Page (Consumer)
    0x0A, 0x38, 0x02,  //     Usage (AC Pan)
    0x81, 0x06,        //     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              //   End Collection
    0xC0,              // End Collection
};

const U8 bts2s_sds_hid_device_svc_record_mid_keyboard[] =
{
    /* HIDDescriptorList */
    0x09, 0x02, 0x06, /* HIDDescriptorList */
    0x35, 0x48,
    0x35, 0x46,
    0x08, 0x22,
    0x25, 0x42,

    0x05, 0x01,      // USAGE_PAGE (Generic Desktop)
    0x09, 0x06,      //# USAGE (Keyboard)
    0xa1, 0x01,      //# COLLECTION (Application)
    0x85, HID_KEYBOARD_REPORT_ID,      //#     REPORT_ID (1)
    0x75, 0x01,      //#     Report Size (1)
    0x95, 0x08,      //#     Report Count (8)
    0x05, 0x07,      //#     Usage Page (Key Codes)
    0x19, 0xE0,      //#     Usage Minimum (224)
    0x29, 0xE7,      //#     Usage Maximum (231)
    0x15, 0x00,      //#     Logical Minimum (0)
    0x25, 0x01,      //#     Logical Maximum (1)
    0x81, 0x02,      //#     Input (Data, Variable, Absolute); Modifier byte
    0x95, 0x01,      //#     Report Count (1)
    0x75, 0x08,      //#     Report Size (8)
    0x81, 0x01,      //#     Input (Constant); Reserved byte
    0x95, 0x05,      //#     Report Count (5)
    0x75, 0x01,      //#     Report Size (1)
    0x05, 0x08,      //#     Usage Page (LEDs)
    0x19, 0x01,      //#     Usage Minimum (1)
    0x29, 0x05,      //#     Usage Maximum (5)
    0x91, 0x02,      //#     Output (Data, Variable, Absolute); LED report
    0x95, 0x01,      //#     Report Count (1)
    0x75, 0x03,      //#     Report Size (3)
    0x91, 0x01,      //#     Output (Constant); LED report padding
    0x95, 0x06,      //#     Report Count (6)
    0x75, 0x08,      //#     Report Size (8)
    0x15, 0x00,      //#     Logical Minimum (0)
    0x25, 0x65,      //#     Logical Maximum (101)
    0x05, 0x07,      //#     Usage Page (Key Codes)
    0x19, 0x00,      //#     Usage Minimum (0)
    0x29, 0x65,      //#     Usage Maximum (101)
    0x81, 0x00,      //#     Input (Data, Array); Key array (6 bytes)
    0xc0             //# END_COLLECTION
};

/* Length of the keyboard HID SDP descriptor above. */
const U16 bts2s_sds_hid_device_svc_record_mid_keyboard_len = sizeof(bts2s_sds_hid_device_svc_record_mid_keyboard);

const U8 bts2s_sds_hid_device_svc_record_mid_controller[] =
{
    /* HIDDescriptorList */
    0x09, 0x02, 0x06, /* HIDDescriptorList */
    0x35, 0x69,
    0x35, 0x67,
    0x08, 0x22,
    0x25, 0x63,

    0x05, 0x01,      // USAGE_PAGE (Generic Desktop)
    0x09, 0x06,      //# USAGE (Keyboard)
    0xa1, 0x01,      //# COLLECTION (Application)
    0x85, HID_CONTROLLER_REPORT_ID,      //#     REPORT_ID (4)
    0x05, 0x07,      //#     Usage Page (Key Codes)
    0x19, 0xE0,      //#     Usage Minimum (224)
    0x29, 0xE7,      //#     Usage Maximum (231)
    0x15, 0x00,      //#     Logical Minimum (0)
    0x25, 0x01,      //#     Logical Maximum (1)
    0x75, 0x01,      //#     Report Size (1)
    0x95, 0x08,      //#     Report Count (8)
    0x81, 0x02,      //#     Input (Data, Variable, Absolute); Modifier byte
    0x95, 0x06,      //#     Report Count (6)
    0x75, 0x08,      //#     Report Size (8)
    0x15, 0x00,      //#     Logical Minimum (0)
    0x25, 0x65,      //#     Logical Maximum (101)
    0x05, 0x07,      //#     Usage Page (Key Codes)
    0x19, 0x00,      //#     Usage Minimum (0)
    0x29, 0x65,      //#     Usage Maximum (101)
    0x81, 0x00,      //#     Input (Data, Array,Absolute); Modifier byte
    0xc0,            //# END_COLLECTION

    0x05, 0x0C,       // Usage Page (Consumer)
    0x09, 0x01,       //Usage (Consumer Control)
    0xA1, 0x01,       //Collection (Application)
    0x85, HID_CONSUMER_REPORT_ID,       //    Report Id (3)
    0x15, 0x00,       //    Logical minimum (0)
    0x26, 0xff, 0x03,       //    Logical maximum (1023)
    0x19, 0x00,      //#     Usage Minimum (0)
    0x2a, 0xff, 0x03,     //#     Usage Maximum (1023)
    0x75, 0x10,      //#     Report Size (16)
    0x95, 0x03,      //#     Report Count (3)
    0x81, 0x00,      //#     Input (Data, Array,Absolute); Modifier byte
    0xc0,            //# END_COLLECTION

    0x05, 0x01,      // USAGE_PAGE (Generic Desktop)
    0x09, 0x80,       //Usage (System Control)
    0xA1, 0x01,       //Collection (Application)
    0x85, HID_SYSTEM_CONTROL_REPORT_ID,       //    Report Id (5)
    0x05, 0x0C,       // Usage Page (Consumer)
    0x09, 0x81,       //Usage (Assign Selection)
    0x09, 0x83,       //Usage (Recall Last)
    0x09, 0x82,       //Usage (Mode Step)
    0x15, 0x00,      //#     Logical Minimum (0)
    0x25, 0x01,      //#     Logical Maximum (1)
    0x75, 0x01,      //#     Report Size (1)
    0x95, 0x03,      //#     Report Count (3)
    0x81, 0x02,      //#     Input (Data, Variable,Absolute); Modifier byte
    0x75, 0x01,      //#     Report Size (1)
    0x95, 0x05,      //#     Report Count (5)
    0x81, 0x03,      //#     Input (Constant, Variable,Absolute); Modifier byte
    0xc0,            //# END_COLLECTION
};

const U8 bts2s_sds_hid_device_svc_record_mid_touch[] =
{
    /* HIDDescriptorList */
    0x09, 0x02, 0x06, /* HIDDescriptorList */
    0x35, 0x4f,
    0x35, 0x4d,
    0x08, 0x22,
    0x25, 0x49,


    0x05, 0x0d,         //              USAGE_PAGE (Digitizer)
    0x09, 0x01,     //              USAGE (Touch Screen)
    0xa1, 0x01,     //                  COLLECTION (Application (mouse, keyboard))
    0x85, 0x4,     //REPORT_ID (2)
    0x09, 0x22,     //                  USAGE (Finger)
    0xa1, 0x02,     //                  COLLECTION (Logical (interrelated data))
    0x09, 0x42,     //                      USAGE (Tip Switch)
    0x15, 0x00,     //                      LOGICAL_MINIMUM (0)
    0x25, 0x01,     //                      LOGICAL_MAXIMUM (1)
    0x75, 0x01,     //                      REPORT_SIZE (1)
    0x95, 0x01,     //                      REPORT_COUNT (1)
    0x81, 0x02,     //                      INPUT (Data, Variable, Absolute)
    0x09, 0x32,     //                      USAGE (In Range)
    0x81, 0x02,     //                      INPUT (Data, Variable, Absolute)
    0x95, 0x06,     //                      REPORT_COUNT (6)
    0x81, 0x03,       //                        INPUT (Constant, Variable, Absolute)
    0x05, 0x01,     //                      USAGE_PAGE (Generic Desktop Controls)
    0x26, 0xe8, 0x03, //                      LOGICAL_MAXIMUM (4095)
    0x75, 0x10,     //                      REPORT_SIZE (16)
    0x95, 0x01,     //                      REPORT_COUNT (1)
    0x55, 0x00,     //                      UNIT_EXPONENT (0)
    0x65, 0x00,       //                        UNIT (0)
    0x09, 0x30,     //                      USAGE (X)
    0x35, 0x00,     //                      PHYSICAL_MINIMUM (0)
    0x46, 0xe8, 0x03, //                      PHYSICAL_MAXIMUM (906)
    0x81, 0x02,     //                      INPUT (Data, Variable, Absolute)
    0x09, 0x31,     //                      USAGE (Y)
    0x46, 0xe8, 0x03, //                      PHYSICAL_MAXIMUM (1157)
    0x81, 0x02,     //                      INPUT (Data, Variable, Absolute)
    0xc0,           //                      END_COLLECTION
    0x05, 0x0d,     //              USAGE_PAGE (Digitizer)
    0x09, 0x48,     //              USAGE (72)
    0x09, 0x49,     //              USAGE (73)
    0x95, 0x02,     //              REPORT_COUNT (2)
    0x81, 0x02,     //              INPUT (Data, Variable, Absolute)
    0xc0,           //              END_COLLECTION
};

const U8 bts2s_sds_hid_device_svc_record_mid_consumer[] =
{
    // /* HIDDescriptorList */
    0x09, 0x02, 0x06, /* HIDDescriptorList */
    0x35, 0x3a,
    0x35, 0x38,
    0x08, 0x22,
    0x25, 0x34,

    0x05, 0x0C,       // Usage Page (Consumer)
    0x09, 0x01,       //Usage (Consumer Control)
    0xA1, 0x01,       //Collection (Application)
    0x85, HID_CONSUMER_REPORT_ID,       //    Report Id (3)
    0x15, 0x00,       //    Logical minimum (0)
    0x25, 0x01,       //    Logical maximum (1)
    0x75, 0x01,       //    Report Size (1)
    0x95, 0x01,       //    Report Count (1)

    0x09, 0xCD,       //    Usage (Play/Pause)
    0x81, 0x06,       //    Input (Data,Value,Relative,Bit Field)
    0x0A, 0x83, 0x01, //    Usage (AL Consumer Control Configuration)
    0x81, 0x06,       //    Input (Data,Value,Relative,Bit Field)
    0x09, 0xB5,       //    Usage (Scan Next Track)
    0x81, 0x06,       //    Input (Data,Value,Relative,Bit Field)
    0x09, 0xB6,       //    Usage (Scan Previous Track)
    0x81, 0x06,      //    Input (Data,Value,Relative,Bit Field)

    0x09, 0xEA,       //     Usage (Volume Down)
    0x81, 0x06,       //    Input (Data,Value,Relative,Bit Field)
    0x09, 0xE9,       //    Usage (Volume Up)
    0x81, 0x06,       //    Input (Data,Value,Relative,Bit Field)
    0x0A, 0x25, 0x02, //    Usage (AC Forward)
    0x81, 0x06,       //    Input (Data,Value,Relative,Bit Field)
    0x0A, 0x24, 0x02, //    Usage (AC Back)
    0x81, 0x06,       //    Input (Data,Value,Relative,Bit Field)
    0xC0              //End Collection
};

//*/************************************************static function declaration***************************************************************/
static void bt_hid_send_report_req(bts2_app_stru *bts2_app_data, U8 conn_idx, U16 data_len, U8 *data, BOOL send_on_interrupt_channel);
static void bt_hid_mouse_reset(bts2_app_stru *bts2_app_data, U8 conn_idx);
static void bt_hid_mouse_reset_at_middle(bts2_app_stru *bts2_app_data, U8 con_idx);
static void bt_hid_timeout_handler_reset_at_middle_ios(void *parameter);
static void bt_hid_timeout_handler_reset_at_middle_ios1(void *parameter);
static void bt_hid_mouse_move_without_reset(bts2_app_stru *bts2_app_data, S16 dx, S16 dy, U8 con_idx);
static void bt_hid_timeout_handler_drag_up(void *parameter);
static void bt_hid_timeout_handler_reset_at_middle1(void *parameter);
static void bt_hid_timeout_handler_drag_down(void *parameter);
static void bt_hid_timeout_handler_reset_at_middle2(void *parameter);
static void bt_hid_receive_contro_handle(bts2_app_stru *bts2_app_data);
static void hid_receive_get_report_handle(bts2_app_stru *bts2_app_data);
static void hid_send_response_report(bts2_app_stru *bts2_app_data, U8 conn_idx, hid_report_type_enum_t report_type, struct hid_report_data_t *report_data, BOOL has_report_id);
static void hid_receive_set_report_handle(bts2_app_stru *bts2_app_data);
static void hid_receive_get_protocol_handle(bts2_app_stru *bts2_app_data);
static void hid_receive_set_protocol_handle(bts2_app_stru *bts2_app_data);
static void hid_receive_interrupt_report(bts2_app_stru *bts2_app_data);
static BOOL bt_hid_check_is_ios_device(U8 idx);
static void bt_hid_consumer_report_reset(bts2_app_stru *bts2_app_data, U8 conn_idx);
static void bt_hid_timeout_handler_reset_report(void *parameter);

//*/************************************************function definition***************************************************************/
/*
Description:
    Combined hid descriptor
Input:

Time:2026/04/08 17:29:17

Author:zhengyu

Modify:
*/
//!Customers need to implement this weak function to add custom hid descriptors
__WEAK void bt_hid_cmpose_hid_descriptor(void)
{
    //!The sdk provides hid descriptors for mouse, keyboard, consumer and touch by default, and customers can call bt_hid_add_descriptor to add an hid descriptor as needed
    bt_hid_add_descriptor(bts2s_sds_hid_device_svc_record_mid_mouse, sizeof(bts2s_sds_hid_device_svc_record_mid_mouse));
    // bt_hid_add_descriptor(bts2s_sds_hid_device_svc_record_mid_controller, sizeof(bts2s_sds_hid_device_svc_record_mid_controller));
    // bt_hid_add_descriptor(bts2s_sds_hid_device_svc_record_mid_keyboard, sizeof(bts2s_sds_hid_device_svc_record_mid_keyboard));
    bt_hid_add_descriptor(bts2s_sds_hid_device_svc_record_mid_consumer, sizeof(bts2s_sds_hid_device_svc_record_mid_consumer));
    // bt_hid_add_descriptor(bts2s_sds_hid_device_svc_record_mid_touch,sizeof(bts2s_sds_hid_device_svc_record_mid_touch));

}


/*
Description:
    Add new descriptor
Input:
    data:pointer of descriptor
    len:length of descriptor
Time:2026/04/08 17:30:02

Author:zhengyu

Modify:
*/
void bt_hid_add_descriptor(const U8 *data, U8 len)
{
    BTS2S_HID_DESCRIPTOR_LIST *hid_descriptor_list = hid_get_descriptor_list();
    BTS2S_HID_DESCRIPTOR_LIST *hid_descriptor = (BTS2S_HID_DESCRIPTOR_LIST *)malloc(sizeof(BTS2S_HID_DESCRIPTOR_LIST));

    hid_descriptor->payload = (U8 *)malloc(len);
    bmemcpy(hid_descriptor->payload, data, len);
    hid_descriptor->data_len = len;
    hid_descriptor->next_struct = NULL;

    if (hid_descriptor_list == NULL)
    {
        hid_descriptor_list = hid_descriptor;
    }
    else
    {
        BTS2S_HID_DESCRIPTOR_LIST *curr_list = hid_descriptor_list;
        while (curr_list->next_struct != NULL)
        {
            curr_list = (BTS2S_HID_DESCRIPTOR_LIST *)curr_list->next_struct;
        }

        if (curr_list->next_struct == NULL)
        {
            curr_list->next_struct = hid_descriptor;
        }
    }
}


/*
Description:
    hid profile init
Input:
    global app bt instance
Time:2023/04/25 14:10:33

Author:zhengyu

Modify:
*/
void bt_hid_init(bts2_app_stru *bts2_app_data)
{
    for (U8 i = 0; i < CFG_MAX_HID_CONN_NUM; i++)
    {
        bts2_app_data->hid_inst.conn[i].is_hid_device_role = TRUE;
        bts2_app_data->hid_inst.conn[i].hid_time_handle = NULL;
        bts2_app_data->hid_inst.conn[i].hid_time_handle_reset_report = NULL;
        bts2_app_data->hid_inst.conn[i].hid_time_handle_reset_at_middle_ios = NULL;
        bts2_app_data->hid_inst.conn[i].local_protocol_mode = HID_REPORT_PROTOCOL_MODE;
    }
#ifdef CFG_OPEN_HID
    bts2s_hid_openFlag = 1;
#else
    bts2s_hid_openFlag = 0;
#endif

    bt_hid_cmpose_hid_descriptor();

#ifndef BSP_BQB_TEST
    if (1 == bts2s_hid_openFlag)
#endif
    {
        hid_enb_req(bts2_app_data->phdl, HID_Device);
    }
}


/*
Description:
    open the hid
Input:
    null
Time:2023/05/19 10:15:42

Author:zhengyu

Modify:
*/
void bt_hid_open(void)
{
    bts2_app_stru *bts2_app_data = bts2g_app_p;

    USER_TRACE("bt_hid_open %d flag\n", bts2s_hid_openFlag);

    if (0 == bts2s_hid_openFlag)
    {
        bts2s_hid_openFlag = 0x01;
        hid_enb_req(bts2_app_data->phdl, HID_Device);
    }
    else
    {
        bt_interface_bt_event_notify(BT_NOTIFY_HID, BT_NOTIFY_HID_OPEN_COMPLETE, NULL, 0);

        INFO_TRACE(">> URC HID open,alreay open\n");
    }
}


/*
Description:
    close the hid
Input:
    null
Time:2023/05/19 10:15:42

Author:zhengyu

Modify:
*/
void bt_hid_close(void)
{
    USER_TRACE("bt_hid_close %d flag\n", bts2s_hid_openFlag);

    if (0x01 == bts2s_hid_openFlag)
    {
        bts2s_hid_openFlag = 0x00;
        hid_disb_req(); //disable hid
    }
    else
    {
        bt_interface_bt_event_notify(BT_NOTIFY_HID, BT_NOTIFY_HID_CLOSE_COMPLETE, NULL, 0);

        INFO_TRACE(">> alreay close,urc HID close\n");
    }
}


/*
Description:
    connect remote device hid profile
Input:
    global app bt instance
Time:2023/04/25 14:12:35

Author:zhengyu

Modify:
*/
void bt_hid_connect_requset(BTS2S_BD_ADDR *bd)
{
    U8 idx = CFG_MAX_HID_CONN_NUM;
    bts2_app_stru *bts2_app_data = bts2g_app_p;
    USER_TRACE(" -- address: %04X:%02X:%06lX\n",
               bd->nap,
               bd->uap,
               bd->lap);

    if (bt_hid_check_connection_by_addr(bd, &idx))
    {
        USER_TRACE("This address already connected or connecting\n");
        return;
    }

    idx = bt_hid_get_available_connection();

    if (idx == CFG_MAX_HID_CONN_NUM)
    {
        USER_TRACE("hid connection is full\n");
        return;
    }
    else
    {
        bd_copy(&bts2_app_data->hid_inst.conn[idx].rmt_bd, bd);
        bts2_app_data->hid_inst.conn[idx].is_hid_device_role = TRUE;
        hid_conn_req(bts2_app_data->phdl, *bd, HID_Host, HID_Device);
    }
}


/*
Description:
    disconnect remote device hid profile
Input:
    global app bt instance
Time:2023/04/25 14:13:53

Author:zhengyu

Modify:
*/
void bt_hid_disc_2_dev(BTS2S_BD_ADDR *bd_addr)
{
    U8 idx = CFG_MAX_HID_CONN_NUM;

    if (bt_hid_check_connection_by_addr(bd_addr, &idx))
    {
        USER_TRACE("[U-L] disconnect with remote hid...\n");
        hid_disc_req_ext(bd_addr);
    }
    else
    {
        USER_TRACE("[U-L] Please enter a valid address\n");
    }
}


/*
Description:
    handle hid profile connect confirmation
Input:
    global app bt instance
Time:2023/04/25 14:46:02

Author:zhengyu

Modify:
*/
static void bt_hid_hdl_conn_cfm(bts2_app_stru *bts2_app_data)
{
    U8 idx = CFG_MAX_HID_CONN_NUM;
    BTS2S_HID_CONN_CFM *msg;
    msg = (BTS2S_HID_CONN_CFM *)bts2_app_data->recv_msg;

    if (!bt_hid_check_connection_by_addr(&msg->bd, &idx))
    {
        USER_TRACE("[U-L]Why don't pre-alloction connection\n");
        return;
    }

    if (msg->res == BTS2_SUCC)
    {
        if (msg->local_psm == BT_PSM_HID_INTR)
        {
            USER_TRACE("[L-U]HID connect success\n");

            bd_copy(&bts2_app_data->hid_inst.conn[idx].rmt_bd, &msg->bd);

            bt_hid_mouse_reset_at_middle(bts2_app_data, idx);

            bt_notify_profile_state_info_t profile_state;
            bt_addr_convert(&msg->bd, profile_state.mac.addr);
            profile_state.profile_type = BT_NOTIFY_HID;
            profile_state.res = BTS2_SUCC;
            bt_profile_update_connection_state(BT_NOTIFY_HID, BT_NOTIFY_HID_PROFILE_CONNECTED, &profile_state);
        }
    }
    else
    {
        USER_TRACE("[L-U] confirmation connect failed\n");
        bd_set_empty(&bts2_app_data->hid_inst.conn[idx].rmt_bd);

        bt_notify_profile_state_info_t profile_state;
        bt_addr_convert(&msg->bd, profile_state.mac.addr);
        profile_state.profile_type = BT_NOTIFY_HID;
        profile_state.res = msg->res;
        bt_profile_update_connection_state(BT_NOTIFY_HID, BT_NOTIFY_HID_PROFILE_DISCONNECTED, &profile_state);
    }
}


/*
Description:
    handle hid profile disconnect confirmation
Input:
    global app bt instance
Time:2023/04/25 14:47:27

Author:zhengyu

Modify:
*/
static void bt_hid_hdl_disconn_cfm(bts2_app_stru *bts2_app_data)
{
    U8 idx = CFG_MAX_HID_CONN_NUM;
    BTS2S_HID_DISC_CFM *msg;
    msg = (BTS2S_HID_DISC_CFM *)bts2_app_data->recv_msg;

    if (!bt_hid_check_connection_by_addr(&msg->bd, &idx))
    {
        USER_TRACE("[U-L]Why receive un-connected disconnect confirm\n");
        return;
    }

    if (msg->local_psm == BT_PSM_HID_CTRL)
    {
        USER_TRACE("[L-U]HID control channel disconnect success\n");
        bts2_app_data->hid_inst.conn[idx].is_hid_point_calibrated = false;
        bts2_app_data->hid_inst.conn[idx].is_hid_device_role = false;
        bts2_app_data->hid_inst.conn[idx].is_device_ios = false;
        bd_set_empty(&bts2_app_data->hid_inst.conn[idx].rmt_bd);

        bt_notify_profile_state_info_t profile_state;
        bt_addr_convert(&msg->bd, profile_state.mac.addr);
        profile_state.profile_type = BT_NOTIFY_HID;
        profile_state.res = msg->res;
        bt_profile_update_connection_state(BT_NOTIFY_HID, BT_NOTIFY_HID_PROFILE_DISCONNECTED, &profile_state);
    }
}


/*
Description:
    handle hid profile disconnect indication
Input:
    global app bt instance
Time:2023/04/25 14:47:27

Author:zhengyu

Modify:
*/
static void bt_hid_hdl_disconn_ind(bts2_app_stru *bts2_app_data)
{
    U8 idx = CFG_MAX_HID_CONN_NUM;
    BTS2S_HID_DISC_IND *msg;
    msg = (BTS2S_HID_DISC_IND *)bts2_app_data->recv_msg;

    if (!bt_hid_check_connection_by_addr(&msg->bd, &idx))
    {
        USER_TRACE("[U-L]Why receive un-connected disconnect indication\n");
        return;
    }

    if (msg->local_psm == BT_PSM_HID_CTRL)
    {
        USER_TRACE("[L-U]receive remote disconnect control channel...\n\n");
        bts2_app_data->hid_inst.conn[idx].is_hid_point_calibrated = false;
        bts2_app_data->hid_inst.conn[idx].is_hid_device_role = false;
        bts2_app_data->hid_inst.conn[idx].is_device_ios = false;
        bd_set_empty(&bts2_app_data->hid_inst.conn[idx].rmt_bd);

        bt_notify_profile_state_info_t profile_state;
        bt_addr_convert(&msg->bd, profile_state.mac.addr);
        profile_state.profile_type = BT_NOTIFY_HID;
        profile_state.res = msg->res;
        bt_profile_update_connection_state(BT_NOTIFY_HID, BT_NOTIFY_HID_PROFILE_DISCONNECTED, &profile_state);
    }
}


/*
Description:
    send hid report data
Input:
    bts2_app_data:global app bt instance
    conn_idx:connection index
    data_len:data length
    data:pointer of data
    send_on_interrupt_channel:wheather send report on interrupt channel
Time:2026/04/08 17:46:59

Author:zhengyu

Modify:
*/
static void bt_hid_send_report_req(bts2_app_stru *bts2_app_data, U8 conn_idx, U16 data_len, U8 *data, BOOL send_on_interrupt_channel)
{
    //!exit sniff mode before send data
    if (bts2_app_data->hid_inst.conn[conn_idx].mode == SNIFF_MODE)
    {
        USER_TRACE("[U-L]hid exit sniff mode before send report\n");
        hcia_exit_sniff_mode(&bts2_app_data->hid_inst.conn[conn_idx].rmt_bd, NULL);
    }

    hid_send_report_req_ext(bts2_app_data->phdl, &bts2_app_data->hid_inst.conn[conn_idx].rmt_bd, data_len, data, send_on_interrupt_channel);
}


//*/************************************************mouse descriptor function***************************************************************/
/*
Description:
    reset hid mouse operation
Input:
    global app bt instance
Time:2023/04/25 15:06:15

Author:zhengyu

Modify:
*/
static void bt_hid_mouse_reset(bts2_app_stru *bts2_app_data, U8 conn_idx)
{
    hid_msg_mouse_t mouse_msg = {0};
    mouse_msg.header = (HID_MSG_TYPE_DATA << 4) | (HID_REPORT_TYPE_INPUT & 0x3);
    mouse_msg.report_id = HID_MOUSE_REPORT_ID;
    bt_hid_send_report_req(bts2_app_data, conn_idx, sizeof(mouse_msg), (U8 *)&mouse_msg, TRUE);
}


/*
Description:
    reset hid mouse operation at middle
Input:
    global app bt instance
Time:2023/05/05 14:02:48

Author:zhengyu

Modify:
*/
//? 还要在调一下
static void bt_hid_mouse_reset_at_middle(bts2_app_stru *bts2_app_data, U8 con_idx)
{
    if (!bt_hid_check_is_ios_device(con_idx))
    {
        bt_hid_mouse_move_without_reset(bts2_app_data, -2047, -2047, con_idx);
        bt_hid_mouse_move_without_reset(bts2_app_data, 180, 300, con_idx);
        bts2_app_data->hid_inst.conn[con_idx].is_hid_point_calibrated = true;
    }
    else
    {
        bt_hid_mouse_move_without_reset(bts2_app_data, -2047, 2047, con_idx);
        bt_hid_mouse_move_without_reset(bts2_app_data, -2047, 2047, con_idx);
        bt_hid_mouse_move(bts2_app_data, -2047, 2047, con_idx);
        // bt_hid_mouse_move(bts2_app_data, -1024, 1024, con_idx);

        if (!bts2_app_data->hid_inst.conn[con_idx].hid_time_handle_reset_at_middle_ios)
        {
            bts2_app_data->hid_inst.conn[con_idx].hid_time_handle_reset_at_middle_ios = rt_timer_create("hid_ti_ios", bt_hid_timeout_handler_reset_at_middle_ios, (void *)&bts2_app_data->hid_inst.conn[con_idx].rmt_bd,
                    rt_tick_from_millisecond(1000), RT_TIMER_FLAG_SOFT_TIMER);
        }
        else
        {
            rt_timer_stop(bts2_app_data->hid_inst.conn[con_idx].hid_time_handle_reset_at_middle_ios);
        }
        rt_timer_start(bts2_app_data->hid_inst.conn[con_idx].hid_time_handle_reset_at_middle_ios);
    }
}


/*
Description:
    reset hid mouse operation at middle
Input:
    bd address pointer
Time:2023/05/05 14:02:48

Author:zhengyu

Modify:
*/
static void bt_hid_timeout_handler_reset_at_middle_ios(void *parameter)
{
    BTS2S_BD_ADDR *bd_addr = (BTS2S_BD_ADDR *)parameter;
    bts2_app_stru *bts2_app_data = bts2g_app_p;

    U8 con_idx = CFG_MAX_HID_CONN_NUM;

    if (!bt_hid_check_connection_by_addr(bd_addr, &con_idx))
    {
        USER_TRACE("[U-L]Why don't pre-alloction connection\n");
        return;
    }

    bt_hid_mouse_move(bts2_app_data, 800, -1000, con_idx);
    // bt_hid_mouse_move(bts2_app_data, 100, -200, con_idx);
    bts2_app_data->hid_inst.conn[con_idx].is_hid_point_calibrated = true;
}


/*
Description:
    hid device control the mobile drag down once
Input:
    bts2_app_data:global app bt instance
    dx:X-direction offset
    dy:Y-direction offset
Time:2023/04/25 15:07:58

Author:zhengyu

Modify:
*/
void bt_hid_mouse_move(bts2_app_stru *bts2_app_data, S16 dx, S16 dy, U8 conn_idx)
{
    hid_msg_mouse_t mouse_msg = {0};

    mouse_msg.header = (HID_MSG_TYPE_DATA << 4) | (HID_REPORT_TYPE_INPUT & 0x3);
    mouse_msg.report_id = HID_MOUSE_REPORT_ID;
    mouse_msg.dx = (dx & 0x0FFF);
    mouse_msg.dy = (dy & 0x0FFF);
    bt_hid_send_report_req(bts2_app_data, conn_idx, sizeof(mouse_msg), (U8 *)&mouse_msg, TRUE);
    bt_hid_mouse_reset(bts2_app_data, conn_idx);
}

/*
Description:
    send a generic HID keyboard input report (modifier + up to 6 standard keycodes).
    Send an all-zero report (key_num=0) to release; no auto-reset.
Input:
    bts2_app_data:global app bt instance
    conn_idx:connection index
    modifier:HID keyboard modifier byte
    keys:array of HID keycodes
    key_num:number of valid keycodes (0..6)
*/
void bt_hid_send_keyboard(bts2_app_stru *bts2_app_data, U8 conn_idx, U8 modifier, const U8 *keys, U8 key_num)
{
    hid_msg_controller_t kbd_msg = {0};

    kbd_msg.header = (HID_MSG_TYPE_DATA << 4) | (HID_REPORT_TYPE_INPUT & 0x3);
    kbd_msg.report_id = HID_KEYBOARD_REPORT_ID;
    kbd_msg.modifierKeys = modifier;
    if (key_num > 6)
    {
        key_num = 6;
    }
    if (key_num > 0) kbd_msg.standardkey1 = keys[0];
    if (key_num > 1) kbd_msg.standardkey2 = keys[1];
    if (key_num > 2) kbd_msg.standardkey3 = keys[2];
    if (key_num > 3) kbd_msg.standardkey4 = keys[3];
    if (key_num > 4) kbd_msg.standardkey5 = keys[4];
    if (key_num > 5) kbd_msg.standardkey6 = keys[5];
    bt_hid_send_report_req(bts2_app_data, conn_idx, sizeof(kbd_msg), (U8 *)&kbd_msg, TRUE);
}


static void bt_hid_mouse_move_without_reset(bts2_app_stru *bts2_app_data, S16 dx, S16 dy, U8 con_idx)
{
    hid_msg_mouse_t mouse_msg = {0};

    mouse_msg.header = (HID_MSG_TYPE_DATA << 4) | (HID_REPORT_TYPE_INPUT & 0x3);
    mouse_msg.report_id = HID_MOUSE_REPORT_ID;
    mouse_msg.dx = (dx & 0x0FFF);
    mouse_msg.dy = (dy & 0x0FFF);
    bt_hid_send_report_req(bts2_app_data, con_idx, sizeof(mouse_msg), (U8 *)&mouse_msg, TRUE);
}


/*
Description:
    hid device control the mobile click
Input:
    bts2_app_data:global app bt instance
    conn_idx:connection index
Time:2023/04/25 15:07:58

Author:zhengyu

Modify:
*/
void bt_hid_mouse_left_click(bts2_app_stru *bts2_app_data, U8 conn_idx)
{
    if (!bts2_app_data->hid_inst.conn[conn_idx].is_hid_point_calibrated)
    {
        return;
    }

    hid_msg_mouse_t mouse_msg = {0};

    mouse_msg.header = (HID_MSG_TYPE_DATA << 4) | (HID_REPORT_TYPE_INPUT & 0x3);
    mouse_msg.report_id = HID_MOUSE_REPORT_ID;
    mouse_msg.buttons |= 0x01;
    bt_hid_send_report_req(bts2_app_data, conn_idx, sizeof(mouse_msg), (U8 *)&mouse_msg, TRUE);
    bt_hid_mouse_reset(bts2_app_data, conn_idx);
}


/*
Description:
    hid device control the mobile bakeup
Input:
    bts2_app_data:global app bt instance
    conn_idx:connection index
Time:2023/04/25 15:07:58

Author:zhengyu

Modify:
*/
void bt_hid_mouse_right_click(bts2_app_stru *bts2_app_data, U8 conn_idx)
{
    hid_msg_mouse_t mouse_msg = {0};

    mouse_msg.header = (HID_MSG_TYPE_DATA << 4) | (HID_REPORT_TYPE_INPUT & 0x3);
    mouse_msg.report_id = HID_MOUSE_REPORT_ID;
    mouse_msg.buttons |= 0x02;
    bt_hid_send_report_req(bts2_app_data, conn_idx, sizeof(mouse_msg), (U8 *)&mouse_msg, TRUE);
    bt_hid_mouse_reset(bts2_app_data, conn_idx);
}


/*
Description:
    hid device control the mobile left double click
Input:
    bts2_app_data:global app bt instance
    conn_idx:connection index
Time:2023/04/25 15:07:58

Author:zhengyu

Modify:
*/
void bt_hid_mouse_left_double_click(bts2_app_stru *bts2_app_data, U8 conn_idx)
{
    bt_hid_mouse_left_click(bts2_app_data, conn_idx);

    if (!bts2_app_data->hid_inst.conn[conn_idx].hid_time_handle)
    {
        bts2_app_data->hid_inst.conn[conn_idx].hid_time_handle = rt_timer_create("hid_ti_double_click", bt_hid_timeout_handler, (void *)&bts2_app_data->hid_inst.conn[conn_idx].rmt_bd,
                rt_tick_from_millisecond(100), RT_TIMER_FLAG_SOFT_TIMER);
    }
    else
    {
        rt_timer_stop(bts2_app_data->hid_inst.conn[conn_idx].hid_time_handle);
    }
    rt_timer_start(bts2_app_data->hid_inst.conn[conn_idx].hid_time_handle);
}


/*
Description:
    hid device control the mobile left double click
Input:
    parameter:bd address pointer
Time:2023/04/25 15:07:58

Author:zhengyu

Modify:
*/
void bt_hid_timeout_handler(void *parameter)
{
    BTS2S_BD_ADDR *bd_addr = (BTS2S_BD_ADDR *)parameter;
    bts2_app_stru *bts2_app_data = bts2g_app_p;

    U8 conn_idx = CFG_MAX_HID_CONN_NUM;

    if (!bt_hid_check_connection_by_addr(bd_addr, &conn_idx))
    {
        USER_TRACE("[U-L]Why don't pre-alloction connection\n");
        return;
    }

    bt_hid_mouse_left_click(bts2_app_data, conn_idx);
}


/*
Description:
    hid device control the mobile right double click
Input:
    bts2_app_data:global app bt instance
    conn_idx:connection index
Time:2023/04/26 09:23:13

Author:zhengyu

Modify:
*/
void bt_hid_mouse_right_double_click(bts2_app_stru *bts2_app_data, U8 conn_idx)
{
    bt_hid_mouse_right_click(bts2_app_data, conn_idx);
    rt_thread_mdelay(50);
    bt_hid_mouse_right_click(bts2_app_data, conn_idx);
}


/*
Description:
    hid device control the mobile bakeup home
Input:
    bts2_app_data:global app bt instance
    conn_idx:connection index
Time:2023/04/25 15:07:58

Author:zhengyu

Modify:
*/
void bt_hid_mouse_middle_button_click(bts2_app_stru *bts2_app_data, U8 conn_idx)
{
    hid_msg_mouse_t mouse_msg = {0};

    mouse_msg.header = (HID_MSG_TYPE_DATA << 4) | (HID_REPORT_TYPE_INPUT & 0x3);
    mouse_msg.report_id = HID_MOUSE_REPORT_ID;
    mouse_msg.buttons |= 0x04;
    bt_hid_send_report_req(bts2_app_data, conn_idx, sizeof(mouse_msg), (U8 *)&mouse_msg, TRUE);
    bt_hid_mouse_reset(bts2_app_data, conn_idx);
}


/*
Description:
    hid device control the mobile drag once
Input:
    bts2_app_data:global app bt instance
    conn_idx:connection index
    dx:X-direction move
    dy:Y-direction move
    wheel_offset:mouse wheel movement
Time:2023/04/25 15:07:58

Author:zhengyu

Modify:
*/
void bt_hid_mouse_drag_page(bts2_app_stru *bts2_app_data, U8 conn_idx, U8 buttons, S16 dx, S16 dy, S8 wheel_offset)
{
    hid_msg_mouse_t mouse_msg = {0};

    mouse_msg.header = (HID_MSG_TYPE_DATA << 4) | (HID_REPORT_TYPE_INPUT & 0x3);
    mouse_msg.report_id = HID_MOUSE_REPORT_ID;
    mouse_msg.buttons |= buttons;
    mouse_msg.dx = dx;
    mouse_msg.dy = dy;
    mouse_msg.wheel = wheel_offset;
    bt_hid_send_report_req(bts2_app_data, conn_idx, sizeof(mouse_msg), (U8 *)&mouse_msg, TRUE);
}


/*
Description:
    hid device control the mobile drag up once
Input:
    bts2_app_data:global app bt instance
    conn_idx:connection index
Time:2023/05/05 14:07:00

Author:zhengyu

Modify:
*/
void bt_hid_mouse_drag_page_up(bts2_app_stru *bts2_app_data, U8 conn_idx)
{
    if (!bts2_app_data->hid_inst.conn[conn_idx].is_hid_point_calibrated)
    {
        USER_TRACE("%s wait calibrated.", __func__);
        return;
    }

    if (!bt_hid_check_is_ios_device(conn_idx))
    {
        bt_hid_mouse_drag_page(bts2_app_data, conn_idx, 0, 0, 0, 127);
        bt_hid_mouse_drag_page(bts2_app_data, conn_idx, 0, 0, 0, 127);
    }
    else
    {
        bt_hid_mouse_drag_page(bts2_app_data, conn_idx, 0, 0, 0, -127);
        bt_hid_mouse_drag_page(bts2_app_data, conn_idx, 0, 0, 0, -127);
    }
}


/*
Description:
    hid device control the mobile drag down once
Input:
    bts2_app_data:global app bt instance
    conn_idx:connection index
Time:2023/05/05 14:07:00

Author:zhengyu

Modify:
*/
void bt_hid_mouse_drag_page_down(bts2_app_stru *bts2_app_data, U8 conn_idx)
{
    if (!bts2_app_data->hid_inst.conn[conn_idx].is_hid_point_calibrated)
    {
        USER_TRACE("%s wait calibrated.", __func__);
        return;
    }

    if (!bt_hid_check_is_ios_device(conn_idx))
    {
        bt_hid_mouse_drag_page(bts2_app_data, conn_idx, 0, 0, 0, -127);
        bt_hid_mouse_drag_page(bts2_app_data, conn_idx, 0, 0, 0, -127);
    }
    else
    {
        bt_hid_mouse_drag_page(bts2_app_data, conn_idx, 0, 0, 0, 127);
        bt_hid_mouse_drag_page(bts2_app_data, conn_idx, 0, 0, 0, 127);
    }
}


/*
Description:
    Set whether the peer device is an ios device
Input:
    bd_addr:bd address pointer
    is_ios:whether the peer device is an ios device
Time:2026/04/14 13:36:02

Author:zhengyu

Modify:
*/
void bt_hid_set_ios_device(BTS2S_BD_ADDR *bd_addr, U8 is_ios)
{
    bts2_app_stru *bts2_app_data = bts2g_app_p;
    U8 conn_idx = CFG_MAX_HID_CONN_NUM;

    if (!bt_hid_check_connection_by_addr(bd_addr, &conn_idx))
    {
        USER_TRACE("[U-L]Why don't pre-alloction connection\n");
        return;
    }
    bts2_app_data->hid_inst.conn[conn_idx].is_device_ios = is_ios;
    bt_hid_mouse_reset_at_middle(bts2_app_data, conn_idx);
}


/*
Description:
    Check whether the peer is an ios device
Input:
    idx:connection index
Time:2023/04/26 09:24:42

Author:zhengyu

Modify:
*/
static BOOL bt_hid_check_is_ios_device(U8 idx)
{
    bts2_app_stru *bts2_app_data = bts2g_app_p;
    return bts2_app_data->hid_inst.conn[idx].is_device_ios;
}


//*/************************************************consumer descriptor function***************************************************************/
/*
Description:
    send consumer reset data report
Input:
    bts2_app_data:global app bt instance
    conn_idx:connection index
Time:2026/04/14 13:43:29

Author:zhengyu

Modify:
*/
static void bt_hid_consumer_report_reset(bts2_app_stru *bts2_app_data, U8 conn_idx)
{
    hid_msg_consumer_t consumer_msg = {0};

    consumer_msg.header = (HID_MSG_TYPE_DATA << 4) | (HID_REPORT_TYPE_INPUT & 0x3);
    consumer_msg.report_id = HID_CONSUMER_REPORT_ID;
    consumer_msg.consumer = 0x00;
    bt_hid_send_report_req(bts2_app_data, conn_idx, sizeof(consumer_msg), (U8 *)&consumer_msg, TRUE);
}


/*
Description:
    send play status change report
Input:
    bts2_app_data:global app bt instance
    conn_idx:connection index
Time:2026/04/14 13:43:29

Author:zhengyu

Modify:
*/
void bt_hid_consumer_report_play_status(bts2_app_stru *bts2_app_data, U8 conn_idx)
{
    hid_msg_consumer_t consumer_msg = {0};

    consumer_msg.header = (HID_MSG_TYPE_DATA << 4) | (HID_REPORT_TYPE_INPUT & 0x3);
    consumer_msg.report_id = HID_CONSUMER_REPORT_ID;
    consumer_msg.consumer = 0x01;
    bt_hid_send_report_req(bts2_app_data, conn_idx, sizeof(consumer_msg), (U8 *)&consumer_msg, TRUE);
}


/*
Description:
    send sos call report
Input:
    bts2_app_data:global app bt instance
    conn_idx:connection index
Time:2026/04/14 13:43:29

Author:zhengyu

Modify:
*/
void bt_hid_consumer_report_power_onoff(bts2_app_stru *bts2_app_data, U8 conn_idx)
{
    time_count_power_onoff = 0;
    hid_msg_consumer_t consumer_msg = {0};
    consumer_msg.header = (HID_MSG_TYPE_DATA << 4) | (HID_REPORT_TYPE_INPUT & 0x3);
    consumer_msg.report_id = HID_CONSUMER_REPORT_ID;
    consumer_msg.consumer = 0x01;
    bt_hid_send_report_req(bts2_app_data, conn_idx, sizeof(consumer_msg), (U8 *)&consumer_msg, TRUE);
    bt_hid_consumer_report_reset(bts2_app_data, conn_idx);
    if (!bts2_app_data->hid_inst.conn[conn_idx].hid_time_handle_reset_report)
    {
        bts2_app_data->hid_inst.conn[conn_idx].hid_time_handle_reset_report = rt_timer_create("hid_timer_power", bt_hid_timeout_handler_reset_report, (void *)&bts2_app_data->hid_inst.conn[conn_idx].rmt_bd,
                rt_tick_from_millisecond(POWER_OFF_DELAY + time_count_power_onoff), RT_TIMER_FLAG_SOFT_TIMER);
    }
    else
    {
        rt_timer_stop(bts2_app_data->hid_inst.conn[conn_idx].hid_time_handle_reset_report);
    }
    rt_timer_start(bts2_app_data->hid_inst.conn[conn_idx].hid_time_handle_reset_report);
}


/*
Description:
    send sos call report
Input:
    parameter:bd address pointer
Time:2026/04/14 13:43:29

Author:zhengyu

Modify:
*/
static void bt_hid_timeout_handler_reset_report(void *parameter)
{
    BTS2S_BD_ADDR *bd_addr = (BTS2S_BD_ADDR *)parameter;
    bts2_app_stru *bts2_app_data = bts2g_app_p;

    U8 conn_idx = CFG_MAX_HID_CONN_NUM;

    if (!bt_hid_check_connection_by_addr(bd_addr, &conn_idx))
    {
        USER_TRACE("[U-L]Why don't pre-alloction connection\n");
        return;
    }

    if (time_count_power_onoff < POWER_OFF_TIMES)
    {
        hid_msg_consumer_t consumer_msg = {0};

        consumer_msg.header = (HID_MSG_TYPE_DATA << 4) | (HID_REPORT_TYPE_INPUT & 0x3);
        consumer_msg.report_id = HID_CONSUMER_REPORT_ID;
        consumer_msg.consumer = 0x01;

        time_count_power_onoff++;
        bt_hid_send_report_req(bts2_app_data, conn_idx, sizeof(consumer_msg), (U8 *)&consumer_msg, TRUE);
        bt_hid_consumer_report_reset(bts2_app_data, conn_idx);
    }

    if (time_count_power_onoff < POWER_OFF_TIMES)
    {
        if (!bts2_app_data->hid_inst.conn[conn_idx].hid_time_handle_reset_report)
        {
            bts2_app_data->hid_inst.conn[conn_idx].hid_time_handle_reset_report = rt_timer_create("hid_timer_power", bt_hid_timeout_handler_reset_report, (void *)bts2_app_data,
                    rt_tick_from_millisecond(POWER_OFF_DELAY + time_count_power_onoff), RT_TIMER_FLAG_SOFT_TIMER);
        }
        else
        {
            rt_timer_stop(bts2_app_data->hid_inst.conn[conn_idx].hid_time_handle_reset_report);
        }
        rt_timer_start(bts2_app_data->hid_inst.conn[conn_idx].hid_time_handle_reset_report);
    }
    else
    {
        time_count_power_onoff = 0;
    }
}


/*
Description:
    send next song change report
Input:
    bts2_app_data:global app bt instance
    conn_idx:connection index
Time:2026/04/14 13:43:29

Author:zhengyu

Modify:
*/
void bt_hid_consumer_report_next_track(bts2_app_stru *bts2_app_data, U8 conn_idx)
{
    hid_msg_consumer_t consumer_msg = {0};

    consumer_msg.header = (HID_MSG_TYPE_DATA << 4) | (HID_REPORT_TYPE_INPUT & 0x3);
    consumer_msg.report_id = HID_CONSUMER_REPORT_ID;
    consumer_msg.consumer = 0x04;
    bt_hid_send_report_req(bts2_app_data, conn_idx, sizeof(consumer_msg), (U8 *)&consumer_msg, TRUE);
}


/*
Description:
    send previous song change report
Input:
    bts2_app_data:global app bt instance
    conn_idx:connection index
Time:2026/04/14 13:43:29

Author:zhengyu

Modify:
*/
void bt_hid_consumer_report_back_track(bts2_app_stru *bts2_app_data, U8 conn_idx)
{
    hid_msg_consumer_t consumer_msg = {0};

    consumer_msg.header = (HID_MSG_TYPE_DATA << 4) | (HID_REPORT_TYPE_INPUT & 0x3);
    consumer_msg.report_id = HID_CONSUMER_REPORT_ID;
    consumer_msg.consumer = 0x08;
    bt_hid_send_report_req(bts2_app_data, conn_idx, sizeof(consumer_msg), (U8 *)&consumer_msg, TRUE);
}


/*
Description:
    send volume down report
Input:
    bts2_app_data:global app bt instance
    conn_idx:connection index
Time:2026/04/14 13:43:29

Author:zhengyu

Modify:
*/
void bt_hid_consumer_report_volume_down(bts2_app_stru *bts2_app_data, U8 conn_idx)
{
    hid_msg_consumer_t consumer_msg = {0};

    consumer_msg.header = (HID_MSG_TYPE_DATA << 4) | (HID_REPORT_TYPE_INPUT & 0x3);
    consumer_msg.report_id = HID_CONSUMER_REPORT_ID;
    consumer_msg.consumer = 0x10;
    bt_hid_send_report_req(bts2_app_data, conn_idx, sizeof(consumer_msg), (U8 *)&consumer_msg, TRUE);
    bt_hid_consumer_report_reset(bts2_app_data, conn_idx);
}


/*
Description:
    send volume up report
Input:
    bts2_app_data:global app bt instance
    conn_idx:connection index
Time:2026/04/14 13:43:29

Author:zhengyu

Modify:
*/
void bt_hid_consumer_report_volume_up(bts2_app_stru *bts2_app_data, U8 conn_idx)
{
    hid_msg_consumer_t consumer_msg = {0};

    consumer_msg.header = (HID_MSG_TYPE_DATA << 4) | (HID_REPORT_TYPE_INPUT & 0x3);
    consumer_msg.report_id = HID_CONSUMER_REPORT_ID;
    consumer_msg.consumer = 0x20;
    bt_hid_send_report_req(bts2_app_data, conn_idx, sizeof(consumer_msg), (U8 *)&consumer_msg, TRUE);
    bt_hid_consumer_report_reset(bts2_app_data, conn_idx);
}


/*
Description:
    send forward report
Input:
    bts2_app_data:global app bt instance
    conn_idx:connection index
Time:2026/04/14 13:43:29

Author:zhengyu

Modify:
*/
void bt_hid_consumer_report_forward(bts2_app_stru *bts2_app_data, U8 conn_idx)
{
    hid_msg_consumer_t consumer_msg = {0};

    consumer_msg.header = (HID_MSG_TYPE_DATA << 4) | (HID_REPORT_TYPE_INPUT & 0x3);
    consumer_msg.report_id = HID_CONSUMER_REPORT_ID;
    consumer_msg.consumer = 0x40;
    bt_hid_send_report_req(bts2_app_data, conn_idx, sizeof(consumer_msg), (U8 *)&consumer_msg, TRUE);
}


/*
Description:
    send go back report
Input:
    bts2_app_data:global app bt instance
    conn_idx:connection index
Time:2026/04/14 13:43:29

Author:zhengyu

Modify:
*/
void bt_hid_consumer_report_go_back(bts2_app_stru *bts2_app_data, U8 conn_idx)
{
    hid_msg_consumer_t consumer_msg = {0};

    consumer_msg.header = (HID_MSG_TYPE_DATA << 4) | (HID_REPORT_TYPE_INPUT & 0x3);
    consumer_msg.report_id = HID_CONSUMER_REPORT_ID;
    consumer_msg.consumer = 0x80;
    bt_hid_send_report_req(bts2_app_data, conn_idx, sizeof(consumer_msg), (U8 *)&consumer_msg, TRUE);
}


//*/************************************************controller descriptor function***************************************************************/
/*
Description:
    send right control key report
Input:
    bts2_app_data:global app bt instance
    conn_idx:connection index
Time:2026/04/14 13:43:29

Author:zhengyu

Modify:
*/
void bt_hid_controller_report_right_arrow(bts2_app_stru *bts2_app_data, U8 conn_idx)
{
    hid_msg_controller_t controller_msg = {0};

    controller_msg.header = (HID_MSG_TYPE_DATA << 4) | (HID_REPORT_TYPE_INPUT & 0x3);
    controller_msg.report_id = HID_CONTROLLER_REPORT_ID;
    controller_msg.standardkey1 = 0x4f;
    bt_hid_send_report_req(bts2_app_data, conn_idx, sizeof(controller_msg), (U8 *)&controller_msg, TRUE);
}


/*
Description:
    send left control key report
Input:
    bts2_app_data:global app bt instance
    conn_idx:connection index
Time:2026/04/14 13:43:29

Author:zhengyu

Modify:
*/
void bt_hid_controller_report_left_arrow(bts2_app_stru *bts2_app_data, U8 conn_idx)
{
    hid_msg_controller_t controller_msg = {0};

    controller_msg.header = (HID_MSG_TYPE_DATA << 4) | (HID_REPORT_TYPE_INPUT & 0x3);
    controller_msg.report_id = HID_CONTROLLER_REPORT_ID;
    controller_msg.standardkey1 = 0x50;
    bt_hid_send_report_req(bts2_app_data, conn_idx, sizeof(controller_msg), (U8 *)&controller_msg, TRUE);
}


/*
Description:
    send up control key report
Input:
    bts2_app_data:global app bt instance
    conn_idx:connection index
Time:2026/04/14 13:43:29

Author:zhengyu

Modify:
*/
void bt_hid_controller_report_up_arrow(bts2_app_stru *bts2_app_data, U8 conn_idx)
{
    hid_msg_controller_t controller_msg = {0};

    controller_msg.header = (HID_MSG_TYPE_DATA << 4) | (HID_REPORT_TYPE_INPUT & 0x3);
    controller_msg.report_id = HID_CONTROLLER_REPORT_ID;
    controller_msg.standardkey1 = 0x52;
    bt_hid_send_report_req(bts2_app_data, conn_idx, sizeof(controller_msg), (U8 *)&controller_msg, TRUE);
}


/*
Description:
    send down control key report
Input:
    bts2_app_data:global app bt instance
    conn_idx:connection index
Time:2026/04/14 13:43:29

Author:zhengyu

Modify:
*/
void bt_hid_controller_report_down_arrow(bts2_app_stru *bts2_app_data, U8 conn_idx)
{
    hid_msg_controller_t controller_msg = {0};

    controller_msg.header = (HID_MSG_TYPE_DATA << 4) | (HID_REPORT_TYPE_INPUT & 0x3);
    controller_msg.report_id = HID_CONTROLLER_REPORT_ID;
    controller_msg.standardkey1 = 0x51;
    bt_hid_send_report_req(bts2_app_data, conn_idx, sizeof(controller_msg), (U8 *)&controller_msg, TRUE);
}


/*
Description:
    send controller reset report
Input:
    bts2_app_data:global app bt instance
    conn_idx:connection index
Time:2026/04/14 13:43:29

Author:zhengyu

Modify:
*/
void bt_hid_controller_report_reset(bts2_app_stru *bts2_app_data, U8 conn_idx)
{
    hid_msg_controller_t controller_msg = {0};

    controller_msg.header = (HID_MSG_TYPE_DATA << 4) | (HID_REPORT_TYPE_INPUT & 0x3);
    controller_msg.report_id = HID_CONTROLLER_REPORT_ID;
    bt_hid_send_report_req(bts2_app_data, conn_idx, sizeof(controller_msg), (U8 *)&controller_msg, TRUE);
}


/*
Description:
    hid device handle control packet
Input:
    bts2_app_data:global app bt instance
Time:2023/04/24 17:43:11

Author:zhengyu

Modify:
*/
static void bt_hid_receive_contro_handle(bts2_app_stru *bts2_app_data)
{
    //to do
    BTS2S_HID_CONTROL_IND *msg;
    msg = (BTS2S_HID_CONTROL_IND *)bts2_app_data->recv_msg;

    U8 conn_idx = CFG_MAX_HID_CONN_NUM;

    if (!bt_hid_check_connection_by_addr(msg->bd, &conn_idx))
    {
        USER_TRACE("[U-L]Why don't pre-alloction connection\n");
        return;
    }

    if (!bts2_app_data->hid_inst.conn[conn_idx].is_hid_device_role)
    {
        USER_TRACE("%s only hid device support this msg\n", __func__);
        return;
    }

    if (msg->param == HID_CONTROL_PARAM_SUSPEND)
    {
        if (bts2_app_data->hid_inst.conn[conn_idx].is_hid_device_role)
        {
            USER_TRACE("%s enter suspend mode\n", __func__);
        }
        else
        {
            USER_TRACE("%s invalid CONTROL param %d for hid host role\n", __func__, msg->param);
        }
    }
    else if (msg->param == HID_CONTROL_PARAM_EXIT_SUSPEND)
    {
        if (bts2_app_data->hid_inst.conn[conn_idx].is_hid_device_role)
        {
            USER_TRACE("%s exit suspend mode\n", __func__);
        }
        else
        {
            USER_TRACE("%s invalid CONTROL param %d for hid host role\n", __func__, msg->param);
        }
    }
    else if (msg->param == HID_CONTROL_PARAM_VIRTUAL_CABLE_UNPLUG)
    {
        bt_hid_disc_2_dev(&bts2_app_data->hid_inst.conn[conn_idx].rmt_bd);
    }
    else
    {
        USER_TRACE("%s unknown CONTROL param %d", __func__, msg->param);
    }
}


/*
Description:
    hid device handle get report packet from hid host
Input:
    bts2_app_data:global app bt instance
Time:2023/04/26 11:22:58

Author:zhengyu

Modify:
*/
static void hid_receive_get_report_handle(bts2_app_stru *bts2_app_data)
{

    BTS2S_HID_GET_REPORT_IND *msg;
    msg = (BTS2S_HID_GET_REPORT_IND *)bts2_app_data->recv_msg;
    BOOL send_portion_report_data = ((msg->param & (1 << 3)) != 0);
    U8 report_type = (msg->param & 0x3);
    U8 report_id = 0;
    BOOL has_report_id = FALSE;
    U16 portion_buffer_size = 0;
    struct hid_report_data_t *report_data;

    U8 conn_idx = CFG_MAX_HID_CONN_NUM;

    if (!bt_hid_check_connection_by_addr(msg->bd, &conn_idx))
    {
        USER_TRACE("[U-L]Why don't pre-alloction connection\n");
        return;
    }

    if (!bts2_app_data->hid_inst.conn[conn_idx].is_hid_device_role)
    {
        USER_TRACE("%s only hid device support this msg\n", __func__);
        return;
    }

    LOG_D("send_portion_report_data = %d\n", send_portion_report_data);
    if (send_portion_report_data)
    {
        if (msg->data_len == 2)
        {
            portion_buffer_size = msg->data[0] | (((U16)(msg->data[1])) << 8);
        }
        else if (msg->data_len == 3)
        {
            has_report_id = TRUE;
            report_id = msg->data[0];
            portion_buffer_size = msg->data[1] | (((U16)(msg->data[2])) << 8);
        }
        else
        {
            USER_TRACE("%s receive invalid data length (%d)\n", __func__, msg->data_len);
            hid_send_handshake(bts2_app_data, conn_idx, HID_HANDSHAKE_PARAM_TYPE_ERR_INVALID_PARAMETER);
            return;
        }
    }
    else
    {
        if (msg->data_len == 1)
        {
            has_report_id = TRUE;
            report_id = msg->data[0];
            if (report_id == 0xff)
            {
                USER_TRACE("%s receive invalid report id %d\n", __func__, report_id);
                hid_send_handshake(bts2_app_data, conn_idx,  HID_HANDSHAKE_PARAM_TYPE_ERR_INVALID_REPORT_ID);
                return;
            }
        }
        else
        {
            USER_TRACE("%s receive invalid data length <%d>\n", __func__, msg->data_len);
            hid_send_handshake(bts2_app_data, conn_idx,  HID_HANDSHAKE_PARAM_TYPE_ERR_INVALID_PARAMETER);
            return;
        }
    }

    switch ((hid_report_type_enum_t)report_type)
    {
    case HID_REPORT_TYPE_INPUT:
        report_data = (struct hid_report_data_t *)bcalloc(1, sizeof(struct hid_report_data_t));
        break;
    case HID_REPORT_TYPE_OUTPUT:
        report_data = (struct hid_report_data_t *)bcalloc(1, sizeof(struct hid_report_data_t));
        break;
    case HID_REPORT_TYPE_FEATURE:
        report_data = (struct hid_report_data_t *)bcalloc(1, sizeof(struct hid_report_data_t));
        break;
    default:
        USER_TRACE("%s receive invalid report type %d\n", __func__, report_type);
        hid_send_handshake(bts2_app_data, conn_idx,  HID_HANDSHAKE_PARAM_TYPE_ERR_INVALID_PARAMETER);
        return;
    }

    if (has_report_id)
    {
        if (portion_buffer_size > 0)
        {
            // portion_buffer_size include Report ID (if present) + Report Data
            portion_buffer_size -= 1; /* portion_buffer_size currently is report data length */
        }

        report_data->report_id = report_id;
    }

    if (send_portion_report_data && portion_buffer_size < report_data->data_len)
    {
        report_data->data_len = portion_buffer_size;
    }

    hid_send_response_report(bts2_app_data, conn_idx, report_type, report_data, has_report_id);
}


/*
Description:
    hid device response get report request from hid host
Input:
    bts2_app_data:global app bt instance
    conn_idx:connection index
    report_type:input/output/feature
    report_data:report buffer
    has_report_id:check report_id
Time:2023/04/26 13:45:40

Author:zhengyu

Modify:
*/
static void hid_send_response_report(bts2_app_stru *bts2_app_data, U8 conn_idx, hid_report_type_enum_t report_type, struct hid_report_data_t *report_data, BOOL has_report_id)
{
    struct hid_frame_t data_frame;
    data_frame.header = (HID_MSG_TYPE_DATA << 4) | (report_type & 0x3);

    if (bts2_app_data->hid_inst.conn[conn_idx].is_hid_device_role)
    {
        if (bts2_app_data->hid_inst.conn[conn_idx].local_protocol_mode == HID_REPORT_PROTOCOL_MODE)
        {
            if (has_report_id)
            {
                if (report_data->data_len > HID_FRAME_DATA_MAX_LEN)
                {
                    USER_TRACE("%s report data len too long\n", __func__);
                    bfree(report_data);
                    return;
                }
                memcpy(data_frame.data, report_data, report_data->data_len + 1);
                data_frame.data_len = report_data->data_len + 1;
            }
            else
            {
                if (report_data->data_len > HID_FRAME_DATA_MAX_LEN)
                {
                    USER_TRACE("%s report data len too long\n", __func__);
                    bfree(report_data);
                    return;
                }
                memcpy(data_frame.data, report_data->data, report_data->data_len);
                data_frame.data_len = report_data->data_len;
            }
        }
        else
        {
            if (report_data->report_id == HID_BOOT_PROTOCOL_KEYBOARD_REPORT_ID || report_data->report_id == HID_BOOT_PROTOCOL_MOUSE_REPORT_ID)
            {
                if (report_data->data_len > HID_FRAME_DATA_MAX_LEN)
                {
                    USER_TRACE("%s report data len too long\n", __func__);
                    bfree(report_data);
                    return;
                }
                memcpy(data_frame.data, report_data, report_data->data_len + 1);
                data_frame.data_len = report_data->data_len + 1;
            }
            else
            {
                USER_TRACE("%s unsupported boot protocol report id %d\n", __func__, report_data->report_id);
                bfree(report_data);
                return;
            }
        }
        bt_hid_send_report_req(bts2_app_data, conn_idx, data_frame.data_len + 1, (U8 *)&data_frame, FALSE);
    }
    bfree(report_data);
}


/*
Description:
    hid device handle set report packet from hid host
Input:
    bts2_app_data:global app bt instance
Time:2023/04/26 14:12:33

Author:zhengyu

Modify:
*/
static void hid_receive_set_report_handle(bts2_app_stru *bts2_app_data)
{
    BTS2S_HID_SET_REPORT_IND *msg;
    msg = (BTS2S_HID_SET_REPORT_IND *)bts2_app_data->recv_msg;

    BOOL local_report_descriptor_has_report_id = TRUE;
    U8 report_type = (msg->param & 0x3);
    U8 report_id = msg->data[0];;
    BOOL has_report_id = FALSE;
    U8 report_data_len = 0;
    struct hid_report_data_t *local_report_data = NULL;
    U8 *rx_report_data = NULL;

    U8 conn_idx = CFG_MAX_HID_CONN_NUM;

    if (!bt_hid_check_connection_by_addr(msg->bd, &conn_idx))
    {
        USER_TRACE("[U-L]Why don't pre-alloction connection\n");
        return;
    }


    if (!bts2_app_data->hid_inst.conn[conn_idx].is_hid_device_role)
    {
        USER_TRACE("%s only hid device support this msg\n", __func__);
        return;
    }

    if (bts2_app_data->hid_inst.conn[conn_idx].local_protocol_mode == HID_REPORT_PROTOCOL_MODE)
    {
        if (local_report_descriptor_has_report_id)
        {
            report_data_len = msg->data_len - 1; /* report->data[0] is reort_id */
            has_report_id = TRUE;
        }
        else
        {
            report_data_len = msg->data_len;
        }
    }
    else
    {
        if (report_id == HID_BOOT_PROTOCOL_KEYBOARD_REPORT_ID || report_id == HID_BOOT_PROTOCOL_MOUSE_REPORT_ID)
        {
            report_data_len = msg->data_len - 1; /* report->data[0] is reort_id */
            has_report_id = TRUE;
        }
        else
        {
            USER_TRACE("%s unsupported boot protocol report id %d\n", __func__, report_id);
            hid_send_handshake(bts2_app_data, conn_idx,  HID_HANDSHAKE_PARAM_TYPE_ERR_INVALID_REPORT_ID);
            return;
        }
    }

    if (report_data_len > HID_FRAME_DATA_MAX_LEN)
    {
        USER_TRACE("%s report data len too long\n", __func__);
        hid_send_handshake(bts2_app_data, conn_idx,  HID_HANDSHAKE_PARAM_TYPE_ERR_INVALID_PARAMETER);
        return;
    }

    switch ((hid_report_type_enum_t)report_type)
    {
    case HID_REPORT_TYPE_INPUT:
        local_report_data = (struct hid_report_data_t *)bcalloc(1, sizeof(struct hid_report_data_t));
        bts2_app_data->hid_inst.conn[conn_idx].device_input_data = (void *)local_report_data;
        break;
    case HID_REPORT_TYPE_OUTPUT:
        local_report_data = (struct hid_report_data_t *)bcalloc(1, sizeof(struct hid_report_data_t));
        bts2_app_data->hid_inst.conn[conn_idx].device_output_data = (void *)local_report_data;
        break;
    case HID_REPORT_TYPE_FEATURE:
        local_report_data = (struct hid_report_data_t *)bcalloc(1, sizeof(struct hid_report_data_t));
        bts2_app_data->hid_inst.conn[conn_idx].device_feature_data = (void *)local_report_data;
        break;
    default:
        USER_TRACE("%s receive invalid report type %d\n", __func__, report_type);
        hid_send_handshake(bts2_app_data, conn_idx,  HID_HANDSHAKE_PARAM_TYPE_ERR_INVALID_PARAMETER);
        return;
    }

    if (has_report_id)
    {
        rx_report_data = msg->data + 1;
    }
    else
    {
        rx_report_data = msg->data;
    }

    USER_TRACE("%s report_id %d report_type %d length %d\n", __func__, report_id, report_type, report_data_len);

    memcpy(local_report_data->data, rx_report_data, report_data_len);
    local_report_data->data_len = report_data_len;

    hid_send_handshake(bts2_app_data, conn_idx,  HID_HANDSHAKE_PARAM_TYPE_SUCCESSFUL);
}


/*
Description:
    hid device handle get protocol packet from hid host
Input:
    bts2_app_data:global app bt instance
Time:2023/04/26 14:23:09

Author:zhengyu

Modify:
*/
static void hid_receive_get_protocol_handle(bts2_app_stru *bts2_app_data)
{
    BTS2S_GET_PROTOCOL_IND *msg;
    msg = (BTS2S_GET_PROTOCOL_IND *)bts2_app_data->recv_msg;

    U8 conn_idx = CFG_MAX_HID_CONN_NUM;

    if (!bt_hid_check_connection_by_addr(msg->bd, &conn_idx))
    {
        USER_TRACE("[U-L]Why don't pre-alloction connection\n");
        return;
    }

    struct hid_frame_t data_frame;

    if (!bts2_app_data->hid_inst.conn[conn_idx].is_hid_device_role)
    {
        USER_TRACE("%s only hid device support this msg\n", __func__);
        return;
    }

    data_frame.header = (HID_MSG_TYPE_DATA << 4) | (HID_REPORT_TYPE_RESERVED & 0x3);
    data_frame.data[0] = (bts2_app_data->hid_inst.conn[conn_idx].local_protocol_mode & 0x3);
    data_frame.data_len = 1;

    bt_hid_send_report_req(bts2_app_data, conn_idx, data_frame.data_len + 1, (U8 *)&data_frame, FALSE);
}


/*
Description:
    hid device handle set protocol packet from hid host
Input:
    bts2_app_data:global app bt instance
Time:2023/04/26 14:23:09

Author:zhengyu

Modify:
*/
static void hid_receive_set_protocol_handle(bts2_app_stru *bts2_app_data)
{
    BTS2S_SET_PROTOCOL_IND *msg;
    msg = (BTS2S_SET_PROTOCOL_IND *)bts2_app_data->recv_msg;

    U8 conn_idx = CFG_MAX_HID_CONN_NUM;

    if (!bt_hid_check_connection_by_addr(msg->bd, &conn_idx))
    {
        USER_TRACE("[U-L]Why don't pre-alloction connection\n");
        return;
    }

    U8 protocol_mode = (msg->param & 0x3);

    if (!bts2_app_data->hid_inst.conn[conn_idx].is_hid_device_role)
    {
        USER_TRACE("%s only hid device support this msg\n", __func__);
        return;
    }

    if (protocol_mode == HID_REPORT_PROTOCOL_MODE || protocol_mode == HID_BOOT_PROTOCOL_MODE)
    {
        USER_TRACE("%s set protocol mode %d\n", __func__, protocol_mode);
        bts2_app_data->hid_inst.conn[conn_idx].local_protocol_mode = protocol_mode;
        hid_send_handshake(bts2_app_data, conn_idx,  HID_HANDSHAKE_PARAM_TYPE_SUCCESSFUL);
    }
    else
    {
        USER_TRACE("%s invalid protocol mode %d\n", __func__, protocol_mode);
        hid_send_handshake(bts2_app_data, conn_idx,  HID_HANDSHAKE_PARAM_TYPE_ERR_INVALID_PARAMETER);
    }
}


/*
Description:
    hid device handle interrupt report packet from hid host
Input:
    bts2_app_data:global app bt instance
Time:2023/04/26 15:03:18

Author:zhengyu

Modify:
*/
static void hid_receive_interrupt_report(bts2_app_stru *bts2_app_data)
{
    BTS2S_HID_DATA_IND *msg;
    msg = (BTS2S_HID_DATA_IND *)bts2_app_data->recv_msg;

    U8 conn_idx = CFG_MAX_HID_CONN_NUM;

    if (!bt_hid_check_connection_by_addr(msg->bd, &conn_idx))
    {
        USER_TRACE("[U-L]Why don't pre-alloction connection\n");
        return;
    }

    BOOL local_report_descriptor_has_report_id = TRUE;
    U8 report_type = (msg->param & 0x3);
    U8 report_id = msg->data[0];;
    BOOL has_report_id = FALSE;
    U8 report_data_len = 0;
    struct hid_report_data_t *local_report_data = NULL;
    U8 *rx_report_data = NULL;

    if (bts2_app_data->hid_inst.conn[conn_idx].is_hid_device_role)
    {
        switch ((hid_report_type_enum_t)report_type)
        {
        case HID_REPORT_TYPE_OUTPUT:
            local_report_data = (struct hid_report_data_t *)bcalloc(1, sizeof(struct hid_report_data_t));
            bts2_app_data->hid_inst.conn[conn_idx].device_output_data = (void *)local_report_data;
            break;
        case HID_REPORT_TYPE_FEATURE:
            local_report_data = (struct hid_report_data_t *)bcalloc(1, sizeof(struct hid_report_data_t));
            bts2_app_data->hid_inst.conn[conn_idx].device_feature_data = (void *)local_report_data;
            break;
        default:
            USER_TRACE("%s invalid report_type %d for hid device\n", __func__, report_type);
            return;
        }

        if (bts2_app_data->hid_inst.conn[conn_idx].local_protocol_mode == HID_REPORT_PROTOCOL_MODE)
        {
            if (local_report_descriptor_has_report_id)
            {
                report_data_len = msg->data_len - 1; /* report->data[0] is reort_id */
                has_report_id = TRUE;
            }
            else
            {
                report_data_len = msg->data_len;
            }
        }
        else
        {
            if (report_id == HID_BOOT_PROTOCOL_KEYBOARD_REPORT_ID || report_id == HID_BOOT_PROTOCOL_MOUSE_REPORT_ID)
            {
                report_data_len = msg->data_len - 1; /* report->data[0] is reort_id */
                has_report_id = TRUE;
            }
            else
            {
                USER_TRACE("%s unsupported boot protocol report id %d\n", __func__, report_id);
                return;
            }
        }

        if (report_data_len > HID_FRAME_DATA_MAX_LEN)
        {
            USER_TRACE("%s report data len too long\n", __func__);
            return;
        }

        if (has_report_id)
        {
            rx_report_data = msg->data + 1;
        }
        else
        {
            rx_report_data = msg->data;
        }

        USER_TRACE("%s report_id %d report_type %d length %d\n", __func__, report_id, report_type, report_data_len);

        memcpy(local_report_data->data, rx_report_data, report_data_len);
        local_report_data->data_len = report_data_len;
    }
}


/*
Description:
    hid device send handshake packet to hid host
Input:
    bts2_app_data:global app bt instance
    conn_idx:connection index
    code:results code
Time:2023/04/23 20:09:03

Author:zhengyu

Modify:
*/
void hid_send_handshake(bts2_app_stru *bts2_app_data, U8 conn_idx, hid_handshake_param_type_enum_t code)
{
    struct hid_frame_t data_frame;

    if (!bts2_app_data->hid_inst.conn[conn_idx].is_hid_device_role)
    {
        USER_TRACE("%s only hid device support this msg\n", __func__);
        return;
    }

    data_frame.header = (HID_MSG_TYPE_HANDSHAKE << 4) | (code & 0xf);
    data_frame.data_len = 0;

    bt_hid_send_report_req(bts2_app_data, conn_idx, data_frame.data_len + 1, (U8 *)&data_frame, FALSE);
}


/*
Description:
    handle the message from hid stack
Input:
    bts2_app_data:global app bt instance
Time:2023/04/26 09:24:42

Author:zhengyu

Modify:
*/
void bt_hid_msg_handler(bts2_app_stru *bts2_app_data)
{
    U16 *msg_type;
    msg_type = (U16 *)bts2_app_data->recv_msg;
    switch (*msg_type)
    {
    case BTS2MU_HID_ENB_CFM:
    {
        BTS2S_HID_ENB_CFM *msg;
        msg = (BTS2S_HID_ENB_CFM *)bts2_app_data->recv_msg;

        if (msg->res == BTS2_SUCC)
        {
            USER_TRACE(">> HID enabled\n");
            bt_interface_bt_event_notify(BT_NOTIFY_HID, BT_NOTIFY_HID_OPEN_COMPLETE, NULL, 0);
            INFO_TRACE(">> URC hid open\n");
        }
        break;
    }
    case BTS2MU_HID_DISB_CFM:
    {
        BTS2S_HID_DISB_CFM *msg;
        msg = (BTS2S_HID_DISB_CFM *)bts2_app_data->recv_msg;

        if (msg->res == BTS2_SUCC)
        {
            USER_TRACE("[L-U]hid disable success\n");
        }
        break;
    }
    case BTS2MU_HID_CONN_CFM :
    {
        bt_hid_hdl_conn_cfm(bts2_app_data);
        break;
    }
    case BTS2MU_HID_CONN_IND:
    {
        BTS2S_HID_CONN_IND *msg;
        msg = (BTS2S_HID_CONN_IND *)bts2_app_data->recv_msg;
        if (msg->local_psm == BT_PSM_HID_CTRL)
        {
            USER_TRACE("[L-U]remote connect HID control channel success\n");
        }
        else if (msg->local_psm == BT_PSM_HID_INTR)
        {

            U8 idx = CFG_MAX_HID_CONN_NUM;

            if (bt_hid_check_connection_by_addr(&msg->bd, &idx))
            {
                USER_TRACE("[U-L] why receive repeated connection...\n");
                hid_disc_req_ext(&msg->bd);
                break;
            }
            else
            {
                idx = bt_hid_get_available_connection();

                if (idx == CFG_MAX_HID_CONN_NUM)
                {
                    USER_TRACE("The connection is full\n");
                    hid_disc_req_ext(&msg->bd);
                    break;
                }
                else
                {
                    bd_copy(&bts2_app_data->hid_inst.conn[idx].rmt_bd, &msg->bd);
                    bts2_app_data->hid_inst.conn[idx].is_hid_device_role = TRUE;
                }

                USER_TRACE("[L-U]remote connect HID interrupt channel success\n");
                bt_notify_profile_state_info_t profile_state;
                bt_addr_convert(&msg->bd, profile_state.mac.addr);
                profile_state.profile_type = BT_NOTIFY_HID;
                profile_state.res = BTS2_SUCC;
                bt_profile_update_connection_state(BT_NOTIFY_HID, BT_NOTIFY_HID_PROFILE_CONNECTED, &profile_state);

                bt_hid_mouse_reset_at_middle(bts2_app_data, idx);
            }
        }
        break;
    }
    case BTS2MU_HID_DISC_CFM:
    {
        bt_hid_hdl_disconn_cfm(bts2_app_data);
        break;
    }
    case BTS2MU_HID_DISC_IND:
    {
        bt_hid_hdl_disconn_ind(bts2_app_data);
        break;
    }
    case BTS2MU_HID_MODE_CHANGE_IND:
    {
        BTS2S_HID_MODE_CHANGE_IND *msg;
        msg = (BTS2S_HID_MODE_CHANGE_IND *)bts2_app_data->recv_msg;

        U8 idx = CFG_MAX_HID_CONN_NUM;

        if (bt_hid_check_connection_by_addr(msg->bd, &idx))
        {
            bts2_app_data->hid_inst.conn[idx].mode = msg->mode;
            USER_TRACE("[L-U]hid mode change to %d\n", bts2_app_data->hid_inst.conn[idx].mode);
        }
        break;
    }
    case BTS2MU_HID_CONTROL_IND:
    {
        bt_hid_receive_contro_handle(bts2_app_data);
        break;
    }
    case BTS2MU_HID_GET_REPORT_IND:
    {
        hid_receive_get_report_handle(bts2_app_data);
        break;
    }
    case BTS2MU_HID_SET_REPORT_IND:
    {
        hid_receive_set_report_handle(bts2_app_data);
        break;
    }
    case BTS2MU_HID_GET_PROTOCOL_IND:
    {
        hid_receive_get_protocol_handle(bts2_app_data);
        break;
    }
    case BTS2MU_HID_SET_PROTOCOL_IND:
    {
        hid_receive_set_protocol_handle(bts2_app_data);
        break;
    }
    case BTS2MD_HID_DATA_IND:
    {
        hid_receive_interrupt_report(bts2_app_data);
        break;
    }
    case BTS2MD_HID_UNKNOWN:
    {
        BTS2S_HID_DATA_IND *msg;
        msg = (BTS2S_HID_DATA_IND *)bts2_app_data->recv_msg;

        U8 conn_idx = CFG_MAX_HID_CONN_NUM;

        if (!bt_hid_check_connection_by_addr(msg->bd, &conn_idx))
        {
            USER_TRACE("[U-L]Why don't pre-alloction connection\n");
            return;
        }
        hid_send_handshake(bts2_app_data, conn_idx,  HID_HANDSHAKE_PARAM_TYPE_ERR_UNSUPPORTED_REQUEST);
        break;
    }
    default:
        break;
    }

}


//*/************************************************hid test function***************************************************************/
static S16 dx;
static S16 dy;
static U16 buttons;


/*
Description:
    test hid function
Input:
    global app bt instance
Time:2023/04/25 15:51:12

Author:zhengyu

Modify:
*/
void bt_hid_mouse_test1(bts2_app_stru *bts2_app_data, U8 con_idx)
{
    switch (bts2_app_data->input_str[0])
    {
    case 'w':
        dy -= MOUSE_SPEED;
        bt_hid_mouse_move(bts2_app_data, 0, dy, con_idx);
        break;
    case 'a':
        dx -= MOUSE_SPEED;
        bt_hid_mouse_move(bts2_app_data, dx, 0, con_idx);
        break;
    case 's':
        dy += MOUSE_SPEED;
        bt_hid_mouse_move(bts2_app_data, 0, dy, con_idx);
        break;
    case 'd':
        dx += MOUSE_SPEED;
        bt_hid_mouse_move(bts2_app_data, dx, 0, con_idx);
        break;
    case 'L':
        bt_hid_mouse_left_click(bts2_app_data, con_idx);
        break;
    case 'R':
        bt_hid_mouse_right_click(bts2_app_data, con_idx);
        break;
    case 'O':
        bt_hid_mouse_middle_button_click(bts2_app_data, con_idx);
        break;

    default:
        break;
    }

    dx = 0;
    dy = 0;
}


void bt_hid_mouse_test2(bts2_app_stru *bts2_app_data, U8 con_idx)
{
    bt_hid_mouse_drag_page_up(bts2_app_data, con_idx);
}


void bt_hid_mouse_test3(bts2_app_stru *bts2_app_data, U8 con_idx)
{
    bt_hid_mouse_drag_page_down(bts2_app_data, con_idx);
}


void bt_hid_mouse_test4(bts2_app_stru *bts2_app_data, U8 con_idx)
{
    bt_hid_mouse_left_double_click(bts2_app_data, con_idx);
}


void bt_hid_mouse_test5(bts2_app_stru *bts2_app_data, U8 con_idx)
{
    bt_hid_mouse_right_double_click(bts2_app_data, con_idx);
}


void bt_hid_mouse_test6(bts2_app_stru *bts2_app_data, U8 con_idx)
{
    bt_hid_mouse_reset_at_middle(bts2_app_data, con_idx);
}


void bt_hid_mouse_test7(bts2_app_stru *bts2_app_data, U8 con_idx)
{
    bts2g_app_p->hid_inst.conn[con_idx].is_device_ios = TRUE;
    bt_hid_mouse_reset_at_middle(bts2_app_data, con_idx);
    LOG_D("is_device_ios = %d\n", bts2g_app_p->hid_inst.conn[con_idx].is_device_ios);
}


void bt_hid_mouse_test8(bts2_app_stru *bts2_app_data, U8 con_idx)
{
    bts2g_app_p->hid_inst.conn[con_idx].is_device_ios = FALSE;
    bt_hid_mouse_reset_at_middle(bts2_app_data, con_idx);
    LOG_I("is_device_ios = %d\n", bts2g_app_p->hid_inst.conn[con_idx].is_device_ios);
}


void bt_hid_mouse_test9(bts2_app_stru *bts2_app_data, U8 con_idx)
{
    switch (bts2_app_data->input_str[0])
    {
    case '7':
        bt_hid_consumer_report_play_status(bts2_app_data, con_idx);
        break;
    case '8':
        bt_hid_consumer_report_next_track(bts2_app_data, con_idx);
        break;
    case '9':
        bt_hid_consumer_report_back_track(bts2_app_data, con_idx);
        break;
    case '+':
        bt_hid_consumer_report_volume_up(bts2_app_data, con_idx);
        break;
    case '-':
        bt_hid_consumer_report_volume_down(bts2_app_data, con_idx);
        break;

    default:
        break;
    }
}


//*/************************************************public api***************************************************************/
/*
Description:
    check connection exist by addr
Input:
    bd:bd address pointer
    idx:connection index
Time:2026/04/14 14:00:42

Author:zhengyu

Modify:
*/
BOOL bt_hid_check_connection_by_addr(BTS2S_BD_ADDR *bd, U8 *idx)
{
    bts2_app_stru *bts2_app_data = bts2g_app_p;

    for (U8 i = 0; i < CFG_MAX_HID_CONN_NUM; i++)
    {
        if (bd_eq(&bts2_app_data->hid_inst.conn[i].rmt_bd, bd))
        {
            *idx = i;
            return TRUE;
        }
    }

    return FALSE;
}


/*
Description:
    get available connection
Input:

Time:2026/04/14 14:02:17

Author:zhengyu

Modify:
*/
U8 bt_hid_get_available_connection(void)
{
    U8 idx;
    bts2_app_stru *bts2_app_data = bts2g_app_p;

    for (idx = 0; idx < CFG_MAX_HID_CONN_NUM; idx++)
    {
        if (bd_is_empty(&bts2_app_data->hid_inst.conn[idx].rmt_bd))
        {
            break;
        }
    }

    return idx;
}

#endif
