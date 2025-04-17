/**
  ******************************************************************************
  * @file   rw_rtt.c
  * @author Sifli software development team
  * @brief Implement RWIP rtthread porting functions.
 *
  ******************************************************************************
*/
/*
 * @attention
 * Copyright (c) 2019 - 2022,  Sifli Technology
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Sifli integrated circuit
 *    in a product or a software update for such product, must reproduce the above
 *    copyright notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of Sifli nor the names of its contributors may be used to endorse
 *    or promote products derived from this software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Sifli integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY SIFLI TECHNOLOGY "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL SIFLI TECHNOLOGY OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
*/
#include "rtthread.h"
#include "bf0_ble_common.h"
#include "os_adaptor.h"

#ifndef RT_CONSOLEBUF_SIZE
    #define RT_CONSOLEBUF_SIZE 128
#endif

void rw_kprintf(const char *fmt, ...)
{
    va_list args;
    static char rt_log_buf[RT_CONSOLEBUF_SIZE];
    va_start(args, fmt);
    rt_vsnprintf(rt_log_buf, sizeof(rt_log_buf) - 1, fmt, args);
    rt_kputs(rt_log_buf);
    va_end(args);
}

void rw_assert_handler(const char *ex, const char *func, size_t line)
{
    rt_assert_handler(ex, func, line);
}

uint32_t rw_tick_get_millisecond(void)
{
    return rt_tick_get_millisecond();
}

uint32_t rw_tick_from_millisecond(int32_t ms)
{
    return (uint32_t)rt_tick_from_millisecond(ms);
}

#if !EMB_PRESENT && BF0_HCPU
static struct rt_timer rw_rttimer;

void rwip_timer_co_set(uint32_t target_bts)
{
    rt_tick_t rwip_ticks = rt_tick_from_millisecond(target_bts / 1000);
    extern void timeout_isr(void *param);

    if (rw_rttimer.timeout_func == NULL)
    {
        rt_timer_init(&rw_rttimer, "BLEHost", timeout_isr, NULL, rwip_ticks, RT_TIMER_FLAG_ONE_SHOT);
        rt_timer_start(&rw_rttimer);
    }
    else
    {
        rt_timer_stop(&rw_rttimer);
        rt_timer_control(&rw_rttimer, RT_TIMER_CTRL_SET_TIME, (void *)&rwip_ticks);
        rt_timer_start(&rw_rttimer);
    }
}
#endif

RT_WEAK void rw_os_delay_ms(int ms)
{
    rt_thread_delay(rt_tick_from_millisecond(ms));
}

/************************ (C) COPYRIGHT Sifli Technology *******END OF FILE****/
