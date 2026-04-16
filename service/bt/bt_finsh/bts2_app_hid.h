/*
 * SPDX-FileCopyrightText: 2019-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _BTS2_APP_HID_H_
#define _BTS2_APP_HID_H_



#ifdef __cplusplus
extern "C" {
#endif

#ifdef BSP_USING_PC_SIMULATOR
#define __packed
#endif

#include "bf0_hal_def.h"

#ifdef  CFG_HID

typedef struct
{
    U8 header;
    U8 report_id;
    U8 buttons;
    S16 dx: 12;
    S16 dy: 12;
    S8 wheel;
    U8 ac_pan;
} __packed hid_msg_mouse_t;


typedef struct
{
    U8 header;
    U8 report_id;
    U8 consumer;
} __packed hid_msg_consumer_t;

typedef struct
{
    U8 header;
    U8 report_id;
    U8 modifierKeys;
    U8 standardkey1;
    U8 standardkey2;
    U8 standardkey3;
    U8 standardkey4;
    U8 standardkey5;
    U8 standardkey6;
} __packed hid_msg_controller_t;

typedef struct
{
    U8 report_id;
    U8 data[9];
} hid_msg_touch_t;


#define HID_FRAME_DATA_MAX_LEN 11 /* DM1 is recommended packet */
struct hid_report_data_t
{
    U8 report_id;
    U8 data[HID_FRAME_DATA_MAX_LEN];
    U8 data_len;
};

struct hid_frame_t
{
    U8 header;
    U8 data[HID_FRAME_DATA_MAX_LEN];
    U8 data_len;
};

/*
Description:
    handle the message from hid stack
Input:
    bts2_app_data:global app bt instance
Time:2026/04/20 09:24:42

Author:zhengyu

Modify:
*/
extern void bt_hid_msg_handler(bts2_app_stru *bts2_app_data);

/*
Description:
    hid profile init
Input:
    global app bt instance
Time:2026/04/20 14:10:33

Author:zhengyu

Modify:
*/
void bt_hid_init(bts2_app_stru *bts2_app_data);

/*
Description:
    open the hid
Input:
    null
Time:2026/04/20 10:15:42

Author:zhengyu

Modify:
*/
void bt_hid_open(void);

/*
Description:
    Combined hid descriptor
Input:

Time:2026/04/08 17:29:17

Author:zhengyu

Modify:
*/
void bt_hid_cmpose_hid_descriptor(void);

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
void bt_hid_add_descriptor(const U8 *data, U8 len);

/*
Description:
    close the hid
Input:
    null
Time:2026/04/20 10:15:42

Author:zhengyu

Modify:
*/
void bt_hid_close(void);

/*
Description:
    connect remote device hid profile
Input:
    global app bt instance
Time:2026/04/20 14:12:35

Author:zhengyu

Modify:
*/
void bt_hid_connect_requset(BTS2S_BD_ADDR *bd);

/*
Description:
    disconnect remote device hid profile
Input:
    global app bt instance
Time:2026/04/20 14:13:53

Author:zhengyu

Modify:
*/
void bt_hid_disc_2_dev(BTS2S_BD_ADDR *bd_addr);

/*
Description:
    hid device control the mobile drag down once
Input:
    bts2_app_data:global app bt instance
    dx:X-direction offset
    dy:Y-direction offset
Time:2026/04/20 15:07:58

Author:zhengyu

Modify:
*/
void bt_hid_mouse_move(bts2_app_stru *bts2_app_data, S16 dx, S16 dy, U8 conn_idx);

/*
Description:
    hid device control the mobile click
Input:
    bts2_app_data:global app bt instance
    conn_idx:connection index
Time:2026/04/2015:07:58

Author:zhengyu

Modify:
*/
void bt_hid_mouse_left_click(bts2_app_stru *bts2_app_data, U8 conn_idx);

/*
Description:
    hid device control the mobile bakeup
Input:
    bts2_app_data:global app bt instance
    conn_idx:connection index
Time:2026/04/20 15:07:58

Author:zhengyu

Modify:
*/
void bt_hid_mouse_right_click(bts2_app_stru *bts2_app_data, U8 conn_idx);

/*
Description:
    hid device control the mobile bakeup home
Input:
    bts2_app_data:global app bt instance
    conn_idx:connection index
Time:2026/04/20 15:07:58

Author:zhengyu

Modify:
*/
void bt_hid_mouse_middle_button_click(bts2_app_stru *bts2_app_data, U8 conn_idx);

/*
Description:
    hid device control the mobile left double click
Input:
    bts2_app_data:global app bt instance
    conn_idx:connection index
Time:2026/04/20 15:07:58

Author:zhengyu

Modify:
*/
void bt_hid_mouse_left_double_click(bts2_app_stru *bts2_app_data, U8 conn_idx);

/*
Description:
    hid device control the mobile left double click
Input:
    parameter:bd address pointer
Time:2026/04/20 15:07:58

Author:zhengyu

Modify:
*/
void bt_hid_timeout_handler(void *parameter);

/*
Description:
    hid device control the mobile right double click
Input:
    bts2_app_data:global app bt instance
    conn_idx:connection index
Time:2026/04/20 09:23:13

Author:zhengyu

Modify:
*/
void bt_hid_mouse_right_double_click(bts2_app_stru *bts2_app_data, U8 conn_idx);

/*
Description:
    hid device control the mobile drag once
Input:
    bts2_app_data:global app bt instance
    conn_idx:connection index
    dx:X-direction move
    dy:Y-direction move
    wheel_offset:mouse wheel movement
Time:2026/04/20 15:07:58

Author:zhengyu

Modify:
*/
void bt_hid_mouse_drag_page(bts2_app_stru *bts2_app_data, U8 conn_idx, U8 buttons, S16 dx, S16 dy, S8 wheel_offset);

/*
Description:
    hid device control the mobile drag up once
Input:
    bts2_app_data:global app bt instance
    conn_idx:connection index
Time:2026/04/20 14:07:00

Author:zhengyu

Modify:
*/
void bt_hid_mouse_drag_page_up(bts2_app_stru *bts2_app_data, U8 conn_idx);

/*
Description:
    hid device control the mobile drag down once
Input:
    bts2_app_data:global app bt instance
    conn_idx:connection index
Time:2026/04/20 14:07:00

Author:zhengyu

Modify:
*/
void bt_hid_mouse_drag_page_down(bts2_app_stru *bts2_app_data, U8 conn_idx);

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
void bt_hid_set_ios_device(BTS2S_BD_ADDR *bd_addr, U8 is_ios);

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
void bt_hid_consumer_report_power_onoff(bts2_app_stru *bts2_app_data, U8 conn_idx);

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
void bt_hid_consumer_report_play_status(bts2_app_stru *bts2_app_data, U8 conn_idx);

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
void bt_hid_consumer_report_next_track(bts2_app_stru *bts2_app_data, U8 conn_idx);

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
void bt_hid_consumer_report_back_track(bts2_app_stru *bts2_app_data, U8 conn_idx);

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
void bt_hid_consumer_report_volume_down(bts2_app_stru *bts2_app_data, U8 conn_idx);

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
void bt_hid_consumer_report_volume_up(bts2_app_stru *bts2_app_data, U8 conn_idx);

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
void bt_hid_consumer_report_forward(bts2_app_stru *bts2_app_data, U8 conn_idx);

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
void bt_hid_consumer_report_go_back(bts2_app_stru *bts2_app_data, U8 conn_idx);

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
void bt_hid_controller_report_right_arrow(bts2_app_stru *bts2_app_data, U8 conn_idx);

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
void bt_hid_controller_report_left_arrow(bts2_app_stru *bts2_app_data, U8 conn_idx);

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
void bt_hid_controller_report_up_arrow(bts2_app_stru *bts2_app_data, U8 conn_idx);

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
void bt_hid_controller_report_down_arrow(bts2_app_stru *bts2_app_data, U8 conn_idx);

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
void bt_hid_controller_report_reset(bts2_app_stru *bts2_app_data, U8 conn_idx);

/*
Description:
    hid device send handshake packet to hid host
Input:
    bts2_app_data:global app bt instance
    conn_idx:connection index
    code:results code
Time:2026/04/20 20:09:03

Author:zhengyu

Modify:
*/
void hid_send_handshake(bts2_app_stru *bts2_app_data, U8 conn_idx, hid_handshake_param_type_enum_t code);

/*
Description:
    test hid function
Input:
    global app bt instance
Time:2026/04/20 15:51:12

Author:zhengyu

Modify:
*/
void bt_hid_mouse_test1(bts2_app_stru *bts2_app_data, U8 con_idx);

void bt_hid_mouse_test2(bts2_app_stru *bts2_app_data, U8 con_idx);

void bt_hid_mouse_test3(bts2_app_stru *bts2_app_data, U8 con_idx);

void bt_hid_mouse_test4(bts2_app_stru *bts2_app_data, U8 con_idx);

void bt_hid_mouse_test5(bts2_app_stru *bts2_app_data, U8 con_idx);

void bt_hid_mouse_test6(bts2_app_stru *bts2_app_data, U8 con_idx);

void bt_hid_mouse_test7(bts2_app_stru *bts2_app_data, U8 con_idx);

void bt_hid_mouse_test8(bts2_app_stru *bts2_app_data, U8 con_idx);

void bt_hid_mouse_test9(bts2_app_stru *bts2_app_data, U8 con_idx);

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
BOOL bt_hid_check_connection_by_addr(BTS2S_BD_ADDR *bd, U8 *idx);

/*
Description:
    get available connection
Input:

Time:2026/04/14 14:02:17

Author:zhengyu

Modify:
*/
U8 bt_hid_get_available_connection(void);

#endif

#ifdef __cplusplus
}
#endif

#endif
