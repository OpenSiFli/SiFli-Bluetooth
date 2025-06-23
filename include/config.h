/*
 * SPDX-FileCopyrightText: 2019-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*Host lib supported profiles*/
/*CFG_NO_L2C mean profile service can't work without gap_l2c*/
#ifndef CFG_NO_L2C
    #define CFG_AVRCP_LIB
    #define CFG_AV_LIB
    #define CFG_AV_SRC_LIB
    #define CFG_AV_SNK_LIB
    #define CFG_HID_LIB
    #define CFG_BR_GATT_SRV_LIB
#endif

#define CFG_SPP_CLT_LIB
#define CFG_SPP_SRV_LIB
#define CFG_HFP_HF_LIB
#define CFG_HFP_AG_LIB
#define CFG_PAN_LIB
#define CFG_PBAP_CLT_LIB
#define CFG_BT_DIS_LIB
#define CFG_BT_L2CAP_PROFILE_LIB

/*If not customized, use all profiles*/
#ifndef BT_PROFILE_CUSTOMIZE
    #ifndef CFG_NO_L2C
        #ifdef CFG_AVRCP_LIB
            #define CFG_AVRCP
        #endif
        #ifdef CFG_AV_LIB
            #define CFG_AV
        #endif
        #ifdef CFG_AV_SRC_LIB
            #define CFG_AV_SRC
        #endif
        #ifdef CFG_AV_SNK_LIB
            #define CFG_AV_SNK
        #endif
        #ifdef CFG_BR_GATT_SRV_LIB
            #define CFG_BR_GATT_SRV
        #endif
        #ifdef CFG_HID_LIB
            #define CFG_HID
        #endif
        #ifdef CFG_BT_L2CAP_PROFILE_LIB
            #define CFG_BT_L2CAP_PROFILE
        #endif
    #endif
    #ifdef CFG_SPP_CLT_LIB
        #define CFG_SPP_CLT
    #endif
    #ifdef CFG_SPP_SRV_LIB
        #define CFG_SPP_SRV
    #endif
    #ifdef CFG_HFP_HF_LIB
        #define CFG_HFP_HF
    #endif
    #ifdef CFG_HFP_AG_LIB
        #define CFG_HFP_AG
    #endif
    #ifdef CFG_PAN_LIB
        #define CFG_PAN
    #endif
    #ifdef CFG_PBAP_CLT_LIB
        #define CFG_PBAP_CLT
    #endif
    #ifdef CFG_BT_DIS_LIB
        #define CFG_BT_DIS
    #endif
#endif




/*
 * HID
 */


#define CFG_OPEN_SCAN
#ifdef CFG_AV_SNK
    #define CFG_OPEN_AVSNK
#endif
//#define CFG_OPEN_SDKRECONN
#ifdef CFG_AVRCP
    #define CFG_OPEN_AVRCP
#endif

#ifdef CFG_HID
    #define CFG_OPEN_HID
#endif
//#define CFG_OPNE_3WAY


#define CFG_MAX_ACL_CONN_NUM (7)
//support the maximum number of spp client and server.
#define SPP_CLT_MAX_CONN_NUM 2
#define SPP_SRV_MAX_CONN_NUM 3




