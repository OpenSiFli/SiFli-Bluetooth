/*
 * SPDX-FileCopyrightText: 2019-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
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


