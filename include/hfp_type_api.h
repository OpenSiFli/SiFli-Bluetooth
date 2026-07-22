/*
 * SPDX-FileCopyrightText: 2019-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _HFP_TYPE_API_H_
#define _HFP_TYPE_API_H_

#ifdef __cplusplus
extern "C" {
#endif

/****************************************gobal define***************************************/

#define PHONE_NUM_LEN 23


/* HFP peer features */
#define HFP_HF_FEAT_ECNR        0x0001      /* Echo cancellation/noise reduction */
#define HFP_HF_FEAT_3WAY        0x0002      /* Call waiting and three-way calling */
#define HFP_HF_FEAT_CLI         0x0004      /* Caller ID presentation capability */
#define HFP_HF_FEAT_VREC        0x0008      /* Voice recognition activation */
#define HFP_HF_FEAT_VOL         0x0010      /* Remote volume control */
#define HFP_HF_FEAT_ECS         0x0020      /* Enhanced Call Status */
#define HFP_HF_FEAT_ECC         0x0040      /* Enhanced Call Control */
#define HFP_HF_FEAT_CODEC       0x0080      /* Codec Negotiation */
#define HFP_HF_FEAT_HF_IND      0x0100      /* HF Indicators */
#define HFP_HF_FEAT_ESCO        0x0200      /* eSCO S4 (and T2) setting supported */
#define HFP_HF_FEAT_ENVR_EXT    0x0400      /*ENHANCED_VOICE_RECOGNITION_ST*/
#define HFP_HF_FEAT_VREC_TEXT   0x0800

#define HFP_HF_FEAT_VALID_MASK  0x00000FFF  /* Bits 0~11 valid */
#define HFP_HF_FEAT_RFU_MASK    0xFFFFF000  /* Bits 12~31 reserved */
/* Proprietary features: using bits after 12 */

/* Pass unknown AT command responses to application */
#define HFP_HF_FEAT_UNAT 0x1000
#define HFP_HF_FEAT_VOIP 0x2000         /* VoIP call */
#define HFP_HF_FEAT_SWB         0x4000         /* Super wide band speech */

/* HFP HF SDP SupportedFeatures bit mask */
#define HFP_HF_SDP_FEAT_ECNR        0x0001      /* Bit 0: EC and/or NR function */
#define HFP_HF_SDP_FEAT_3WAY        0x0002      /* Bit 1: Call waiting or three-way calling */
#define HFP_HF_SDP_FEAT_CLI         0x0004      /* Bit 2: CLI presentation capability */
#define HFP_HF_SDP_FEAT_VREC        0x0008      /* Bit 3: Voice recognition activation */
#define HFP_HF_SDP_FEAT_VOL         0x0010      /* Bit 4: Remote audio volume control */
#define HFP_HF_SDP_FEAT_WBS         0x0020      /* Bit 5: Wide Band Speech */
#define HFP_HF_SDP_FEAT_EVRS        0x0040      /* Bit 6: Enhanced Voice Recognition Status */
#define HFP_HF_SDP_FEAT_VREC_TEXT   0x0080      /* Bit 7: Voice Recognition Text */
#define HFP_HF_SDP_FEAT_SWBS        0x0100      /* Bit 8: Super Wide Band Speech */

/* AG feature masks */
#define HFP_AG_FEAT_3WAY    0x00000001      /* Three-way calling */
#define HFP_AG_FEAT_ECNR    0x00000002      /* Echo cancellation/noise reduction */
#define HFP_AG_FEAT_VREC    0x00000004      /* Voice recognition */
#define HFP_AG_FEAT_INBAND  0x00000008      /* In-band ring tone */
#define HFP_AG_FEAT_VTAG    0x00000010      /* Attach a phone number to a voice tag */
#define HFP_AG_FEAT_REJECT  0x00000020      /* Ability to reject incoming call */
#define HFP_AG_FEAT_ECS     0x00000040      /* Enhanced Call Status */
#define HFP_AG_FEAT_ECC     0x00000080      /* Enhanced Call Control */
#define HFP_AG_FEAT_EXTERR  0x00000100      /* Extended error codes */
#define HFP_AG_FEAT_CODEC   0x00000200      /* Codec Negotiation */

/* Valid feature bit mask for HFP 1.6 (and below) */

/* HFP 1.7+ */
#define HFP_AG_FEAT_HF_IND  0x00000400      /* HF Indicators */
#define HFP_AG_FEAT_ESCO    0x00000800      /* eSCO S4 (and T2) setting supported */
#define HFP_AG_FEAT_EVRS        0x00001000      /* Bit 12: Enhanced Voice Recognition Status */
#define HFP_AG_FEAT_VREC_TEXT   0x00002000      /* Bit 13: Voice Recognition Text */

#define HFP_AG_FEAT_VALID_MASK  0x00003FFF      /* Bits 0~13 valid */
#define HFP_AG_FEAT_RFU_MASK    0xFFFFC000      /* Bits 14~31 reserved */
/* Proprietary features: using 31 ~ 16 bits */
#define HFP_AG_FEAT_BTRH    0x00010000      /* CCAP incoming call hold */
#define HFP_AG_FEAT_UNAT    0x00020000      /* Pass unknown AT commands to app */
#define HFP_AG_FEAT_NOSCO   0x00040000      /* No SCO control performed by BTA AG */
#define HFP_AG_FEAT_NO_ESCO 0x00080000      /* Do not allow or use eSCO */
#define HFP_AG_FEAT_VOIP    0x00100000      /* VoIP call */
#define HFP_AG_FEAT_SWB     0x00200000      /*Super wide band speech for 1.9*/
#define HFP_AG_FEAT_VALID_MASK  0x00003FFF      /* Bits 0~13 valid */
#define HFP_AG_FEAT_RFU_MASK    0xFFFFC000      /* Bits 14~31 reserved */
#define HFP_AG_SDP_FEAT_3WAY        0x0001      /* Bit 0: Three-way calling */
#define HFP_AG_SDP_FEAT_ECNR        0x0002      /* Bit 1: EC and/or NR function */
#define HFP_AG_SDP_FEAT_VREC        0x0004      /* Bit 2: Voice recognition function */
#define HFP_AG_SDP_FEAT_INBAND      0x0008      /* Bit 3: In-band ring tone capability */
#define HFP_AG_SDP_FEAT_VTAG        0x0010      /* Bit 4: Attach a phone number for a voice tag */
#define HFP_AG_SDP_FEAT_WBS         0x0020      /* Bit 5: Wide Band Speech */
#define HFP_AG_SDP_FEAT_EVRS        0x0040      /* Bit 6: Enhanced Voice Recognition Status */
#define HFP_AG_SDP_FEAT_VREC_TEXT   0x0080      /* Bit 7: Voice Recognition Text */
#define HFP_AG_SDP_FEAT_SWBS        0x0100      /* Bit 8: Super Wide Band Speech */

/* HFP errcode - Set when HFP_OK_ERROR is returned in 'ok_flag' */
#define HFP_ERR_PHONE_FAILURE       0       /* Phone Failure */
#define HFP_ERR_NO_CONN_PHONE       1       /* No connection to phone */
#define HFP_ERR_OP_NOT_ALLOWED      3       /* Operation not allowed */
#define HFP_ERR_OP_NOT_SUPPORTED    4       /* Operation not supported */
#define HFP_ERR_PHSIM_PIN_REQ       5       /* PH-SIM PIN required */
#define HFP_ERR_SIM_NOT_INSERTED    10      /* SIM not inserted */
#define HFP_ERR_SIM_PIN_REQ         11      /* SIM PIN required */
#define HFP_ERR_SIM_PUK_REQ         12      /* SIM PUK required */
#define HFP_ERR_SIM_FAILURE         13      /* SIM failure */
#define HFP_ERR_SIM_BUSY            14      /* SIM busy */
#define HFP_ERR_INCORRECT_PWD       16      /* Incorrect password */
#define HFP_ERR_SIM_PIN2_REQ        17      /* SIM PIN2 required */
#define HFP_ERR_SIM_PUK2_REQ        18      /* SIM PUK2 required */
#define HFP_ERR_MEMORY_FULL         20      /* Memory full */
#define HFP_ERR_INVALID_INDEX       21      /* Invalid index */
#define HFP_ERR_MEMORY_FAILURE      23      /* Memory failure */
#define HFP_ERR_TEXT_TOO_LONG       24      /* Text string too long */
#define HFP_ERR_INV_CHAR_IN_TSTR    25      /* Invalid characters in text string */
#define HFP_ERR_DSTR_TOO_LONG       26      /* Dial string too long */
#define HFP_ERR_INV_CHAR_IN_DSTR    27      /* Invalid characters in dial string */
#define HFP_ERR_NO_NETWORK_SERV     30      /* No network service */
#define HFP_ERR_NETWORK_TIME_OUT    31      /* Network timeout */
#define HFP_ERR_NO_NET_EMG_ONLY     32      /* Network not allowed - emergency service only */
#define HFP_ERR_VOIP_CS_CALLS       33      /* AG cannot create simultaneous VoIP and CS calls */
#define HFP_ERR_NOT_FOR_VOIP        34      /* Not supported on this call type(VoIP) */
#define HFP_ERR_SIP_RESP_CODE       35      /* SIP 3 digit response code */

#define HFP_AG_SDP_FEAT_SPEC                                \
        (HFP_AG_FEAT_3WAY | HFP_AG_FEAT_ECNR | HFP_AG_FEAT_VREC | \
            HFP_AG_FEAT_INBAND | HFP_AG_FEAT_VTAG)

/* Max number of peer and local HF indicators */
#define HFP_AG_LOCAL_HF_IND_MAX_NUM   4
/****************************************enum define***************************************/
typedef enum
{
    HFP_DEVICE_DISCONNECTED = 0,
    HFP_DEVICE_CONNECTING,
    /** Session in config state, HFP configuration */
    HFP_DEVICE_CONFIG,
    HFP_DEVICE_CONNECTED,
    HFP_DEVICE_DISCONNECTING,
    HFP_DEVICE_AUDIO_DISCONNECTED,
    HFP_DEVICE_AUDIO_CONNECTING,
    HFP_DEVICE_AUDIO_CONNECTED,
    HFP_DEVICE_AUDIO_DISCONNECTING,
} hfp_device_state_t;

typedef enum
{
    HFP_CALL_IDLE = 0x00,
    HFP_CALL_INCOMING,
    HFP_CALL_OUTGOING_DAILING,
    HFP_CALL_OUTGOING_ALERTING,
    HFP_CALL_ACTIVE,
    HFP_CALL_WAITING,
    HFP_CALL_ONHOLD,
} hfp_call_state_t;

typedef enum
{
    HFP_AG_SUCC = 0,
    HFP_AG_FAILED = 0,
    HFP_AG_BUSYING,
    HFP_AG_ERR_CMD
} HFP_RESULT_CODE;

//HFP_AG_CIND_INFO                                                       \
//"(\"service\",(0,1)),(\"call\",(0,1)),(\"callsetup\",(0,3)),(\"battchg\",(0,5)),(\"signal\",(0,5)),(\"roam\",(0,1)),(\"callheld\",(0,2))"

enum
{
    HFP_AG_CIND_SERVICE_TYPE = 0x01,    //(0,1)
    HFP_AG_CIND_CALL_TYPE,              //(0,1)
    HFP_AG_CIND_CALLSETUP_TYPE,         //(0,3)
    HFP_AG_CIND_BATT_TYPE,              //(0,5)
    HFP_AG_CIND_SIGNAL_TYPE,            //(0,5)
    HFP_AG_CIND_ROAM_TYPE,              //(0,1)
    HFP_AG_CIND_CALLHELD_TYPE,          //(0,2)
};

/* for clcc_res var start */
typedef enum
{
    PHONE_CALL_ACTIVE = 0x00,
    PHONE_CALL_HELD,
    PHONE_CALL_DAILING,
    PHONE_CALL_ALERTING,
    PHONE_CALL_INCOMING,
    PHONE_CALL_WAITING,
} phone_call_status_t;

typedef enum
{
    PHONE_CALL_DIR_OUTGOING = 0x00,
    PHONE_CALL_DIR_INCOMING,
} phone_call_dir_t;

typedef enum
{
    PHONE_CALL_MODE_VOICE = 0x00,
    PHONE_CALL_MODE_DATA,
    PHONE_CALL_MODE_FAX,
    PHONE_CALL_MODE_RESERVE,
} phone_call_mode_t;

typedef enum
{
    PHONE_CALL_MPTY_SINGLE = 0x00, // one call
    PHONE_CALL_MPTY_MULTI,        // two or more than calls
} phone_call_mpty_t;

typedef enum
{
    PHONE_NUMBER_TYPE_UNKNOWN = 0x81,
    PHONE_NUMBER_TYPE_INTERNATIONAL = 0x91,
} phone_number_type_t;
/* for clcc_res var end */

typedef enum
{
    /*! No HF indicator. */
    hfp_indicator_mask_none           = 0x00,
    /*! Enhanced safety indicator. */
    hfp_enhanced_safety_mask          = 1 << 0,
    /*! Battery level indicator. */
    hfp_battery_level_mask            = 1 << 1,
} hfp_hf_indicators_mask;

typedef enum
{
    HFP_SCO_RETRY_NONE,
    HFP_SCO_RETRY_ONCE,
    HFP_SCO_RETRY_TWICE,
    HFP_SCO_RETRY_DEFAULT,
    SCO_RETRY_MAX   /* 枚举项总数，用于数组大小/边界检查 */
} hfp_sco_retry_flag_t;

/****************************************struct define***************************************/
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
    U32 supp_featr;
    U16 hfp_indicators_mask;
} BTS2S_HFP_ENB_REQ_EXT;

typedef struct
{
    U16 type;
    U8 mux_id;
    BTS2S_BD_ADDR bd;
    BOOL audio_on;
    U16 sco_hdl;
    /*
    * 0x00 SCO connection
    * 0x01 Reserved for future use
    * 0x02 eSCO connection
    */
    U8 profile_type;
    U8 tx_intvl;
    /* Retransmission Window */
    U8 we_sco;
    U16 rx_pkt_len;
    U16 tx_pkt_len;
    /*
    * 0x00 |��-law log
    * 0x01 A-law log
    * 0x02 CVSD
    * 0x03 Transparent Data
    */
    U8 air_mode;
    U8 codec_id;
} BTS2S_HF_AUDIO_INFO;

typedef struct
{
    char phone_number[PHONE_NUM_LEN];
    U8 type;
} hfp_phone_number_t;

typedef struct
{
    U8 call_idx;
    U8 call_dir;
    U8 call_status;
    U8 call_mode;
    U8 call_mtpty;
    hfp_phone_number_t phone_info;
} hfp_phone_call_info_t;

typedef struct
{
    U8 service_status;
    U8 call;
    U8 callsetup;
    U8 batt_level;
    U8 signal;
    U8 roam_status;
    U8 callheld;
} hfp_cind_status_t;

typedef struct
{
    U16 type;
    U8  mux_id;
    U8 num_active;
    U8 num_held;
    U8 callsetup_state;
    U8 phone_type;
    U8 phone_len;
    U8 phone_number[1];
} HFP_CALL_INFO_T;

typedef struct
{
    U8 ind_type;
    U8 ind_val;
} HFP_IND_INFO_T;

typedef struct
{
    U8 num_call;
    hfp_phone_call_info_t *calls;
} hfp_remote_calls_info_t;

typedef struct
{
    U8 batt_status;  // 1: mean current batt_val 2: device is in charging
    U8 batt_val;
} hfp_batt_vaule_t;

#define HFP_HF_IND_ENHANCED_SAFETY 1
#define HFP_HF_IND_BATTERY_LEVEL   2


struct hf_ind_enhanced_safety
{
    U8 local_support: 1;
    U8 remote_support: 1;
    U8 enabled: 1;
    U8 value: 5; /* 0 or 1 */
};

struct hf_ind_battery_level
{
    U8 local_support: 1;
    U8 remote_support: 1;
    U8 enabled: 6;
    U8 value; /* 0 ~ 100 */
};

typedef struct
{
    struct hf_ind_enhanced_safety enhanced_safety;
    struct hf_ind_battery_level battery_level;
} hf_indicator_t;
#ifdef __cplusplus
}
#endif

#endif
