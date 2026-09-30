/*
 * app_alert_task.c - Alert task (highest application priority, APP_PRI_ALERT)
 *
 * Sleeps on the application event flag. When the judge task confirms a fall it
 * turns the red LED on first (before anything else), then reports on the console,
 * including the time from the capture of the confirming frame to the LED.
 *
 * LED usage: LED1 blue = AI inference running (Renesas sample), LED2 green = monitoring,
 *            LED3 red = fall detected.
 *
 * Copyright (c) 2026 Makoto Tanaka (syouun)
 * SPDX-License-Identifier: MIT
 */
#include "app_common.h"
#include "common_util.h"
#include "time_counter.h"

void app_alert_task(INT stacd, void *exinf)
{
    (void) stacd;
    (void) exinf;

    LED3_OFF;
    LED2_ON;   /* green: monitoring */

    while (1)
    {
        UINT ptn = 0;

        if (tk_wai_flg(g_app_flg, APP_EVT_FALL_CONFIRMED | APP_EVT_FALL_RECOVERED, TWF_ORW | TWF_BITCLR, &ptn, TMO_FEVR) != E_OK)
        {
            continue;
        }
        app_heartbeat(APP_TASK_ALERT);

        if (ptn & APP_EVT_FALL_CONFIRMED)
        {
            /* 1. Notify first */
            LED3_ON;
            LED2_OFF;

            /* 2. Measure: capture of the confirming frame -> LED on */
            UW now_cnt = TimeCounter_CurrentCountGet();
            app_status_t st;
            app_status_get(&st);
            UW latency_ms = TimeCounter_CountValueConvertToMs(st.fall_capture_cnt, now_cnt);
            app_status_set_latency(latency_ms);

            /* 3. Report */
            app_log("\r\n[ALERT] ***** FALL DETECTED ***** (#%u, frame capture -> LED %u ms, confirm window %u ms)\r\n",
                    (unsigned) st.fall_count, (unsigned) latency_ms, (unsigned) APP_FJ_CONFIRM_MS);
        }

        if (ptn & APP_EVT_FALL_RECOVERED)
        {
            LED3_OFF;
            LED2_ON;
            app_log("[ALERT] recovered: the person is up again\r\n");
        }
    }
}
