/*
 * SPDX-FileCopyrightText: 2022-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <stdio.h>
#include <string.h>
#include "drv_bt.h"
#include "bt_rt_device.h"
#include "bts2_global.h"
#include "bts2_app_inc.h"
#include "bf0_ble_common.h"
#include "bf0_sibles.h"
#include "bt_connection_manager.h"

#define DBG_TAG               "bt_rt_device.control_avrcp"
//#define DBG_LVL               DBG_INFO
#include <log.h>


#ifdef BT_USING_AVRCP
bt_err_t bt_sifli_set_avrcp_volume(rt_bt_device_t *dev, bt_volume_set_t *set)
{
    int volume, temp_volume;
    temp_volume = set->volume.media_volume;
    bt_err_t ret = BT_EOK;
    uint8_t max_vol = 15;
#ifdef     AUDIO_USING_MANAGER
    max_vol = audio_server_get_max_volume();
#endif // AUDIO_USING_MANAGER
    volume = bt_interface_avrcp_local_vol_2_abs_vol(temp_volume, max_vol);

#ifndef BT_CONNECT_SUPPORT_MULTI_LINK
    if (BT_STATE_CONNECTED == rt_bt_get_connect_state(dev, BT_PROFILE_AVRCP))
#endif
    {
        if (BT_ROLE_MASTER == dev->role)
        {
            ret = bt_interface_avrcp_set_absolute_volume_as_tg_role(volume);
        }
        else
        {
            ret = bt_interface_avrcp_set_absolute_volume_as_ct_role(volume);
        }
    }

    if (BT_EOK == ret)
    {
#ifdef AUDIO_USING_MANAGER
        audio_server_set_private_volume(AUDIO_TYPE_BT_MUSIC, temp_volume);
#endif // AUDIO_USING_MANAGER
    }

    return ret;
}
#endif

bt_err_t bt_sifli_control_avrcp(struct rt_bt_device *bt_handle, int cmd, void *args)
{
    bt_err_t ret = BT_EOK;

    switch (cmd)
    {
    case BT_CONTROL_OPEN_AVRCP:
    {
        LOG_I("open avrcp ,event %x", bt_sifli_get_bt_event());
        if (bt_sifli_check_bt_event(BT_SET_AVRCP_CLOSE_EVENT))
        {
            LOG_I("during avrcp close porcess");
            return BT_ERROR_STATE;
        }
        else
        {
            bt_sifli_set_bt_event(BT_SET_AVRCP_OPEN_EVENT);
            bt_interface_open_avrcp();
        }
    }
    break;

    case BT_CONTROL_CLOSE_AVRCP:
    {
        LOG_I("close avrcp ,event %x", bt_sifli_get_bt_event());
        if (bt_sifli_check_bt_event(BT_SET_AVRCP_OPEN_EVENT))
        {
            LOG_I("during avrcp open porcess");
            return BT_ERROR_STATE;
        }
        else
        {
            bt_sifli_set_bt_event(BT_SET_AVRCP_CLOSE_EVENT);
            bt_interface_close_avrcp();
        }
    }
    break;
    case BT_CONTROL_PHONE_PLAY_NEXT:
    {
        bt_interface_avrcp_next();
    }
    break;

    case BT_CONTROL_PHONE_PLAY:
    {
        bt_interface_avrcp_play();
    }
    break;

    case BT_CONTROL_PHONE_PLAY_SUSPEND:
    {
        bt_interface_avrcp_pause();
    }
    break;

    case BT_CONTROL_PHONE_PLAY_STOP:
    {
        bt_interface_avrcp_stop();
    }
    break;

    case BT_CONTROL_AVRCP_VOLUME_UP:
    {
        bt_interface_avrcp_volume_up();
    }
    break;

    case BT_CONTROL_AVRCP_VOLUME_DOWN:
    {
        bt_interface_avrcp_volume_down();
    }
    break;

    case BT_CONTROL_PHONE_PLAY_PREVIOUS:
    {
        bt_interface_avrcp_previous();
    }
    break;

    default:
        ret = BT_ERROR_UNSUPPORTED;
        break;
    }
    return ret;
}

