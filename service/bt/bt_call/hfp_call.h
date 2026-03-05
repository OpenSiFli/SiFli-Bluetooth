#ifndef _HFP_CALL_H_
#define _HFP_CALL_H_

#ifdef __cplusplus
extern "C" {
#endif
#ifdef RT_USING_BT
#include "bt_rt_device.h"
#endif
#ifdef CFG_HFP_CALL_STATUS
#include <stdint.h>
#include "hfp_call_service.h"

/*!
    @brief The call state of a connection. Maps directly onto the call SM.
*/
typedef enum
{
    HFP_CALL_STATE_IDLE,
    HFP_CALL_STATE_INCOMING,
    HFP_CALL_STATE_INCOMING_HELD,
    HFP_CALL_STATE_OUTGOING,
    HFP_CALL_STATE_ACTIVE,
    HFP_CALL_STATE_TWC_INCOMING,    /* TWC: Three way call */
    HFP_CALL_STATE_TWC_OUTGOING,
    HFP_CALL_STATE_HELD_ACTIVE,
    HFP_CALL_STATE_HELD_REMAINING,
    HFP_CALL_STATE_MULTIPARTY
} hfp_call_state;

// typedef enum
// {
//     HFP_NO_CALL_ACTIVE,                     /* No active call */
//     HFP_CALL_ACTIVE                  /* Active call */
// } hfp_call;

typedef enum
{
    HFP_CALLSETUP_IDLE,               /* No call currently being established.*/
    HFP_CALLSETUP_INCOMING,         /* HFP device currently ringing.*/
    HFP_CALLSETUP_OUTGOING,         /* Call currently being dialed.*/
    HFP_CALLSETUP_OUTGOING_ALERTING /* Remote end currently ringing.*/
} hfp_call_setup;

typedef enum
{
    HFP_HOLD_NO_CALL,                /* No call on hold */
    HFP_HOLD_ACTIVE,                 /* Held call and active call */
    HFP_HOLD_REMAINING               /* Held call and no active call  */
} hfp_call_hold;

typedef struct
{
    hfp_call_state              call_state;
    uint8_t                     first_incoming_call;
    uint16_t                    mux_id;
    hf_call_notify_func_t       callback;
} hfp_link_data;

/****************************************************************************
NAME
    hfp_call_dial_complete_hdl

DESCRIPTION
    Handle any call state changes required when dial functions complete

RETURNS
    void
*/
void hfp_call_ring_indication_hdl(hfp_link_data *link);
void hfp_call_dial_complete_hdl(hfp_link_data *link, uint8_t status);
void hfp_call_voicerec_disabled_hdl(hfp_link_data *link);
void hfp_call_indication_hdl(hfp_link_data *link, uint8_t call);
void hfp_callsetup_indication_hdl(hfp_link_data *link, uint8_t call_setup);
void hfp_callheld_indication_hdl(hfp_link_data *link, uint8_t call_hold);
void hfp_call_current_indication_hdl(hfp_link_data *link, uint8_t multiparty);
void hfp_call_response_hold_indication_hdl(hfp_link_data *link, uint8_t state);
#ifdef __cplusplus
}
#endif

#endif
#endif