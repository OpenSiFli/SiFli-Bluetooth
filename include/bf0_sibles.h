/*
 * SPDX-FileCopyrightText: 2019-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef BF0_SIBLES_H_
#define BF0_SIBLES_H_

/**
 ****************************************************************************************
 * @addtogroup sibles BLE interface
 * @ingroup middleware
 * @brief Provided standard GAP BLE interface and optimized GATT service and client interface.
 * @{
 */

/**
 * @defgroup sibles_service Sifli BLE service
 * @brief Sifli ble service interface
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdint.h>
#include <stdlib.h>
#include "att.h"
#include "bf0_ble_gatt.h"
#include "bf0_ble_common.h"

#ifdef BT_FINSH
    #include "bts2_bt.h"
#endif

/*
 * DEFINES
 ****************************************************************************************
 */

//#define TASK_FIRST_MSG(x)   0
#define __ARRAY_EMPTY
#define SIBLES_ERROR_REMOTE_HANDLE 0xFFFF

// add for multiple device advertise
#define INVALID_CONN_IDX 0xFF

#define SIG_SIFLI_COMPANY_ID 0x0A4C

#define KEY_ID_LEN 4

#define SVC_SEARCH_BUILD_ID(svc_db_state, index) (((index) << 8)|(svc_db_state))
#define SVC_SEARCH_IDX_GET(svc_id) (((svc_id) >> 8) & 0xFF)
#define SVC_SEARCH_STATE_GET(svc_id) ((svc_id) & 0xFF)
#define SVC_SEARCH_MAX 3

/** @defgroup sible_types Type definitions
  * @{
  */

/**
 * @brief Sibles GATT events that notify user.
 */
enum bf0_sifli_event_t
{
    SIBLES_WRITE_VALUE_RSP = BLE_SIBLE_TYPE,/*!< Write value response. */
    SIBLES_SEARCH_SVC_RSP,                  /*!< Service search response. */
    SIBLES_REGISTER_REMOTE_SVC_RSP,         /*!< Service register response. */
    SIBLES_REMOTE_EVENT_IND,                /*!< Remote event Indication. */
    SIBLES_REMOTE_CONNECTED_IND,            /*!< Remote connected Indication. */
    SIBLES_REMOTE_DISCONNECTED_IND,         /*!< Remote disconnected Indication. */
    SIBLES_READ_REMOTE_VALUE_RSP,           /*!< Remote value request response. */
    SIBLES_MTU_EXCHANGE_IND,                /*!< MTU negotiation indication. */
    SIBLES_EVENT_REQ_IND,
    SIBLES_SVC_CHANGED_CFG,
    SIBLES_WRITE_REMOTE_VALUE_RSP,
    SIBLES_CHANGE_BD_ADDR_RSP,
    SIBLES_REMOTE_SVC_CHANGE_IND,
    SIBLES_ATT_UPDATE_PERM_IND,
    SIBLES_DIS_SET_VAL_RSP,
    SIBLES_ATT_SET_VISIBILITY_IND,
    SIBLES_ATT_ERROR_EVENT_IND,
};

typedef enum
{
    /// Error response
    SIBLES_ATT_ERR_RSP            = 0x01,
    /// Exchange MTU Request
    SIBLES_ATT_MTU_REQ            = 0x02,
    /// Exchange MTU Response
    SIBLES_ATT_MTU_RSP            = 0x03,
    /// Find Information Request
    SIBLES_ATT_FIND_INFO_REQ      = 0x04,
    /// Find Information Response
    SIBLES_ATT_FIND_INFO_RSP      = 0x05,
    /// Find By Type Value Request
    SIBLES_ATT_FIND_BY_TYPE_REQ   = 0x06,
    /// Find By Type Value Response
    SIBLES_ATT_FIND_BY_TYPE_RSP   = 0x07,
    /// Read By Type Request
    SIBLES_ATT_RD_BY_TYPE_REQ     = 0x08,
    /// Read By Type Response
    SIBLES_ATT_RD_BY_TYPE_RSP     = 0x09,
    /// Read Request
    SIBLES_ATT_RD_REQ             = 0x0A,
    /// Read Response
    SIBLES_ATT_RD_RSP             = 0x0B,
    /// Read Blob Request
    SIBLES_ATT_RD_BLOB_REQ        = 0x0C,
    /// Read Blob Response
    SIBLES_ATT_RD_BLOB_RSP        = 0x0D,
    /// Read Multiple Request
    SIBLES_ATT_RD_MULT_REQ        = 0x0E,
    /// Read Multiple Response
    SIBLES_ATT_RD_MULT_RSP        = 0x0F,
    /// Read by Group Type Request
    SIBLES_ATT_RD_BY_GRP_TYPE_REQ = 0x10,
    /// Read By Group Type Response
    SIBLES_ATT_RD_BY_GRP_TYPE_RSP = 0x11,
    /// Write Request
    SIBLES_ATT_WR_REQ             = 0x12,
    /// Write Response
    SIBLES_ATT_WR_RSP             = 0x13,
    /// Write Command
    SIBLES_ATT_WR_CMD_INFO        = 0x14,
    SIBLES_ATT_WR_CMD             = 0x52,
    /// Signed Write Command
    SIBLES_ATT_SIGN_WR_CMD_INFO   = 0x15,
    SIBLES_ATT_SIGN_WR_CMD        = 0xD2,
    /// Prepare Write Request
    SIBLES_ATT_PREP_WR_REQ        = 0x16,
    /// Prepare Write Response
    SIBLES_ATT_PREP_WR_RSP        = 0x17,
    /// Execute Write Request
    SIBLES_ATT_EXE_WR_REQ         = 0x18,
    /// Execute Write Response
    SIBLES_ATT_EXE_WR_RSP         = 0x19,
    /// Handle Value Notification
    SIBLES_ATT_HDL_VAL_NTF        = 0x1B,
    /// Handle Value Indication
    SIBLES_ATT_HDL_VAL_IND        = 0x1D,
    /// Handle Value Confirmation
    SIBLES_ATT_HDL_VAL_CFM        = 0x1E,

    /// max number of security codes
    SIBLES_ATT_MAX
} sibles_attribute_code_t;

typedef enum
{
    SIBLES_ATT_ERR_NO_ERROR                                                               = 0x00,
    /// 0x01: Handle is invalid
    SIBLES_ATT_ERR_INVALID_HANDLE                                                         = 0x01,
    /// 0x02: Read permission disabled
    SIBLES_ATT_ERR_READ_NOT_PERMITTED                                                     = 0x02,
    /// 0x03: Write permission disabled
    SIBLES_ATT_ERR_WRITE_NOT_PERMITTED                                                    = 0x03,
    /// 0x04: Incorrect PDU
    SIBLES_ATT_ERR_INVALID_PDU                                                            = 0x04,
    /// 0x05: Authentication privilege not enough
    SIBLES_ATT_ERR_INSUFF_AUTHEN                                                          = 0x05,
    /// 0x06: Request not supported or not understood
    SIBLES_ATT_ERR_REQUEST_NOT_SUPPORTED                                                  = 0x06,
    /// 0x07: Incorrect offset value
    SIBLES_ATT_ERR_INVALID_OFFSET                                                         = 0x07,
    /// 0x08: Authorization privilege not enough
    SIBLES_ATT_ERR_INSUFF_AUTHOR                                                          = 0x08,
    /// 0x09: Capacity queue for reliable write reached
    SIBLES_ATT_ERR_PREPARE_QUEUE_FULL                                                     = 0x09,
    /// 0x0A: Attribute requested not existing
    SIBLES_ATT_ERR_ATTRIBUTE_NOT_FOUND                                                    = 0x0A,
    /// 0x0B: Attribute requested not long
    SIBLES_ATT_ERR_ATTRIBUTE_NOT_LONG                                                     = 0x0B,
    /// 0x0C: Encryption size not sufficient
    SIBLES_ATT_ERR_INSUFF_ENC_KEY_SIZE                                                    = 0x0C,
    /// 0x0D: Invalid length of the attribute value
    SIBLES_ATT_ERR_INVALID_ATTRIBUTE_VAL_LEN                                              = 0x0D,
    /// 0x0E: Operation not fit to condition
    SIBLES_ATT_ERR_UNLIKELY_ERR                                                           = 0x0E,
    /// 0x0F: Attribute requires encryption before operation
    SIBLES_ATT_ERR_INSUFF_ENC                                                             = 0x0F,
    /// 0x10: Attribute grouping not supported
    SIBLES_ATT_ERR_UNSUPP_GRP_TYPE                                                        = 0x10,
    /// 0x11: Resources not sufficient to complete the request
    SIBLES_ATT_ERR_INSUFF_RESOURCE                                                        = 0x11,
    /// 0x80: Application error (also used in PRF Errors)
    SIBLES_ATT_ERR_APP_ERROR                                                              = 0x80,
} sibles_att_error_reason_t;

/**
 * @brief Sibles GATT write type.
 */
typedef enum
{
    SIBLES_WRITE,                           /*!<BLE write */
    SIBLES_WRITE_WITHOUT_RSP                /*!<BLE write without response */
} sibles_write_type_t;

/**
 * @brief Sibles GATT read type.
 */
typedef enum
{
    SIBLES_READ
} sibles_read_type_t;


typedef enum
{
    SIBLES_TRC_NONE,
    SIBLES_TRC_HCI_ON,
    SIBLES_TRC_ALL_ON,
    SIBLES_TRC_CUSTOMIZE,
    SIBLES_TRC_LAST
} sibles_trc_cfg_t;

typedef enum
{
    SIBLES_CH_BD_TYPE_BT,
    SIBLES_CH_BD_TYPE_BLE,
    SIBLES_CH_BD_TYPE_TOTAL,
} sibles_change_bd_addr_type_t;

typedef enum
{
    SIBLES_CH_BD_METHOD_ORIGIN,
    SIBLES_CH_BD_METHOD_PRESERVE,
    SIBLES_CH_BD_METHOD_CUSTOMIZE,
    SIBLES_CH_BD_METHOD_TOTAL,
} sibles_change_bd_addr_method_t;


/// Internal 16bits UUID service description
struct attm_desc
{
    uint16_t uuid;                          /*!< 16 bits UUID LSB First */
    uint16_t perm;                          /*!< Attribute Permissions (@see enum attm_perm_mask) */
    uint16_t ext_perm;                      /*!< Attribute Extended Permissions (@see enum attm_value_perm_mask) */
    /// note: for characteristic declaration contains handle offset
    /// note: for included service, contains target service handle
    uint16_t max_size;                      /*!< Attribute Max Size */
};

/// Internal 128bits UUID service description
struct attm_desc_128
{
    uint8_t uuid[ATT_UUID_128_LEN];         /*!< 128 bits UUID LSB First */
    uint16_t perm;                          /*!< Attribute Permissions (@see enum attm_perm_mask) */
    uint16_t ext_perm;                      /*!< Attribute Extended Permissions (@see enum attm_value_perm_mask) */
    /// note: for characteristic declaration contains handle offset
    /// note: for included service, contains target service handle
    uint16_t max_size;                       /*!< Attribute Max Size */
};

typedef enum
{
    SIBLES_WRITE_NO_ERR,
    SIBLES_WRITE_HANDLE_ERR,
    SIBLES_WIRTE_TX_FLOWCTRL_ERR,
    SIBLES_OUT_OF_MEMORY,
    OTHER_ERR
} sibles_write_err_t;

typedef enum
{
    SVC_SEARCH_EMPTY,
    SVC_SEARCH_BUSY,
} svc_search_state_t;

/**
 * @brief .Service handle.
 */
typedef void *sibles_hdl;                   /*!< SIBLES service handle type*/

/**
 * @brief .The structure of #SIBLES_WRITE_VALUE_RSP.
 */
typedef struct
{
    uint8_t result;                         /*!< Result of write value response. @see enum hl_err */
    uint8_t conn_idx;
} sibles_write_value_rsp_t;

typedef struct
{
    uint8_t result;                         /*!< Result of write value response. @see enum hl_err */
    uint8_t conn_idx;
} sibles_write_remote_value_rsp_t;


/**
 * @brief .Register GATT service parameter with 16 bit uuid.
 */
typedef struct
{
    uint16_t           uuid;                /*!< UUID of ATT service*/
    struct attm_desc  *att_db;              /*!< ATT service attribute record*/
    int                num_entry;           /*!< Number of service record in att_db*/
    uint8_t            sec_lvl;             /*!< Sercurity level for service*/
} sibles_register_svc_t;

/**
 * @brief .Register GATT service parameter with 128 bit uuid.
 */
typedef struct
{
    uint8_t               *uuid;        /*!< Pointer to 128 bit UUID of ATT service*/
    struct attm_desc_128  *att_db;      /*!< ATT service attribute record*/
    int                    num_entry;   /*!< Number of service record in att_db*/
    uint8_t                sec_lvl;     /*!< Sercurity level for service*/
} sibles_register_svc_128_t;


/**
 * @brief The structure of Sibles GATT value .
 */
typedef struct
{
    sibles_hdl   hdl;               /*!< Service handle */
    uint8_t      idx;               /*!< Value index in Service */
    uint16_t     len;               /*!< Length of value*/
    uint8_t     *value;             /*!< Value content */
} sibles_value_t;

/**
 * @brief The structure of GATT service of peer device.
 */
typedef struct
{
    uint16_t hdl_start;             /*!< BLE service start handle */
    uint16_t hdl_end;               /*!< BLE service end handle */
    uint8_t uuid_len;               /*!< BLE service UUID length */
    uint8_t uuid[ATT_UUID_128_LEN]; /*!< BLE service UUID */
    uint8_t char_count;             /*!< BLE characterisic count*/
    uint8_t *att_db;                /*!< BLE service database, @see sibles_svc_search_char_t */
} sibles_svc_remote_svc_t;

/**
 * @brief The strcture of #SIBLES_SEARCH_SVC_RSP.
 */
typedef struct
{
    uint8_t conn_idx;               /*!< connection index*/
    uint8_t result;                 /*! < search result, @see enum hl_err */
    uint8_t search_svc_len;
    uint8_t search_uuid[ATT_UUID_128_LEN];
    sibles_svc_remote_svc_t *svc;   /*!< Service record, only valid when result is HL_ERR_NO_ERROR*/
} sibles_svc_search_rsp_t;

/**
 * @brief The strcture of event #SIBLES_REMOTE_CONNECTED_IND.
 */
typedef struct
{
    uint8_t conn_idx;               /*!< connection index*/
} sibles_remote_connected_ind_t;

typedef struct
{
    uint8_t conn_idx;
    uint16_t start_handle;
    uint16_t end_handle;
} sibles_remote_svc_change_ind_t;

/**
 * @brief Discovery Characteristic Descriptor indication Structure.
 */
struct sibles_disc_char_desc_ind
{

    uint16_t attr_hdl;          /*!< database element handle */
    uint8_t uuid_len;           /*!< UUID length */
    uint8_t uuid[ATT_UUID_128_LEN]; /*!< character UUID*/
};

/**
 * @brief The strcture of #SIBLES_SEARCH_SVC_RSP.
 */
typedef struct
{
    uint16_t attr_hdl;          /*!<  Database element handle */
    uint16_t pointer_hdl;       /*!<  Pointer attribute handle to UUID*/
    uint8_t prop;               /*!<  Properties*/
    uint8_t uuid_len;           /*!<  UUID length*/
    uint8_t uuid[ATT_UUID_128_LEN]; /*!<  Characteristic UUID*/
    uint8_t desc_count;             /*!<  Descriptor count */
    struct sibles_disc_char_desc_ind desc[__ARRAY_EMPTY]; /*!< Descriptors */
} sibles_svc_search_char_t;

/**
 * @brief The strcture of #SIBLES_REGISTER_REMOTE_SVC_RSP.
 */
typedef struct
{
    uint8_t conn_idx;           /*!< Connection index */
    uint8_t status;              /*!< Status of service register response, @see enum hl_err */
} sibles_register_remote_svc_rsp_t;

/**
 * @brief The data strcture of Sibles GATT write.
 */
typedef struct
{
    sibles_write_type_t write_type; /*!< Character write type */
    uint16_t handle;                /*!< Character handle */
    uint16_t len;                   /*!< Length of value */
    uint8_t *value;                 /*!< Value content*/
} sibles_write_remote_value_t;


/**
 * @brief The strcture of #SIBLES_REMOTE_EVENT_IND.
 */
typedef struct
{
    uint8_t conn_idx;
    uint8_t type;                   /*!<  Event Type */
    uint16_t length;                /*!<  Data length */
    uint16_t handle;                /*!<  Attribute handle  */
    uint8_t *value;                 /*!<  Event Value  */
} sibles_remote_event_ind_t;

/**
 * @brief The data strcture of Sibles GATT read.
 */
typedef struct
{
    sibles_read_type_t read_type;   /*!< Character read type */
    uint16_t handle;                /*!< Character handle */
    uint16_t offset;                /*!< offset of read content */
    uint16_t length;                /*!< Max length of value read*/
} sibles_read_remote_value_req_t;

/**
 * @brief The strcture of #SIBLES_READ_REMOTE_VALUE_RSP.
 */
typedef struct
{
    uint8_t conn_idx;
    uint16_t handle;                /*!<  Attribute handle */
    uint16_t offset;                /*!<  Read offset */
    uint16_t length;                /*!<  Read length */
    uint8_t *value;                 /*!<  Handle value */
} sibles_read_remote_value_rsp_t;

/**
 * @brief The strcture of #SIBLES_MTU_EXCHANGE_IND.
 */
typedef struct
{
    uint8_t conn_idx;
    uint16_t mtu;                  /*!<  Exchanged MTU value */
} sibles_mtu_exchange_ind_t;

typedef struct
{
    /// Event Type
    uint8_t type;
    /// Data length
    uint16_t length;
    /// Attribute handle
    uint16_t handle;
    /// Event Value
    uint8_t *value;
} sibles_event_ind_t;

/**
 * @brief The strcture of set cbk parameter.
 */
typedef struct
{
    uint8_t idx;                   /*!< Index to service */
    uint16_t len;                  /*!< Length to be written */
    uint16_t offset;               /*!< The offset of which data has to be written */
    uint8_t *value;                /*!< Handle value */
} sibles_set_cbk_t;

/**
  * @brief  SIBLES attribte get callback. (uint8_t conn_idx, uint8_t idx, uint16_t *len)
  * @param[in]  conn_idx to associated BLE link.
  * @param[in]  idx Index to service.
  * @param[in,out] len Length of attribute.
  * @retval uint16_t* Pointer to attribute.
  */
typedef uint8_t *(*sibles_get_cbk)(uint8_t, uint8_t, uint16_t *);

/**
  * @brief  SIBLES attribte set callback. (uint8_t conn_idx, sibles_set_cbk_t *para)
  * @param[in]  conn_idx to associated BLE link.
  * @param[in]  para of set callback parameters.
  * @retval uint8_t 0: Success, otherwise, failed.
  */
typedef uint8_t (*sibles_set_cbk)(uint8_t, sibles_set_cbk_t *);

/**
  * @brief  SIBLES remote service callback. (uint8_t idx, uint16_t len, uint8_t * value)
  * @param[in]  event_id Event ID.
  * @param[in]  data Pointer to data parameter.
  * @param[in]  len Length of data.
  * @retval uint8_t 0: Success, otherwise, failed.
  */
typedef int (*sibles_remote_svc_cbk)(uint16_t event_id, uint8_t *data, uint16_t len);

typedef struct
{
    uint8_t conn_idx;
    /**
     * Current value of the Client Characteristic Configuration descriptor for the Service
     * Changed characteristic
     */
    uint16_t ind_cfg;
} sibles_svc_changed_cfg_t;


typedef struct
{
    uint8_t status;
} sibles_change_bd_addr_rsp_t;

typedef struct
{
    uint16_t handle;
    uint8_t status;
} sibles_att_update_perm_ind_t;

typedef struct
{
    uint16_t handle;
    uint8_t status;
} sibles_att_set_visibility_ind_t;

typedef struct
{
    uint8_t state;
    uint8_t uuid[ATT_UUID_128_LEN];
    uint8_t uuid_len;
    uint16_t start_handle;
    uint16_t end_handle;
} sibles_local_svc_t;

typedef struct
{
    // see @diss_msg_id
    uint8_t value;
    uint8_t len;
    uint8_t *data;
} sibles_set_dis_t;

typedef struct
{
    uint8_t value;
    uint8_t status;
} sibles_set_dis_rsp_t;

typedef struct
{
    uint8_t conn_idx;
    // see @sibles_attribute_code_t
    uint8_t op_code;
    uint16_t handle;
    // see @sibles_att_error_reason_t
    uint8_t reason;
} sibles_att_error_ind_t;

/**
* @} sible_types
*/


/** @defgroup sible_gatt_func Sibles GATT functions
  * @{
  */

/**
  * @brief  Register 16 bits GATT service.
  * @param[in]  svc service record for GATT.
  * @retval sibles_hdl Service handle.
  */
sibles_hdl sibles_register_svc(sibles_register_svc_t *svc);

/**
  * @brief  Register 128 bits GATT service.
  * @param[in]  svc 128 bist service record.
  * @retval sibles_hdl Service handle.
  */
sibles_hdl sibles_register_svc_128(sibles_register_svc_128_t *svc);

/**
  * @brief  Register GATT service callback.
  * @param[in]  hdl Service handle.
  * @param[in]  gcbk Service attribute get callback.
  * @param[in]  scbk Service attribute set callback.
  */
void sibles_register_cbk(sibles_hdl hdl, sibles_get_cbk gcbk, sibles_set_cbk scbk);

/**
  * @brief  Update service attribute cache.
  * The content will be used as response to next remote get, get callback will not be called
  * @param[in]  conn_idx Connection index for the service.
  * @param[in]  value Attribute content buffer.
  */
int sibles_set_value(uint8_t conn_idx, sibles_value_t *value);

/**
  * @brief  Send new service attribute to remote using gatt notify.
  * @param[in]  conn_idx Connection index for the service.
  * @param[in]  value Attribute content buffer.
  */
int sibles_write_value(uint8_t conn_idx, sibles_value_t *value);

/**
  * @brief  Send new service attribute to remote using gatt indicate.
  * @param[in]  conn_idx Connection index for the service.
  * @param[in]  value Attribute content buffer.
  */
int sibles_write_value_with_rsp(uint8_t conn_idx, sibles_value_t *value);

/**
  * @brief  Stop GATT service.
  * @param[in]  hdl Service handle.
  */
void sibles_stop_svc(sibles_hdl hdl);

/**
  * @brief  Search peer device's service via uuid.
  * @param[in]  conn_idx connection index.
  * @param[in]  uuid_len uuid length.
  * @param[in]  uuid service uuid.
  * @retval result.
  */
int8_t sibles_search_service(uint8_t conn_idx, uint8_t uuid_len, uint8_t *uuid);

/**
 * @brief  Find expect descriptor in characteristic.
  * @param[in]  char characteristic content for search.
  * @param[in]  descriptor descriptor want to find.
  * @retval result.
 */
uint16_t sibles_descriptor_handle_find(sibles_svc_search_char_t *chara, uint16_t descriptor);

/**
  * @brief  Register callback for a specified service of peer device.
  * @param[in]  conn_idx connection index.
  * @param[in]  start_hdl service start handle.
  * @param[in]  end_hdl service end handle.
  * @param[in]  callback received response or indication from GATT server.
  * @retval result.
  */
uint16_t sibles_register_remote_svc(uint8_t conn_idx, uint16_t start_hdl, uint16_t end_hdl, sibles_remote_svc_cbk callback);
void sibles_unregister_remote_svc(uint8_t conn_idx, uint16_t start_hdl, uint16_t end_hdl, sibles_remote_svc_cbk callback);
void sibles_send_remote_svc_change_ind(uint8_t conn_idx, uint16_t start_hdl, uint16_t end_hdl);

/**
  * @brief  Write value to GATT server.
  * @param[in]  remote_handle specified GATT handle.
  * @param[in]  conn_idx connection index.
  * @param[in]  value write value.
  * @retval result.
  */
int8_t sibles_write_remote_value(uint16_t remote_handle, uint8_t conn_idx, sibles_write_remote_value_t *value);

/**
  * @brief  Read value from GATT server.
  * @param[in]  remote_handle specified GATT handle.
  * @param[in]  conn_idx connection index.
  * @param[in]  value read req parameter.
  * @retval result.
  */
int8_t sibles_read_remote_value(uint16_t remote_handle, uint8_t conn_idx, sibles_read_remote_value_req_t *value);

/**
  * @brief  Exchange MTU size.
  * @param[in]  conn_idx connection index.
  * @retval result.
  */
uint8_t sibles_exchange_mtu(uint8_t conn_idx);


/**
  * @brief  Enable sible BLE system. It will trigger #ble_power_on().
  */
void sifli_ble_enable(void);

void sibles_send_svc_changed_ind(uint8_t conn_idx, uint16_t start_handle, uint16_t end_handle);

#if defined(SOC_SF32LB58X) || defined(SOC_SF32LB56X) || defined(SOC_SF32LB52X) || defined (BSP_USING_PC_SIMULATOR)
    void ble_gap_wlan_coex_enable(void);

    void sibles_set_trc_cfg(sibles_trc_cfg_t cfg_mode, uint32_t mask_ext);

    uint8_t sibles_change_bd_addr(sibles_change_bd_addr_type_t type, sibles_change_bd_addr_method_t method, bd_addr_t *addr);

    #ifdef BT_FINSH
        uint8_t bt_addr_convert_to_general(BTS2S_BD_ADDR *src_addr, bd_addr_t *dest_addr);
        uint8_t bt_addr_convert_to_bts(bd_addr_t        *src_addr, BTS2S_BD_ADDR *dest_addr);
    #endif

#else
    #define sibles_set_trc_cfg(cfg_mode, mask_ext)
#endif

/**
  * @brief  set static random addr type without reboot.
  * @param[in]  conn_idx connection index.
  * @param[in]  value address and type.
  */
void sibles_set_random_addr(uint8_t conn_idx, uint8_t *value);

uint8_t sibles_get_tx_pkts(void);

uint8_t sibles_get_gatt_handle(sibles_hdl svc_handle, uint8_t idx);
uint8_t sibles_get_gatt_handle_by_uuid(uint16_t attr_uuid);
uint16_t sibles_get_uuid_by_attr(uint8_t attr);
sibles_hdl sibles_get_sible_handle_and_index_by_attr(uint8_t attr, uint8_t *write_index);
void sibles_get_all_gatt_handle(sibles_local_svc_t *svc);
void sibles_update_att_permission(uint16_t handle, uint16_t access_mask, uint16_t perm);
void sibles_set_att_visibility(uint16_t handle, uint8_t hide);

/**
  * @brief  User implmentation function. Stack will call this function to get max mtu.
  *         if not implmentation, stack will use default value as 1024.
  * @return Maximum MTU value you want to use,
  *         should greater than or equal to 23, less than or equal to 1024.
  */
uint16_t ble_max_mtu_get(void);

/**
  * @brief  User implmentation function. Stack will enable or disable dis depend on this return value.
  *         default is disable.
  * @return 0 to disable Device Information Service,
  *         none 0 to enable Device Information Service.
  */
uint8_t ble_app_dis_enable(void);

/**
  * @brief     set device information for dis.
  * @param[in] param value, len and data to set
  */
void sibles_set_dev_info(sibles_set_dis_t *param);

#ifdef _MSC_VER
    //#define __PACKED_STRUCT struct
#endif


/// @} sible_gatt_func
/// @} sibles_service
/// @} sibles


#endif // BF0_SIBLES_H_


