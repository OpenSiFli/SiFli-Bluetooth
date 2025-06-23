/*
 * SPDX-FileCopyrightText: 2019-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _HFP_AG_API_H_
#define _HFP_AG_API_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "hfp_type_api.h"


/****************************************enum define*************************************************/
typedef enum
{
    HFP_AG_SPK_EVT      =  7,       /* Speaker volume changed */
    HFP_AG_MIC_EVT      =  8,       /* Microphone volume changed */
    HFP_AG_AT_CKPD_EVT  =  9,       /* CKPD from the HS */
    HFP_AG_AT_A_EVT     = 10,       /* Answer a call */
    HFP_AG_AT_D_EVT     = 11,       /* Place a call using number or memory dial */
    HFP_AG_AT_CHLD_EVT  = 12,       /* Call hold */
    HFP_AG_AT_CHUP_EVT  = 13,       /* Hang up a call */
    HFP_AG_AT_CIND_EVT  = 14,       /* Read indicator settings */
    HFP_AG_AT_VTS_EVT   = 15,       /* Transmit DTMF tone */
    HFP_AG_AT_BINP_EVT  = 16,       /* Retrieve number from voice tag */
    HFP_AG_AT_BLDN_EVT  = 17,       /* Place call to last dialed number */
    HFP_AG_AT_BVRA_EVT  = 18,       /* Enable/disable voice recognition */
    HFP_AG_AT_NREC_EVT  = 19,       /* Disable echo canceling */
    HFP_AG_AT_CNUM_EVT  = 20,       /* Retrieve subscriber number */
    HFP_AG_AT_BTRH_EVT  = 21,       /* CCAP-style incoming call hold */
    HFP_AG_AT_CLCC_EVT  = 22,       /* Query list of current calls */
    HFP_AG_AT_COPS_EVT  = 23,       /* Query list of current calls */
    HFP_AG_AT_UNAT_EVT  = 24,       /* Unknown AT command */
    HFP_AG_AT_CBC_EVT   = 25,       /* Battery Level report from HF */
    HFP_AG_AT_BAC_EVT   = 26,       /* avablable codec */
    HFP_AG_AT_BCS_EVT   = 27,       /* Codec select */
    HFP_AG_AT_BIND_EVT  = 28,       /* HF indicator */
    HFP_AG_AT_BIEV_EVT  = 29,       /* HF indicator updates from peer */
    HFP_AG_DISABLE_EVT  = 30,       /* AG disabled */
    HFP_AG_WBS_EVT      = 31,       /* SCO codec info */
    HFP_AG_BATT_EVT     = 32,       /* BATT from hf*/
} hfp_ag_at_command_evt;

enum
{
    BTS2MU_AG_REG_CFM = BTS2MU_START,
    BTS2MU_AG_UNREG_CFM,
    BTS2MU_AG_CONN_CFM,
    BTS2MU_AG_CONN_IND,
    BTS2MU_AG_DISC_CFM,
    BTS2MU_AG_DISC_IND,
    BTS2MU_AG_AUDIO_CFM,
    BTS2MU_AG_AUDIO_CONN_CFM,
    BTS2MU_AG_AUDIO_CONN_IND,
    BTS2MU_AG_AUDIO_DISC_CFM,
    BTS2MU_AG_AUDIO_DISC_IND,
    BTS2MU_AG_SCO_RENEGOTIATE_IND,
    BTS2MU_AG_SCO_RENEGOTIATE_CFM,
    BTS2MU_AG_AT_CMD_EVENT,
    BTS2MU_AG_AT_CMD_CFM,
};

/* AT res events */
typedef enum
{
    HFP_AG_SPK_RES = 0x00,          /* Update speaker volume */
    HFP_AG_MIC_RES,                 /* Update microphone volume */
    HFP_AG_INBAND_RING_RES,         /* Update inband ring state */
    HFP_AG_CIND_RES,                /* Send indicator response for AT+CIND */
    HFP_AG_BINP_RES,                /* Send phone number for voice tag for AT+BINP */
    HFP_AG_IND_RES,                 /* Update an indicator value */
    HFP_AG_BVRA_RES,                /* Update voice recognition state */
    HFP_AG_CNUM_RES,                /* Send subscriber number response for AT+CNUM */
    HFP_AG_BTRH_RES,                /* Send CCAP incoming call hold */
    HFP_AG_CLCC_RES,                /* Query list of calls */
    HFP_AG_COPS_RES,                /* Read network operator */

    HFP_AG_IN_CALL_RES,             /* Indicate incoming phone call */
    HFP_AG_IN_CALL_CONN_RES,        /* Incoming phone call connected */
    HFP_AG_CALL_WAIT_RES,           /* Call waiting notification */
    HFP_AG_OUT_CALL_ORIG_RES,       /* Outgoing phone call origination */
    HFP_AG_OUT_CALL_ALERT_RES,      /* Outgoing phone call alerting remote party */
    HFP_AG_OUT_CALL_CONN_RES,       /* Outgoing phone call connected */
    HFP_AG_CALL_CANCEL_RES,         /* Incoming/outgoing 3-way canceled before connected */
    HFP_AG_END_CALL_RES,            /* End call */
    HFP_AG_IN_CALL_HELD_RES,        /* Incoming call held */
    HFP_AG_MULTI_CALL_RES,          /* SLC at three way call */

    HFP_AG_UNAT_RES,                /* Response to unknown AT command event */
    HFP_AG_BIND_RES,                /* Activate/Deactivate HF indicator */
} hfp_ag_at_res_event_t;

/* Local AT command result codes defined in hfp_ag_api.h */
typedef enum
{
    HFP_AG_LOCAL_RES_FIRST = 0x0100,
    HFP_AG_LOCAL_RES_OK,
    HFP_AG_LOCAL_RES_ERROR,
    HFP_AG_LOCAL_RES_RING,
    HFP_AG_LOCAL_RES_CLIP,
    HFP_AG_LOCAL_RES_BRSF,
    HFP_AG_LOCAL_RES_CMEE,
    HFP_AG_LOCAL_RES_BCS
} hfp_ag_lcoal_at_res_event_t;
/****************************************struct define*************************************************/
typedef struct
{
    U16 type;
} BTS2S_HFP_CMD_TYPE;

typedef struct
{
    U16 type;
    U32 supp_featr;
} BTS2S_HFP_ENB_REQ;

typedef struct
{
    U16 type;
    BTS2S_BD_ADDR bd;
    U16 rfcomm_chnl; //for reconnect not need sdp search
} BTS2S_AG_CONN_REQ;

typedef struct
{
    U16 type;
    BTS2S_BD_ADDR bd;
} BTS2S_AG_CONN_INFO;

typedef struct
{
    U16 type;
    U8  mux_id;
    S16 command_id;
    S16 val;
    char str[1];
} BTS2S_AG_AT_CMD_INFO;

typedef struct
{
    U16 type;
    U8 res;
} BTS2S_AG_CFM;

typedef struct
{
    U16 type;
    BTS2S_BD_ADDR bd;
    U8  mux_id;
    U8 device_state;
    U8 res;
} BTS2S_AG_CONN_RES;

typedef struct
{
    U16 type;
    BTS2S_BD_ADDR bd;
    U8  mux_id;
    U8 audio_on;
    U8 res;
} BTS2S_AG_AUDIO_CONN_CFM;

typedef struct
{
    U16 type;
    BTS2S_BD_ADDR bd;
    U8  mux_id;
    U16 rfcomm_chnl; //for reconnect not need sdp search
    U8 res;
} BTS2S_AG_CONN_IND_RES;

typedef struct
{
    U16 type;
    BTS2S_BD_ADDR bd;
    U8  mux_id;
    U8  code_id;
} BTS2S_AG_AUDIO_CONN_IND;

typedef struct
{
    U16 type;
    U16 at_cmd_id;
    U8  mux_id;
    U8  res;
} BTS2S_AG_AT_CMD_CFM;
/****************************************func define*************************************************/
/*******************************************************************************
 *
 * Function         hfp_ag_register
 *
 * Description     Initialize the bluetooth HF AG module and init local features
 *
 * Returns         void
 *
 ******************************************************************************/
void hfp_ag_register(U32 local_features);

/*******************************************************************************
 *
 * Function         hfp_ag_deregister
 *
 * Description     De-initialize for HF AG module
 *
 * Returns         void
 *
 ******************************************************************************/
void hfp_ag_deregister(void);

/*******************************************************************************
 *
 * Function         hfp_ag_connect
 *
 * Description      Opens a connection to a headset or hands-free device.
 *                  When connection is open callback function is called
 *                  with a HFP_AG_OPEN_EVT. Only the data connection is
 *                  opened. The audio connection is not opened.
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void hfp_ag_connect(BTS2S_BD_ADDR *bd, U16 rfcomm_chnl);

/*******************************************************************************
 *
 * Function         hfp_ag_connect_ind_res
 *
 * Description      Opens a connection to a headset or hands-free device.
 *                  When connection is open callback function is called
 *                  with a HFP_AG_OPEN_EVT. Only the data connection is
 *                  opened. The audio connection is not opened.
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void hfp_ag_connect_ind_res(BTS2S_BD_ADDR *bd, U16 rfcomm_chnl, U8 res);

/*******************************************************************************
 *
 * Function         hfp_ag_disconnect
 *
 * Description      To establish a Service Level Connection to remote
 *                  bluetooth HFP client device.
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void hfp_ag_disconnect(BTS2S_BD_ADDR *bd);

/*******************************************************************************
*
* Function         hfp_ag_connect_audio
*
* Description      Opens an audio connection to the currently connected
*                  headset or hnadsfree.
*
*
* Returns          void
*
******************************************************************************/
void hfp_ag_connect_audio(BTS2S_BD_ADDR *bd);

/*******************************************************************************
*
* Function         hfp_ag_disconnect_audio
*
* Description      Close the currently active audio connection to a headset
*                  or hnadsfree. The data connection remains open
*
*
* Returns          void
*
******************************************************************************/
void hfp_ag_disconnect_audio(BTS2S_BD_ADDR *bd);

/*******************************************************************************
 *
 * Function         start_voice_recognition
 *
 * Description      start voice recognition
 *
 * Returns          void
 *
 ******************************************************************************/
void hfp_ag_start_voice_recognition(BTS2S_BD_ADDR *bd);

/*******************************************************************************
 *
 * Function         stop_voice_recognition
 *
 * Description      stop_voice_recognition
 *
 * Returns          void
 *
 ******************************************************************************/
void hfp_ag_stop_voice_recognition(BTS2S_BD_ADDR *bd);

/*******************************************************************************
 *
 * Function         phone_state_change
 *
 * Description      notify of a call state change
 *                  number & type: valid only for incoming & waiting call
 *
 * Returns          void
 *
 ******************************************************************************/
void hfp_ag_phone_call_status_changed_api(U8 mux_id, HFP_CALL_INFO_T *p_call_info);


/****************************************AT cmd func define*************************************************/
/*******************************************************************************
 *
 * Function         volume_control
 *
 * Description      speaker volume control
 *
 * Returns          void
 *
 ******************************************************************************/
void hfp_ag_spk_volume_control(U8 mux_id, S8 volume);

/*******************************************************************************
 *
 * Function         volume_control
 *
 * Description      microphone volume control
 *
 * Returns          void
 *
 ******************************************************************************/
void hfp_ag_mic_volume_control(U8 mux_id, S8 volume);

/*******************************************************************************
 *
 * Function         cind_response
 *
 * Description      Response to device individual indicators to HFP Client.
 *
 * Returns          void
 *
 ******************************************************************************/
void hfp_ag_cind_response(U8 mux_id, char *payload, U8 payload_len);

/*******************************************************************************
 *
 * Function         send_indicator_update
 *
 * Description      Send indicator report ��+CIEV: <ind> <value>�� to HFP Client. (CIEV)
 *
 * Returns          void
 *
 ******************************************************************************/
void hfp_ag_ind_status_update(U8 mux_id, char *payload, U8 payload_len);

/*******************************************************************************
 *
 * Function         Enable/disable voice recognition update
 *
 * Description      Enable/disable voice recognition update
 *
 * Returns          void
 *
 ******************************************************************************/
void hfp_ag_brva_response(U8 mux_id, U8 val);

/*******************************************************************************
 *
 * Function         hfp_ag_set_inband
 *
 * Description      Determine whether in-band ring can be used.
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void hfp_ag_set_inband(U8 mux_id, U8 val);

/*******************************************************************************
*
* Function        Response for AT+CNUM command from HF Client.
*                 As a precondition to use this API, Service
*                 Level Connection shall exist with HFP client.
*
* Description      registration number
*                  phone type national or international
*
*
*
* Returns          void
*
******************************************************************************/
void hfp_ag_cnum_response(U8 mux_id, char *payload, U8 payload_len);

/*******************************************************************************
 *
 * Function         hfp_ag_set_btrh
 *
 * Description      Reponse for AT+BRTH command from HF Client.
 *
 * Returns          void
 *
 ******************************************************************************/
void hfp_ag_set_btrh(U8 mux_id, U8 val);

/*******************************************************************************
*
* Function         clcc_response
*
* Description      Response to AT+CLCC command from HFP Client.
*                  Can be iteratively called for each call index. Call index
*                  of 0 will be treated as NULL termination (Completes
*                  response)
*
* Returns          void
*
******************************************************************************/
void hfp_ag_clcc_response(U8 mux_id, char *payload, U8 payload_len);

/*******************************************************************************
 *
 * Function         cops_response
 *
 * Description      Reponse for AT+COPS command from HF Client.
 *
 * Returns          void
 *
 ******************************************************************************/
void hfp_ag_cops_response(U8 mux_id, char *payload, U8 payload_len);

/*******************************************************************************
 *
 * Function         hfp_ag_set_codec_id
 *
 * Description      Specify the codec type to be used for the subsequent
 *                  audio connection.
 *
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void hfp_ag_set_codec_id(U8 mux_id, U8 code_id);

/*******************************************************************************
 *
 * Function         hfp_ag_clip_response
 *
 * Description      when call is coming send +clip:phone_info to hf
 *
 * Returns          void
 *
 ******************************************************************************/
void hfp_ag_clip_response(U8 mux_id, char *payload, U8 payload_len);

/*******************************************************************************
 *
 * Function         hfp_ag_app_send_ring
 *
 * Description      sned ring
 *
 * Returns          void
 *
 ******************************************************************************/
void hfp_ag_app_send_ring(U8 mux_id);

/*******************************************************************************
 *
 * Function         hfp_ag_send_unknow_at_cmd
 *
 * Description      send unknow at cmd
 *
 * Returns          void
 *
 ******************************************************************************/
void hfp_ag_send_unknow_at_cmd(U8 mux_id, char *payload, U8 payload_len);

/*******************************************************************************
 *
 * Function         send_at_result
 *
 * Description      Send AT result code (OK/ERROR)
 *
 * Returns          void
 *
 ******************************************************************************/
void hfp_ag_at_cmd_result(U8 mux_id, U8 res);

void hfp_set_sco_retry_flag(U8 enable);
U8 hfp_get_sco_retry_flag();
uint8_t bt_hfp_ag_profile_get_service_max_num();
void bt_hfp_ag_profile_set_service_max_num(uint8_t max_num);

#ifdef __cplusplus
}
#endif

#endif
