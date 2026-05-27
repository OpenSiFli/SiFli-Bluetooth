/*
 * SPDX-FileCopyrightText: 2019-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _BTS2_APP_AVRCP_H_
#define _BTS2_APP_AVRCP_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CFG_MS
#ifdef _CTRLLER_DLL
#undef  _CTRLLER_EXPORT
#define _CTRLLER_EXPORT __declspec(dllexport)
#else
#undef  _CTRLLER_EXPORT
#define _CTRLLER_EXPORT __declspec(dllimport)
#endif
#endif

// supp cmds
#define BT_AVRCP_PLY             0x01
#define BT_AVRCP_STOP            0x02
#define BT_AVRCP_PAUSE           0x03
#define BT_AVRCP_FORWARD         0x04
#define BT_AVRCP_BACKWARD        0x05
#define BT_AVRCP_VOLUME_UP       0x06
#define BT_AVRCP_VOLUME_DOWN     0x07
#define BT_AVRCP_RECORD          0x08
#define BT_AVRCP_FAST_FORWARD    0x09
#define BT_AVRCP_MUTE            0x0a


#define PLY_PASS_THROUGH          (0x44)
#define STOP_PASS_THROUGH         (0x45)
#define PAUSE_PASS_THROUGH        (0x46)
#define FORWARD_PASS_THROUGH      (0x4b)
#define BACKWARD_PASS_THROUGH     (0x4c)

#define AVRCP_MAX_SONG_NAME_LEN    (128)
#define AVRCP_MAX_SINGER_NAME_LEN  (128)
#define AVRCP_MAX_ALBUM_INFO_LEN   (128)
#define AVRCP_MAX_PLAY_TIME_LEN    (8)

#ifdef CFG_MS
_CTRLLER_EXPORT int AVRCP_ctrl_API(WORD n_cmd_code,  LPVOID lp_param);

typedef int (*PAVRCP_ctrl_API)(WORD n_cmd_code,  LPVOID lp_param);
#endif

#ifdef  CFG_AVRCP


typedef enum
{
    AVRCP_TG_NFY_BV_05_C = 100,
    AVRCP_TG_NFY_BV_08_C,
} AVRCP_BQB_TEST;

typedef struct
{
    uint32_t size;
    uint8_t song_name[AVRCP_MAX_SONG_NAME_LEN];
} bt_avrcp_music_song_name_t;

typedef struct
{
    uint32_t size;
    uint8_t singer_name[AVRCP_MAX_SINGER_NAME_LEN];
} bt_avrcp_music_singer_name_t;


typedef struct
{
    uint32_t size;
    uint8_t album_name[AVRCP_MAX_ALBUM_INFO_LEN];
} bt_avrcp_music_album_info_t;

typedef struct
{
    uint32_t size;
    uint8_t play_time[AVRCP_MAX_PLAY_TIME_LEN];//ascii code  ,unit:ms
} bt_avrcp_music_play_time_t;

typedef struct
{
    uint32_t  song_total_size;          /**< the song's total length */
    bt_avrcp_music_play_time_t duration;                  /**< the song's total duration */
    bt_avrcp_music_song_name_t song_name;          /**< the song's name */
    bt_avrcp_music_singer_name_t singer_name;      /**< the song's singer name */
    bt_avrcp_music_album_info_t album_info;        /**< the song's album name */
    uint16_t          character_set_id;  //UTF-8 0x006A; other??
} bt_avrcp_music_detail_info_t;

typedef struct
{
    uint8_t  track_id;
    uint32_t  attri_req;
    bt_avrcp_music_detail_info_t detail_info;
} bt_avrcp_music_detail_t;

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_avrcp_init(bts2_app_stru *bts2_app_data);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *      BTS2S_BD_ADDR *bd_addr
 *
 * OUTPUT:
 *      U8
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
U8 bt_avrcp_get_role_by_addr(bts2_app_stru *bts2_app_data, BTS2S_BD_ADDR *bd_addr);

void bt_avrcp_set_element_support(bts2_app_stru *bts2_app_data, BTS2S_BD_ADDR *bd_addr, U32 element);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
int bt_avrcp_conn_2_dev(BTS2S_BD_ADDR *bd, BOOL is_target);
int bt_avrcp_target_connect_request(BTS2S_BD_ADDR *bd);
int bt_avrcp_controller_connect_request(BTS2S_BD_ADDR *bd);
/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_avrcp_disc_2_dev(BTS2S_BD_ADDR *bd);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_avrcp_pop(bts2_app_stru *bts2_app_data, BTS2S_BD_ADDR bd, U8 stateOpe);

void bt_avrcp_ply(bts2_app_stru *bts2_app_data, BTS2S_BD_ADDR *bd);

void bt_avrcp_timeout_handler(void *parameter);

void bt_avrcp_vol_timeout_handler(void *parameter);

void bt_avrcp_volume_register_request(bts2_app_stru *bts2_app_data, BTS2S_BD_ADDR *bd);

void bt_avrcp_volume_register_response(bts2_app_stru *bts2_app_data, BTS2S_BD_ADDR *bd, U8 response, U8 volume);

bt_err_t bt_avrcp_set_absolute_volume_request(bts2_app_stru *bts2_app_data, BTS2S_BD_ADDR *bd, U8 volume);

bt_err_t bt_avrcp_change_volume(bts2_app_stru *bts2_app_data, BTS2S_BD_ADDR *bd, U8 volume);

bt_err_t bt_avrcp_change_play_status(bts2_app_stru *bts2_app_data, BTS2S_BD_ADDR *bd, U8 play_status);

void bt_avrcp_get_play_status_request(bts2_app_stru *bts2_app_data, BTS2S_BD_ADDR *bd);

void bt_avrcp_play_status_changed_register_response(bts2_app_stru *bts2_app_data, BTS2S_BD_ADDR *bd, U8 response, U8 play_status);

void bt_avrcp_track_changed_register_response(bts2_app_stru *bts2_app_data, BTS2S_BD_ADDR *bd, U8 response, U8 track_changed);

void bt_avrcp_get_element_attributes_request(bts2_app_stru *bts2_app_data, BTS2S_BD_ADDR *bd, U8 media_attribute);

void bt_avrcp_get_next_element_request(bts2_app_stru *bts2_app_data, BTS2S_BD_ADDR *bd);

void bt_avrcp_get_all_element_request(bts2_app_stru *bts2_app_data, BTS2S_BD_ADDR *bd);

void bt_avrcp_playback_register_request(bts2_app_stru *bts2_app_data, BTS2S_BD_ADDR *bd);

void bt_avrcp_playback_pos_register_request(bts2_app_stru *bts2_app_data, BTS2S_BD_ADDR *bd);

void bt_avrcp_track_register_request(bts2_app_stru *bts2_app_data, BTS2S_BD_ADDR *bd);


/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_avrcp_rewind(bts2_app_stru *bts2_app_data, BTS2S_BD_ADDR *bd);

/*----------------------------------------------------------------------------*
*
* DESCRIPTION:
*
* INPUT:
*      bts2_app_stru *bts2_app_data:
*
* OUTPUT:
*      void.
*
* NOTE:
*      none.
*
*----------------------------------------------------------------------------*/
void bt_avrcp_record(bts2_app_stru *bts2_app_data, BTS2S_BD_ADDR *bd);

/*----------------------------------------------------------------------------*
*
* DESCRIPTION:
*
* INPUT:
*      bts2_app_stru *bts2_app_data:
*
* OUTPUT:
*      void.
*
* NOTE:
*      none.
*
*----------------------------------------------------------------------------*/
void bt_avrcp_select_sound(bts2_app_stru *bts2_app_data, BTS2S_BD_ADDR *bd);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_avrcp_stop(bts2_app_stru *bts2_app_data, BTS2S_BD_ADDR *bd);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_avrcp_pause(bts2_app_stru *bts2_app_data, BTS2S_BD_ADDR *bd);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_avrcp_forward(bts2_app_stru *bts2_app_data, BTS2S_BD_ADDR *bd);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_avrcp_backward(bts2_app_stru *bts2_app_data, BTS2S_BD_ADDR *bd);
/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_avrcp_msg_handler(bts2_app_stru *bts2_app_data);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_avrcp_subunitinfo(bts2_app_stru *bts2_app_data, BTS2S_BD_ADDR *bd);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_avrcp_unitinfo(bts2_app_stru *bts2_app_data, BTS2S_BD_ADDR *bd);

void bt_avrcp_open(void);
void bt_avrcp_close(void);
void bt_avrcp_close_boundary_condition(bts2_app_stru *bts2_app_data);

U8 bt_avrcp_get_available_connection(bts2_app_stru *bts2_app_data);

BOOL bt_avrcp_check_connection_by_addr(bts2_app_stru *bts2_app_data, BTS2S_BD_ADDR *bd, U8 *idx);

U8 bt_avrcp_get_connection_by_addr(bts2_app_stru *bts2_app_data, BTS2S_BD_ADDR *bd);

void bt_avrcp_dump_connection_info(bts2_app_stru *bts2_app_data);

#ifdef CFG_AVRCP_COVER_ART
void bt_avrcp_cover_art_client_connect(BTS2S_BD_ADDR *bd);

void bt_avrcp_player_application_setting_register_request(bts2_app_stru *bts2_app_data, BTS2S_BD_ADDR *bd);

void bt_avrcp_now_playing_content_register_request(bts2_app_stru *bts2_app_data, BTS2S_BD_ADDR *bd);

void bt_avrcp_available_players_register_request(bts2_app_stru *bts2_app_data, BTS2S_BD_ADDR *bd);

void bt_avrcp_uids_changed_register_request(bts2_app_stru *bts2_app_data, BTS2S_BD_ADDR *bd);

void bt_avrcp_addressed_player_register_request(bts2_app_stru *bts2_app_data, BTS2S_BD_ADDR *bd);

bt_err_t bt_avrcp_cover_art_get_linked_thumbnail(bts2_app_stru *bts2_app_data, BTS2S_BD_ADDR *bd);

void bt_avrcp_cover_art_get_linked_thumbnail_test(BTS2S_BD_ADDR *bd);

void bt_avrcp_cover_art_disconnect(BTS2S_BD_ADDR *bd);

#endif
#else

#define bt_avrcp_init(bts2_app_data)
#define bt_avrcp_conn_2_dev(BTS2S_BD_ADDR,is_target)
#define bt_avrcp_disc_2_dev(BTS2S_BD_ADDR)
#define bt_avrcp_pop(bts2_app_data,BTS2S_BD_ADDR,stateOpe)
#define bt_avrcp_ply(bts2_app_data,BTS2S_BD_ADDR)
#define bt_avrcp_timeout_handler()
#define bt_avrcp_volume_register_request(bts2_app_data,BTS2S_BD_ADDR)
#define bt_avrcp_volume_register_response(bts2_app_data,BTS2S_BD_ADDR,response,volume)
#define bt_avrcp_set_absolute_volume_request(bts2_app_data,BTS2S_BD_ADDR,volume)
#define bt_avrcp_change_volume(bts2_app_data,BTS2S_BD_ADDR,volume) (-BT_ERROR_INPARAM)
#define bt_avrcp_change_play_status(bts2_app_data,BTS2S_BD_ADDR,play_status) (-BT_ERROR_INPARAM)
#define bt_avrcp_get_play_status_request(bts2_app_data,BTS2S_BD_ADDR)
#define bt_avrcp_play_status_changed_register_response(bts2_app_data,BTS2S_BD_ADDR,response,play_status)
#define bt_avrcp_rewind(bts2_app_data,BTS2S_BD_ADDR)
#define bt_avrcp_record(bts2_app_data,BTS2S_BD_ADDR)
#define bt_avrcp_select_sound(bts2_app_data,BTS2S_BD_ADDR)
#define bt_avrcp_stop(bts2_app_data,BTS2S_BD_ADDR)
#define bt_avrcp_pause(bts2_app_data,BTS2S_BD_ADDR)
#define bt_avrcp_forward(bts2_app_data,BTS2S_BD_ADDR)
#define bt_avrcp_backward(bts2_app_data,BTS2S_BD_ADDR)
#define bt_avrcp_msg_handler(bts2_app_data)
#define bt_avrcp_subunitinfo(bts2_app_data,BTS2S_BD_ADDR)
#define bt_avrcp_unitinfo(bts2_app_data,BTS2S_BD_ADDR)
#define bt_avrcp_open()
#define bt_avrcp_close()
#define bt_avrcp_close_boundary_condition(bts2_app_data)

#endif

#ifdef __cplusplus
}
#endif

#endif

