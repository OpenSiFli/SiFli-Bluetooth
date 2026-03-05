/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    hfp_call_handler.c

DESCRIPTION


NOTES

*/
#include "bts2_app_inc.h"
#ifdef CFG_HFP_CALL_STATUS

/****************************************************************************
    Header files
*/
#include "hfp_call.h"
#include "ulog.h"
#include <string.h>
#include <stdint.h>

#include "bt_connection_manager.h"

/****************************************************************************
NAME
    hfp_call_status_update

DESCRIPTION
    Set the call state for a given link

RETURNS
    TRUE if state transition was allowed, FALSE otherwise
*/
void hfp_call_status_update(hfp_link_data *link, hfp_call_state call_state)
{
    if (link->call_state != call_state)
    {
        LOG_I("hfp_call_status_update call_state: %d,call_states: %d", link->call_state, call_state);
        link->call_state = call_state;
        if (link->callback)
        {
            link->callback(link->mux_id, link->call_state);
        }

    }
}

/****************************************************************************
NAME
    hfp_call_ring_indication_hdl

DESCRIPTION
    Handle any call state changes required when RING is received on link

RETURNS
    void
*/
void hfp_call_ring_indication_hdl(hfp_link_data *link)
{
    /* Enter incoming state if not there already */
    hfp_call_status_update(link, HFP_CALL_STATE_INCOMING);
}

/****************************************************************************
NAME
    hfp_call_dial_complete_hdl

DESCRIPTION
    Handle any call state changes required when dial functions complete

RETURNS
    void
*/
void hfp_call_dial_complete_hdl(hfp_link_data *link, uint8_t status)
{
    if (status == BTS2_SUCC)
    {
        hfp_call_status_update(link, HFP_CALL_STATE_OUTGOING);
    }
}


/****************************************************************************
NAME
    hfp_call_voicerec_disabled_hdl

DESCRIPTION
    Handle any call state changes required when voice recognition disabled

RETURNS
    void
*/
void hfp_call_voicerec_disabled_hdl(hfp_link_data *link)
{
    hfp_call_status_update(link, HFP_CALL_STATE_IDLE);
}

/****************************************************************************
NAME
    hfp_call_indication_hdl

DESCRIPTION
    Handle a call notification on link

RETURNS
    void
*/
void hfp_call_indication_hdl(hfp_link_data *link, uint8_t call)
{
    if (call)
    {
        switch (link->call_state)
        {
        case HFP_CALL_STATE_IDLE:
        case HFP_CALL_STATE_INCOMING:
        case HFP_CALL_STATE_OUTGOING:
            /* Valid to go to active call state */
            hfp_call_status_update(link, HFP_CALL_STATE_ACTIVE);
            break;
        default:
            /* Ignore in all other states */
            break;
        }
    }
    else
    {
        switch (link->call_state)
        {
        case HFP_CALL_STATE_IDLE:
            /* Idle already so stay there; However, still
            explicitly set the call state in order to inform
            the app of the latest state (which the vm may
            not be aware of, e.g. if a call disconnected
            during linkloss, the vm wouldn't know) */
            hfp_call_status_update(link, HFP_CALL_STATE_IDLE);
            break;
        case HFP_CALL_STATE_TWC_INCOMING:
            /* Drop back to incoming call state */
            hfp_call_status_update(link, HFP_CALL_STATE_INCOMING);
            break;
        case HFP_CALL_STATE_TWC_OUTGOING:
            /* Drop back to outgoing call state */
            hfp_call_status_update(link, HFP_CALL_STATE_OUTGOING);
            break;
        default:
            /* In all other cases, drop back to idle state */
            hfp_call_status_update(link, HFP_CALL_STATE_IDLE);
            break;
        }
    }
}


/****************************************************************************
NAME
    hfp_callsetup_indication_hdl

DESCRIPTION
    Handle a call setup notification on link

RETURNS
    void
*/
void hfp_callsetup_indication_hdl(hfp_link_data *link, uint8_t call_setup)
{
    switch (call_setup)
    {
    case HFP_CALLSETUP_IDLE:
        switch (link->call_state)
        {
        case HFP_CALL_STATE_INCOMING:
        case HFP_CALL_STATE_OUTGOING:
            /* Covered by no call indication, but as a backup */
            hfp_call_status_update(link, HFP_CALL_STATE_IDLE);
            break;
        case HFP_CALL_STATE_TWC_INCOMING:
            /* Try for CLCC to work out what happened to the incoming call */
            //get_cllcc
            bt_interface_get_remote_ph_num();
            /* Incoming TWC no longer there so assume it ended active for now */
            hfp_call_status_update(link, HFP_CALL_STATE_ACTIVE);
            break;
        case HFP_CALL_STATE_TWC_OUTGOING:
            /* In outgoing TWC call was already held, go to held call */
            hfp_call_status_update(link, HFP_CALL_STATE_HELD_ACTIVE);
            break;
        default:
            /* Ignore */
            break;
        }
        break;
    case HFP_CALLSETUP_INCOMING:
        switch (link->call_state)
        {
        case HFP_CALL_STATE_IDLE:
            /* We have an incoming call */
            hfp_call_status_update(link, HFP_CALL_STATE_INCOMING);
            break;
        case HFP_CALL_STATE_ACTIVE:
        case HFP_CALL_STATE_HELD_ACTIVE:
        case HFP_CALL_STATE_HELD_REMAINING:
        case HFP_CALL_STATE_MULTIPARTY:
            /* We have a second incoming call */
            hfp_call_status_update(link, HFP_CALL_STATE_TWC_INCOMING);
            break;
        default:
            /* Ignore */
            break;
        }
        break;
    case HFP_CALLSETUP_OUTGOING:
    case HFP_CALLSETUP_OUTGOING_ALERTING:
        switch (link->call_state)
        {
        case HFP_CALL_STATE_IDLE:
            /* We have an outgoing call */
            hfp_call_status_update(link, HFP_CALL_STATE_OUTGOING);
            break;
        case HFP_CALL_STATE_ACTIVE:
        case HFP_CALL_STATE_HELD_ACTIVE:
        case HFP_CALL_STATE_HELD_REMAINING:
            /* We have a second outgoing call */
            hfp_call_status_update(link, HFP_CALL_STATE_TWC_OUTGOING);
            break;
        default:
            /* Ignore */
            break;
        }
        break;
    default:
        /* Error */
        break;
    }
}


/****************************************************************************
NAME
    hfp_callheld_indication_hdl

DESCRIPTION
    Handle a held call notification on link

RETURNS
    void
*/
void hfp_callheld_indication_hdl(hfp_link_data *link, uint8_t call_hold)
{
    switch (call_hold)
    {
    case HFP_HOLD_NO_CALL:
        switch (link->call_state)
        {
        case HFP_CALL_STATE_TWC_OUTGOING:
            /* When outgoing TWC starts the active call is put
               on hold. If held call has ended then we have an
               outgoing call left... */
            hfp_call_status_update(link, HFP_CALL_STATE_OUTGOING);
            break;
        case HFP_CALL_STATE_HELD_REMAINING:
            /* Remaining call unheld, assume active and call
               none indication will take us back to idle */
            hfp_call_status_update(link, HFP_CALL_STATE_ACTIVE);
            break;
        case HFP_CALL_STATE_HELD_ACTIVE:
            /* Try for CLCC to work out what happened to the held call */
            // hfpSendCommonInternalMessage(HFP_INTERNAL_AT_CLCC_REQ, link);
            //to get clcc
            bt_interface_get_remote_ph_num();
            /* Held call no longer there so assume it was ended for now */
            hfp_call_status_update(link, HFP_CALL_STATE_ACTIVE);
            break;
        default:
            /* Ignore */
            break;
        }
        break;
    case HFP_HOLD_ACTIVE:
        switch (link->call_state)
        {
        case HFP_CALL_STATE_ACTIVE:
        case HFP_CALL_STATE_TWC_INCOMING:
        case HFP_CALL_STATE_HELD_REMAINING:
        case HFP_CALL_STATE_MULTIPARTY:
            /* Enter active + held state */
            hfp_call_status_update(link, HFP_CALL_STATE_HELD_ACTIVE);
            break;
        default:
            /* Ignore */
            break;
        }
        break;
    case HFP_HOLD_REMAINING:
        switch (link->call_state)
        {
        case HFP_CALL_STATE_ACTIVE:
        case HFP_CALL_STATE_TWC_OUTGOING:
        case HFP_CALL_STATE_HELD_ACTIVE:
        case HFP_CALL_STATE_MULTIPARTY:
            hfp_call_status_update(link, HFP_CALL_STATE_HELD_REMAINING);
            break;
        default:
            /* Ignore */
            break;
        }
        break;
    default:
        /* Error */
        break;
    }
}


/****************************************************************************
NAME
    hfp_call_current_indication_hdl

DESCRIPTION
    Handle a current calls notification on link (+CLCC) - this tells us if
    we have a multiparty call or not.

RETURNS
    void
*/
void hfp_call_current_indication_hdl(hfp_link_data *link, uint8_t multiparty)
{
    if (multiparty == 1 && link->call_state != HFP_CALL_STATE_MULTIPARTY)
    {
        hfp_call_status_update(link, HFP_CALL_STATE_MULTIPARTY);
    }
}


/****************************************************************************
NAME
    hfp_call_response_hold_indication_hdl

DESCRIPTION
    Handle a response and hold notification on link (+BTRH)

RETURNS
    void
*/
void hfp_call_response_hold_indication_hdl(hfp_link_data *link, uint8_t state)
{
    switch (state)
    {
    case 0:
        if (link->call_state == HFP_CALL_STATE_INCOMING)
        {
            hfp_call_status_update(link, HFP_CALL_STATE_INCOMING_HELD);
        }
        break;
    case 1:
        if (link->call_state == HFP_CALL_STATE_INCOMING_HELD)
        {
            hfp_call_status_update(link, HFP_CALL_STATE_ACTIVE);
        }
        break;
    case 2:
        if (link->call_state == HFP_CALL_STATE_INCOMING_HELD)
        {
            hfp_call_status_update(link, HFP_CALL_STATE_IDLE);
        }
        break;
    default:
        /* Ignore */
        break;
    }
}
#endif