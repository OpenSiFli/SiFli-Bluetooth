#ifndef _HFP_CALL_SERVICE_H_
#define _HFP_CALL_SERVICE_H_

#ifdef CFG_HFP_CALL_STATUS
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*hf_call_notify_func_t)(uint16_t profile_channel, uint8_t callstate);
int hfp_call_status_register_callback(hf_call_notify_func_t cb);
int hfp_call_status_unregister_callback(void);

#ifdef __cplusplus
}
#endif
#endif
#endif