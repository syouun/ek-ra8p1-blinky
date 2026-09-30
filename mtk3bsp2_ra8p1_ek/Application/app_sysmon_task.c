/*
 * app_sysmon_task.c - System monitor task (APP_PRI_SYSMON) and its cyclic handler
 *
 * A μT-Kernel cyclic handler wakes this task every APP_SYSMON_CYCLE_MS. The task checks
 * that the heartbeat counters of the camera / AI / judge / UI tasks keep increasing and
 * prints a warning if one of them stalls, and prints a status line periodically.
 *
 * Copyright (c) 2026 Makoto Tanaka (syouun)
 * SPDX-License-Identifier: MIT
 */
#include "app_common.h"

static ID s_sysmon_tskid;

/* Cyclic handler (task-independent part): only wakes the monitor task. */
static void sysmon_cyc_handler(void *exinf)
{
    (void) exinf;
    tk_wup_tsk(s_sysmon_tskid);
}

void app_sysmon_task(INT stacd, void *exinf)
{
    /* Tasks whose heartbeat must keep moving. The judge task depends on the AI task, and the
     * alert task only runs on events, so they are reported but not treated as stalled. */
    static const app_task_idx_t watched[] = { APP_TASK_CAMERA, APP_TASK_AI, APP_TASK_UI };
    UW   last[APP_TASK_NUM]  = {0};
    UW   still[APP_TASK_NUM] = {0};
    UW   uptime_s = 0;

    (void) stacd;
    (void) exinf;

    s_sysmon_tskid = tk_get_tid();

    T_CCYC ccyc = {
        .exinf  = NULL,
        .cycatr = TA_HLNG | TA_STA,
        .cychdr = (FP) sysmon_cyc_handler,
        .cyctim = APP_SYSMON_CYCLE_MS,
        .cycphs = APP_SYSMON_CYCLE_MS,
    };
    ID cycid = tk_cre_cyc(&ccyc);
    if (cycid < E_OK)
    {
        app_log("[SYSMON] tk_cre_cyc failed (%d)\r\n", (int) cycid);
    }

    while (1)
    {
        if (cycid >= E_OK)
        {
            tk_slp_tsk(TMO_FEVR);                 /* woken by the cyclic handler */
        }
        else
        {
            tk_dly_tsk(APP_SYSMON_CYCLE_MS);      /* fallback */
        }
        uptime_s++;

        for (unsigned i = 0; i < sizeof(watched) / sizeof(watched[0]); i++)
        {
            app_task_idx_t t  = watched[i];
            UW             hb = app_heartbeat_get(t);
            if (hb == last[t])
            {
                still[t]++;
                if (still[t] == APP_SYSMON_STALL_SEC)
                {
                    app_log("[SYSMON] WARNING: %s task has not run for %u s\r\n", app_task_name(t), (unsigned) still[t]);
                }
            }
            else
            {
                if (still[t] >= APP_SYSMON_STALL_SEC)
                {
                    app_log("[SYSMON] %s task resumed\r\n", app_task_name(t));
                }
                still[t] = 0;
            }
            last[t] = hb;
        }

        if ((uptime_s % APP_SYSMON_REPORT_SEC) == 0)
        {
            app_status_t st;
            app_status_get(&st);
            app_log("[SYSMON] up %us frames %u inferences %u judged %u | state %s person %s falls %u last latency %u ms\r\n",
                    (unsigned) uptime_s, (unsigned) st.frame_count, (unsigned) st.inference_count,
                    (unsigned) app_heartbeat_get(APP_TASK_JUDGE), fall_judge_state_name(st.state),
                    st.person_present ? "yes" : "no", (unsigned) st.fall_count, (unsigned) st.last_latency_ms);
        }
    }
}
