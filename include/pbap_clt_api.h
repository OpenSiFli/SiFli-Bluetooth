/*
 * SPDX-FileCopyrightText: 2019-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _PBAP_CLT_API_H_
#define _PBAP_CLT_API_H_

#ifdef __cplusplus
extern "C" {
#endif

#define PBAPC_MAX_PASSWORD_LEN           0x20
#define PBAPC_MAX_PASSWORD_USERID_LEN    0x20

enum
{
    BTS2MU_PBAP_CLT_CONN_CFM = BTS2MU_START,
    BTS2MU_PBAP_CLT_DISC_IND,
    BTS2MU_PBAP_CLT_AUTH_IND,
    BTS2MU_PBAP_CLT_ABORT_CFM,

    BTS2MU_PBAP_CLT_SET_PB_CFM,
    BTS2MU_PBAP_CLT_PULL_PB_BEGIN_IND,
    BTS2MU_PBAP_CLT_PULL_PB_NEXT_IND,
    BTS2MU_PBAP_CLT_PULL_PB_COMPLETE_IND,
    BTS2MU_PBAP_CLT_PULL_VCARD_BEGIN_IND,
    BTS2MU_PBAP_CLT_PULL_VCARD_NEXT_IND,
    BTS2MU_PBAP_CLT_PULL_VCARD_COMPLETE_IND,
    BTS2MU_PBAP_CLT_PULL_VCARD_LIST_BEGIN_IND,
    BTS2MU_PBAP_CLT_PULL_VCARD_LIST_NEXT_IND,
    BTS2MU_PBAP_CLT_PULL_VCARD_LIST_COMPLETE_IND,
    BTS2MU_PBAP_CLT_FOLDER_VERSION_IND,
};

typedef enum
{
    PBAPC_SUCCESS = 0,
    PBAPC_FAIL,
    PBAPC_ABORT,
    PBAPC_NOT_IDLE,
    PBAPC_WRONG_STATE,
    PBAPC_SDP_FAIL,
    PBAPC_REMOTE_DISCONNECT,
    PBAPC_UNAUTHORISED,
    PBAPC_PULL_BADREQUEST,
    PBAPC_PULL_FORBIDDEN,
    PBAPC_SPB_NO_REPOSITORY,
    PBAPC_SPB_NOT_FOUND,
    PBAPC_VCL_NO_PARAM_RESOURCES,
    PBAPC_VCL_NO_PBOOK_FOLDER,
    PBAPC_VCL_INVALID_PBOOK,
    PBAPC_VCE_NO_PARAM_RESOURCES,
    PBAPC_VCE_NO_NAME_RESOURCES,
    PBAPC_PPB_NO_PARAM_RESOURCES,
    PBAPC_PPB_NO_NAME_RESOURCES,
    PBAPC_PPB_NO_REQUIRED_NAME,
    PBAPC_PPB_NO_REPOSITORY
} BTS2E_PBAPC_RESULT_CODE;

typedef struct
{
    U16          type;          /* message identity */
    U16          mfs;           /* maximum size of packet transferable during this connection */
    U8           res;           /* connection result */
    U8           supp_repos;    /* remote side supported repositories */
    BTS2S_BD_ADDR bd;           /* remote side Bluetooth address */
} BTS2S_PBAP_CLT_CONN_CFM;

typedef struct
{
    U16 type;                   /* message identity */
    U8 res;                     /* disconnection reason */
    BTS2S_BD_ADDR bd;
} BTS2S_PBAP_CLT_DISC_IND;

typedef struct
{
    U16 type;                   /* message identity */
} BTS2S_PBAP_CLT_AUTH_IND;

typedef struct
{
    U16 type;                   /* message identity */
    U8  res;                    /* abort reason */
} BTS2S_PBAP_CLT_ABORT_CFM;

typedef struct
{
    U16 type;                   /* BTS2MU_PBAP_CLT_SET_PB_CFM message identity */
    U8 res;                     /* operation result */
    BTS2S_BD_ADDR bd;
} BTS2S_PBAP_CLT_SET_PB_CFM;

typedef struct
{
    U16 type;                   /* message identity */
    U16 pbook_size;             /* the number of indexes in the device phone book */
    U32 totalLength;            /* the data total length */
    BOOL is_final_packet;       /* is last packet */
    U8 new_missed;              /* the number of missed calls */
    BTS2S_BD_ADDR bd;
    U16 body_data_length;            /* the length of payload */
    U8 body_data[];                   /* payload */
} BTS2S_PBAP_CLT_PULL_PB_BEGIN_IND;

typedef struct
{
    U16 type;                           /* message identity */
    BOOL is_final_packet;               /* is last packet */
    U16 body_data_length;               /* the length of payload */
    U8 body_data[];                     /* payload */
} BTS2S_PBAP_CLT_PULL_PB_NEXT_IND;

typedef struct
{
    U16 type;                   /* message identity */
    U32 totalLength;            /* the data total length */
    BTS2S_BD_ADDR bd;
    BOOL is_final_packet;             /* is last packet */
    U16 body_data_length;            /* the length of payload */
    U8 body_data[];                   /* payload */
} BTS2S_PBAP_CLT_PULL_VCARD_BEGIN_IND;

typedef struct
{
    U16 type;                   /* message identity */
    BOOL is_final_packet;             /* is last packet */
    U16 body_data_length;            /* the length of payload */
    U8 body_data[];                   /* payload */
} BTS2S_PBAP_CLT_PULL_VCARD_NEXT_IND;

typedef struct
{
    U16 type;                   /* message identity */
    U16 pbook_size;             /* the number of indexes in the device phone book */
    U32 totalLength;            /* the data total length */
    BOOL is_final_packet;             /* is last packet */
    U8 new_missed;              /* the number of missed calls */
    BTS2S_BD_ADDR bd;
    U16 body_data_length;                /* the length of payload */
    U8 body_data[];                   /* payload */
} BTS2S_PBAP_CLT_PULL_VCARD_LIST_BEGIN_IND;

typedef struct
{
    U16 type;                   /* message identity */
    BOOL is_final_packet;               /* is last packet */
    U16 body_data_length;               /* the length of payload */
    U8 body_data[];                     /* payload */
} BTS2S_PBAP_CLT_PULL_VCARD_LIST_NEXT_IND;

typedef struct
{
    U16 type;                   /* message identity */
    U8  res;                    /* result code */
    BTS2S_BD_ADDR bd;
} BTS2S_PBAP_CLT_PULL_CMPT_IND;

typedef struct
{
    U16 type;                   /* message identity */
    U8 primary_version_counter[16];
    U8 secondary_version_counter[16];
    U8 database_identifier[16];
} BTS2S_PBAP_CLT_FOLDER_VERSION_IND;
/* PBAP Phonebook object: repository + phonebook */
typedef struct
{
    U8 repository;
    U8 phonebook;
} BTS2S_PBAP_PHONEBOOK_OBJECT;

/* Application Parameter: PropertySelector, Tag 0x06, 8 bytes */
typedef struct
{
    U32 filter_lo;
    U32 filter_hi;
} BTS2S_PBAP_PROPERTY_SELECTOR;

/* Application Parameters: basic PullPhonebook parameters */
typedef struct
{
    /* Application Parameter: Format, Tag 0x07, 1 byte */
    /* for oder if cmd vcardlisting*/
    U8 format;

    /* Application Parameter: MaxListCount, Tag 0x04, 2 bytes */
    U16 max_list;

    /* Application Parameter: ListStartOffset, Tag 0x05, 2 bytes */
    U16 listStart;

    /* Application Parameter: ResetNewMissedCalls, Tag 0x0F, 1 byte */
    U8 reset_new_missed_calls;
} BTS2S_PBAP_PULL_PB_BASIC_PARAM;

/* OBEX Header: Single Response Mode / Single Response Mode Parameter */
typedef struct
{
    /* OBEX Header: Single Response Mode */
    /* PBAP_SRM_DEFAULT means do not send SRM header */
    U8 srm;

    /* OBEX Header: Single Response Mode Parameter */
    /* PBAP_SRMP_DEFAULT means do not send SRMP header */
    U8 srmp;
    U8 is_final;
} BTS2S_PBAP_SRM_PARAM;

/* Application Parameters: vCardSelector + vCardSelectorOperator */
typedef struct
{
    /* Application Parameter: vCardSelector, Tag 0x0C, 8 bytes */
    U32 vcard_selector_lo;
    U32 vcard_selector_hi;

    /* Application Parameter: vCardSelectorOperator, Tag 0x0E, 1 byte */
    U8 vcard_selector_operator;
} BTS2S_PBAP_VCARD_SELECTOR_PARAM;

typedef struct
{
    /* Application Parameter: SearchProperty/SearchAttribute, Tag 0x03, 1 byte */
    U8 srch_attr;

    /* Application Parameter: SearchValue, Tag 0x02, variable length */
    U16 srchval_len;
    U8 *srch_val;
} BTS2S_PBAP_CLT_SEARCH_PARAM;
/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      Initiate a connect towards the peer device.
 *
 * INPUT:
 *      U16 max_pkt_size: indicates the maximum packet size that local device can receive
 *      BOOL auth_flag: whether or not authenticate remote device.
 *      const BTS2S_BD_ADDR *bd: destination Bluetooth address.
 *      const U8 *pass_word: authenticate password, use if auth_flag set.
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      Message BTS2MU_PBAP_CLT_CONN_CFM with structure BTS2S_PBAP_CLT_CONN_CFM
 *      will be received as a confirmation.
 *
 *----------------------------------------------------------------------------*/
void pbap_clt_conn_req(U16 max_pkt_size,
                       BOOL auth_flag,
                       BTS2S_BD_ADDR *bd,
                       U8 *pass_word);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      Send disconnect command with the server.
 *
 * INPUT:
 *      void.
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      Message BTS2MU_PBAP_CLT_DISC_CFM with structure
 *      BTS2S_PBAP_CLT_DISC_CFM will be received as a confirmation.
 *
 *----------------------------------------------------------------------------*/
void pbap_clt_disc_req(void);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      PBAP client send authentication response.
 *
 * INPUT:
 *      const U8 *rsp_password: a pointer to the response password.
 *      U16 rsp_password_len: response password length.
 *      const char *rsp_user_id: user id.
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      Message BTS2MU_PBAP_CLT_AUTH_IND with structure BTS2S_PBAP_CLT_AUTH_IND
 *      will be received as a hint to call this function.
 *
 *----------------------------------------------------------------------------*/
void pbap_clt_auth_rsp(const U8 *rsp_password,
                       U16 rsp_password_len,
                       const char *rsp_user_id);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      Request to abort the current multi-packet operation.
 *
 * INPUT:
 *      void.
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      Message BTS2MU_PBAP_CLT_ABORT_CFM with structure
 *      BTS2S_PBAP_CLT_ABORT_CFM will be received as a confirmation.
 *
 *----------------------------------------------------------------------------*/
void pbap_clt_abort_req(void);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      Change the current active phonebook.
 *
 * INPUT:
 *      U8 repository: phone book repositories, see BTS2E_PBAP_PHONE_REPOSITORY
 *                     in pbap_public.h.
 *      U8 phonebook: phone book objects, see BTS2E_PBAP_PHONE_BOOK in pbap_public.h.
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      1. To change to a different phonebook in the same repository,
 *         param repository may use pbap_current.
 *      2. BTS2S_PBAP_CLT_SET_PB_CFM message will be received by the application.
 *
 *----------------------------------------------------------------------------*/
void pbap_clt_set_pb_req(U8 repository, U8 phonebook);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      Retrieve an entire phone book object from the specific repositories.
 *
 * INPUT:
 *      U8 repository: phone book repositories, see BTS2E_PBAP_PHONE_REPOSITORY
 *                     in pbap_public.h.
 *      U8 phonebook:  phone book objects.
 *      U32 filter_lo: PBAP Parameter filter mask low 32 bits. see pbap_public.h.
 *                     Use zero to not sent the parameter.
 *      U32 filter_hi: PBAP Parameter filter mask high 32 bits.
 *                     Use zero to not sent the parameter.
 *      U8 format: indicate the requested format (vCard 2.1 or 3.0) to be returned
 *                 in the operation. The format vCard 2.1 shall be the default format.
 *      U16 max_list:  indicate the maximum number of entries that the PCE can handle.
 *      U16 list_start_offset: the offset of the first entry of the phonebook, the
 *      first entry of the Phonebook object that would be returned if this parameter
 *      was not specified in the request.
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      Message BTS2MU_PBAP_CLT_PULL_PB_BEGIN_IND with structure
 *      BTS2S_PBAP_CLT_PULL_PB_BEGIN_IND will be received by the application
 *      when the first packet arrives.
 *
 *      Message BTS2MU_PBAP_CLT_PULL_PB_COMPLETE_IND with structure
 *      BTS2S_PBAP_CLT_PULL_CMPT_IND will be received after the last packet.
 *
 *      If remote side reject our request, application will receive message
 *      BTS2MU_PBAP_CLT_PULL_PB_COMPLETE_IND only.
 *
 *      If param max_list is 0 and the pbook is MCH, then NewMissedCalls will
 *      be returned in structure BTS2S_PBAP_CLT_PULL_CMPT_IND.
 *
 *----------------------------------------------------------------------------*/
void pbap_clt_pull_pb_req(U8 repository,
                          U8 phonebook,
                          U32 filter_lo,
                          U32 filter_hi,
                          U8 format,
                          U16 max_list,
                          U16 list_start_offset);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      Start to get a specific vCard from the current phonebook.
 *
 * INPUT:
 *
 *      U8 *vcard_name: VCARD name(*.vcf) is a null terminated Unicode text string describing
 *                      the name of the object.
 *      U32 filter_lo: PBAP Parameter filter mask low 32 bits.
 *                     Use zero to not sent the parameter. see pbap_public.h.
 *      U32 filter_hi: PBAP Parameter filter mask high 32 bits.
 *                     Use zero to not sent the parameter. see pbap_public.h.
 *      U8 format: indicate the requested format (vCard 2.1 or 3.0) to be returned in the operation.
 *                 The format vCard 2.1 shall be the default format.
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      1. If the filter does not include the mandatory fields as required
 *         by the specified format, these will be added.
 *
 *      2. Message BTS2MU_PBAP_CLT_PULL_VCARD_BEGIN_IND with structure
 *         BTS2S_PBAP_CLT_PULL_VCARD_BEGIN_IND will be received by the application
 *         when the first packet arrives.
 *
 *      3. Message BTS2MU_PBAP_CLT_PULL_VCARD_COMPLETE_IND message with structure
 *         BTS2S_PBAP_CLT_PULL_CMPT_IND will be received after the last packet arrived.
 *
 *----------------------------------------------------------------------------*/
void pbap_clt_pull_vcard_req(U8 *vcard_name,
                             U32 filter_lo,
                             U32 filter_hi,
                             U8 format);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      Start to get the vCard listing for the current phonebook.
 *
 * INPUT:
 *      U8 order: indicate to the Server, which sorting order shall be used.
 *                Use 'pbap_order_default' for default order, the default order is "Indexed".
 *      U8 pbook: PhoneBook folder to retrieve. Use 'pbap_b_unknown' for the current folder.
 *      U8 srchAttr: Attribute to search. Use 'pbap_a_unknown' for default search attribute.
 *      U8 srchVal: a UTF-8 text string, which matches the value of the attribute indicated
 *                  using the Search/Attribute.
 *                  All the vCards shall be returned if this header is not specified.
 *      U16 size_srchVal: Length of the search value.
 *      U16 maxList: Maximum number of entries that the PCE can handle.
 *      U16 list_start_offset: the offset of the first entry of the phonebook, the first entry of the Phonebook
 *      object that would be returned if the ListStartOffset parameter was not specified in the request.
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      Message BTS2MU_PBAP_CLT_PULL_VCARD_LIST_BEGIN_IND with structure
 *      BTS2S_PBAP_CLT_PULL_VCARD_LIST_BEGIN_IND will be received by the application
 *      when the first packet arrives.
 *
 *      Message BTS2MU_PBAP_CLT_PULL_VCARD_LIST_COMPLETE_IND with structure
 *      BTS2S_PBAP_CLT_PULL_CMPT_IND will be received after the last packet.
 *
 *      If remote side reject our request, application will receive message
 *      BTS2MU_PBAP_CLT_PULL_PB_COMPLETE_IND only.
 *
 *      If param max_list is 0 and the pbook is MCH, then NewMissedCalls will
 *      be returned in structure BTS2S_PBAP_CLT_PULL_CMPT_IND.
 *
 *----------------------------------------------------------------------------*/
void pbap_clt_pull_vcard_list_req(U8 order,
                                  U8 pbook,
                                  U8 srchAttr,
                                  U8 *srchVal,
                                  U16 size_srchVal,
                                  U16 max_list,
                                  U16 list_start_offset);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *       Get the next vCard listing packet for the current phonebook.
 *
 * INPUT:
 *      void.
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      Message BTS2MU_PBAP_CLT_PULL_PB_NEXT_IND with structure
 *      BTS2S_PBAP_CLT_PULL_PB_NEXT_IND will be received as a confirmation if
 *      continuation packets exist.
 *
 *      Message BTS2MU_PBAP_CLT_PULL_PB_COMPLETE_IND with structure
 *      BTS2S_PBAP_CLT_PULL_CMPT_IND will be received after the last packet arrived.
 *
 *----------------------------------------------------------------------------*/
void pbap_clt_pull_pb_next_req(void);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      Get the next vCard entry packet from the current phonebook.
 *
 * INPUT:
 *      void.
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      Message BTS2MU_PBAP_CLT_PULL_VCARD_NEXT_IND with structure
 *      BTS2S_PBAP_CLT_PULL_VCARD_NEXT_IND will be received by the application
 *      for each packet after the first.
 *
 *      Message BTS2MU_PBAP_CLT_PULL_VCARD_COMPLETE_IND with structure
 *      BTS2S_PBAP_CLT_PULL_CMPT_IND will be received after the last packet arrived.
 *
 *----------------------------------------------------------------------------*/
void pbap_clt_pull_vcard_next_req(void);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *     Get the next vCard listing packet for the current phonebook.
 *
 * INPUT:
 *      void.
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      Message BTS2MU_PBAP_CLT_PULL_VCARD_LIST_NEXT_IND with structure
 *      BTS2S_PBAP_CLT_PULL_VCARD_LIST_NEXT_IND will be received by the application
 *      for each packet after the first.
 *
 *      Message BTS2MU_PBAP_CLT_PULL_VCARD_LIST_COMPLETE_IND with structure
 *      BTS2S_PBAP_CLT_PULL_CMPT_IND will be received after the last packet arrived.
 *
 *----------------------------------------------------------------------------*/
void pbap_clt_pull_vcard_list_next_req(void);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      Retrieve a phone book object from the specified PBAP repository with
 *      extended PullPhonebook parameters.
 *
 *      This API is an extended version of pbap_clt_pull_pb_req(). It groups
 *      PullPhonebook parameters into several structures, including phonebook
 *      object, property selector, basic application parameters, vCard selector,
 *      and SRM/SRMP parameters.
 *
 *      If one of the parameter structures is NULL, this function uses default
 *      values for the corresponding fields.
 *
 * INPUT:
 *      BTS2S_PBAP_PHONEBOOK_OBJECT *obj:
 *          Phonebook object information.
 *          - repository: phone book repository, such as PBAP_LOCAL or PBAP_SIM1.
 *          - phonebook : phone book object, such as PBAP_PB, PBAP_ICH,
 *                        PBAP_OCH, PBAP_MCH or PBAP_CCH.
 *          If obj is NULL, PBAP_LOCAL/PBAP_PB is used by default.
 *
 *      BTS2S_PBAP_PROPERTY_SELECTOR *selector:
 *          Application Parameter: PropertySelector, Tag 0x06, 8 bytes.
 *          - filter_lo: low 32 bits of the vCard property selector.
 *          - filter_hi: high 32 bits of the vCard property selector.
 *          If selector is NULL, both filter_lo and filter_hi are set to 0,
 *          which means the PropertySelector parameter is not used.
 *
 *      BTS2S_PBAP_PULL_PB_BASIC_PARAM *para:
 *          Basic PullPhonebook application parameters.
 *          - format: vCard format, such as PBAP_FORMAT_21 or PBAP_FORMAT_30.
 *          - max_list: MaxListCount, Tag 0x04, 2 bytes.
 *              0      means only request PhonebookSize and do not download
 *                     vCard body data.
 *              0xFFFF means no restriction on the number of returned entries.
 *          - listStart: ListStartOffset, Tag 0x05, 2 bytes.
 *          - reset_new_missed_calls: ResetNewMissedCalls, Tag 0x0F, 1 byte.
 *          If para is NULL, default values are used:
 *              format = PBAP_FORMAT_21,
 *              max_list = 0xFFFF,
 *              listStart = 0,
 *              reset_new_missed_calls = 0.
 *
 *      BTS2S_PBAP_VCARD_SELECTOR_PARAM *vcard_selector:
 *          Application parameters for PBAP 1.2 or later.
 *          - vcard_selector_lo: low 32 bits of vCardSelector, Tag 0x0C.
 *          - vcard_selector_hi: high 32 bits of vCardSelector, Tag 0x0C.
 *          - vcard_selector_operator: vCardSelectorOperator, Tag 0x0E.
 *          If vcard_selector is NULL, these fields are set to 0 and not used.
 *
 *      BTS2S_PBAP_SRM_PARAM *srm_params:
 *          OBEX Single Response Mode parameters.
 *          - srm : Single Response Mode header.
 *          - srmp: Single Response Mode Parameter header.
 *          If srm_params is NULL, PBAP_SRM_DEFAULT/PBAP_SRMP_DEFAULT are used,
 *          which means SRM/SRMP headers are not sent by default.
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      1. This function allocates BTS2S_PBAP_CLT_PULL_PB_REQ internally and
 *         sends it to PBAP client task by bts2_msg_put().
 *
 *      2. To query the total number of phonebook entries only, set:
 *             para->max_list = 0;
 *         The PSE should return PhonebookSize in Application Parameters.
 *
 *      3. To download the full phonebook, set:
 *             para->max_list = 0xFFFF;
 *
 *      4. The result is reported by:
 *             BTS2MU_PBAP_CLT_PULL_PB_BEGIN_IND
 *             BTS2MU_PBAP_CLT_PULL_PB_NEXT_IND
 *             BTS2MU_PBAP_CLT_PULL_PB_COMPLETE_IND
 *
 *----------------------------------------------------------------------------*/
void pbap_clt_pull_pb_req_ext(BTS2S_PBAP_PHONEBOOK_OBJECT *obj,
                              BTS2S_PBAP_PROPERTY_SELECTOR *selector, BTS2S_PBAP_PULL_PB_BASIC_PARAM *para,
                              BTS2S_PBAP_VCARD_SELECTOR_PARAM *vcard_selector, BTS2S_PBAP_SRM_PARAM *srm_params);

void  pbap_clt_pull_vcard_list_req_ext(U8 pbook,
                                       BTS2S_PBAP_PULL_PB_BASIC_PARAM *para,
                                       BTS2S_PBAP_SRM_PARAM *srm_params,
                                       BTS2S_PBAP_CLT_SEARCH_PARAM *search_para,
                                       BTS2S_PBAP_VCARD_SELECTOR_PARAM *vcard_selector);
void pbap_clt_pull_pb_next_req_ext(uint8_t srm, uint8_t srmp, uint8_t final);
U16 pbap_clt_get_max_mtu(void);


#ifdef __cplusplus
}
#endif

#endif






