/*
 * app_judge_task.c - Fall judgement task (priority APP_PRI_JUDGE)
 *
 * Receives the largest detection of every inference from the AI task through a
 * message buffer, runs the fall_judge state machine and wakes the alert task
 * through the application event flag when a fall is confirmed / recovered.
 *
 * Copyright (c) 2026 Makoto Tanaka (syouun)
 * SPDX-License-Identifier: MIT
 */
#include "app_common.h"
#include "ai_application_config.h"

static fall_judge_t s_fj;   /* static: keeps the position history off the task stack */

void app_judge_task(INT stacd, void *exinf)
{
    app_det_msg_t msg;
    fj_params_t   prm;

    (void) stacd;
    (void) exinf;

    fall_judge_default_params(&prm, AI_INPUT_IMAGE_HEIGHT);
    prm.drop_window_ms    = APP_FJ_DROP_WINDOW_MS;
    prm.drop_permille     = APP_FJ_DROP_PERMILLE;
    prm.low_line_permille = APP_FJ_LOW_LINE_PERMILLE;
    prm.confirm_ms        = APP_FJ_CONFIRM_MS;
    prm.recover_ms        = APP_FJ_RECOVER_MS;
    prm.use_aspect        = (APP_FJ_USE_ASPECT != 0);
    fall_judge_init(&s_fj, &prm);

    app_log("[JUDGE] started (drop %d/1000 in %d ms, low line %d/1000, confirm %d ms)\r\n",
            APP_FJ_DROP_PERMILLE, APP_FJ_DROP_WINDOW_MS, APP_FJ_LOW_LINE_PERMILLE, APP_FJ_CONFIRM_MS);

    while (1)
    {
        INT sz = tk_rcv_mbf(g_app_mbf, &msg, TMO_FEVR);
        if (sz != (INT) sizeof(msg))
        {
            continue;
        }
        app_heartbeat(APP_TASK_JUDGE);

        fj_box_t box;
        box.valid = (msg.num_det > 0);
        box.x     = msg.best.m_x;
        box.y     = msg.best.m_y;
        box.w     = msg.best.m_w;
        box.h     = msg.best.m_h;

        UW         now = app_now_ms();
        fj_event_t evt = fall_judge_update(&s_fj, now, &box);

        app_status_set_judge(s_fj.state, fall_judge_person_present(&s_fj, now), box.valid ? &msg.best : NULL);

        switch (evt)
        {
            case FJ_EVT_SUSPECT:
                app_log("[JUDGE] suspect: sudden drop (centre %d/1000)\r\n", (int) s_fj.last_cy_permille);
                break;
            case FJ_EVT_SUSPECT_CANCELLED:
                app_log("[JUDGE] suspect cancelled: back to upper position\r\n");
                break;
            case FJ_EVT_FALL_CONFIRMED:
                app_status_fall_confirmed(msg.capture_cnt);
                tk_set_flg(g_app_flg, APP_EVT_FALL_CONFIRMED);   /* wakes the alert task (highest priority) */
                break;
            case FJ_EVT_RECOVERED:
                tk_set_flg(g_app_flg, APP_EVT_FALL_RECOVERED);
                break;
            default:
                break;
        }
    }
}
