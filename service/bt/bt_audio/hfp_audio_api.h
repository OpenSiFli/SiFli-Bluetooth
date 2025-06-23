/*
 * SPDX-FileCopyrightText: 2019-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _HFP_AUDIO_API_H_
#define _HFP_AUDIO_API_H_

#ifdef AUDIO_USING_MANAGER

#include "audio_server.h"

typedef audio_type_t  hfp_audio_type_t;

typedef struct
{
    audio_client_t                 handle;
    audio_param_bt_voice_t         voice_para;
    hfp_audio_type_t               type_process;
    uint8_t                        audioStop;          //1:on stop process
} hfp_audio_env_t;

//patch
#define AUDIO_BT_VOICE_MSBC_IN_LEN     60
#define CVSD_MODE                       2

void hfp_audio_init(void);
void hfp_audio_set_default_para(void);
void hfp_set_audio_voice_para(BTS2S_HF_AUDIO_INFO *msg, BOOL audio_on, U8 direct_audio_on);
void hfp_aduio_open_path(hfp_audio_type_t audioType);
void hfp_audio_close_path(void);

/*for hfp ag & audio box*/
#define  AUDIO_BOX_EN   1
void hfp_ag_audio_opt(BTS2S_HF_AUDIO_INFO *msg, BOOL audio_on);
void hfp_ag_audio_register(void);
#endif //AUDIO_USING_MANAGER



#endif


