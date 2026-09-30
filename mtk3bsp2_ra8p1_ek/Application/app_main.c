/*
 * app_main.c - usermain(): creates the kernel objects and the 6 application tasks
 *
 *   prio  task      role
 *   ----  --------  -------------------------------------------------------------
 *    5    alert     fall confirmed -> red LED, console report (event flag wait)
 *    8    sysmon    woken by a cyclic handler every second, watches heartbeats
 *   10    camera    camera capture + AI input pre-processing (ported Renesas thread)
 *   11    judge     fall judgement state machine (message buffer receive)
 *   12    ai        Ethos-U55 inference (ported Renesas thread, message buffer send)
 *   14    ui        privacy screen: state + abstract box only, no camera image
 *
 * Copyright (c) 2026 Makoto Tanaka (syouun)
 * SPDX-License-Identifier: MIT (the μT-Kernel sample this file started from is T-License 2.2)
 */
#include <tk/tkernel.h>
#include <tm/tmonitor.h>
#include "app_common.h"

/* Event flag of the ported Renesas sample (FreeRTOS EventGroup replacement, see src/rtos_to_mtk.h) */
ID g_ai_app_event;

extern void camera_display_thread_entry(void *pvParameters);
extern void ai_inference_thread_entry(void *pvParameters);

LOCAL void task_camera(INT stacd, void *exinf)
{
    (void) stacd;
    (void) exinf;
    camera_display_thread_entry(NULL);
    tk_ext_tsk();
}

LOCAL void task_ai(INT stacd, void *exinf)
{
    (void) stacd;
    (void) exinf;
    ai_inference_thread_entry(NULL);
    tk_ext_tsk();
}

typedef struct
{
    const char *name;
    FP          entry;
    PRI         pri;
    SZ          stksz;
} app_task_def_t;

LOCAL const app_task_def_t s_tasks[] = {
    { "alert",  (FP) app_alert_task,  APP_PRI_ALERT,  APP_STK_ALERT  },
    { "sysmon", (FP) app_sysmon_task, APP_PRI_SYSMON, APP_STK_SYSMON },
#if (APP_FALL_DETECTION_ENABLE == 1)
    { "judge",  (FP) app_judge_task,  APP_PRI_JUDGE,  APP_STK_JUDGE  },
#endif
#if (APP_PRIVACY_UI_ENABLE == 1)
    { "ui",     (FP) app_ui_task,     APP_PRI_UI,     APP_STK_UI     },
#endif
    { "ai",     (FP) task_ai,         APP_PRI_AI,     APP_STK_AI     },
    { "camera", (FP) task_camera,     APP_PRI_CAMERA, APP_STK_CAMERA },
};

LOCAL BOOL check(const char *what, INT er)
{
    if (er < E_OK)
    {
        tm_printf((UB *) "[MAIN] %s failed: %d\n", what, er);
        return FALSE;
    }
    return TRUE;
}

/* usermain: called by μT-Kernel after the kernel has started */
EXPORT INT usermain(void)
{
    tm_putstring((UB *) "\n=== Privacy-first fall detection edge AI (uT-Kernel 3.0 / EK-RA8P1) ===\n");

    /* Board LEDs off. LED1(BLUE):P600 LED2(GREEN):P303 LED3(RED):PA07 */
    LED1_OFF;
    LED2_OFF;
    LED3_OFF;

    /* ---- Kernel objects ---- */
    T_CFLG cflg = { .exinf = NULL, .flgatr = TA_TFIFO | TA_WMUL, .iflgptn = 0 };
    g_ai_app_event = tk_cre_flg(&cflg);          /* sample: HW init / capture / inference sync */
    check("tk_cre_flg(ai_app_event)", g_ai_app_event);

    g_app_flg = tk_cre_flg(&cflg);               /* application: fall confirmed / recovered */
    check("tk_cre_flg(app)", g_app_flg);

    T_CMBF cmbf = {
        .exinf  = NULL,
        .mbfatr = TA_TFIFO,
        .bufsz  = (SZ) ((sizeof(app_det_msg_t) + 8U) * APP_MBF_MSG_NUM),  /* + per-message header */
        .maxmsz = (INT) sizeof(app_det_msg_t),
    };
    g_app_mbf = tk_cre_mbf(&cmbf);
    check("tk_cre_mbf", g_app_mbf);

    T_CMTX cmtx = { .exinf = NULL, .mtxatr = TA_INHERIT, .ceilpri = 0 };
    g_app_log_mtx = tk_cre_mtx(&cmtx);
    check("tk_cre_mtx", g_app_log_mtx);

    /* ---- Tasks ---- */
    for (UINT i = 0; i < sizeof(s_tasks) / sizeof(s_tasks[0]); i++)
    {
        T_CTSK ctsk = {
            .exinf   = NULL,
            .tskatr  = TA_HLNG | TA_RNG3,
            .task    = s_tasks[i].entry,
            .itskpri = s_tasks[i].pri,
            .stksz   = s_tasks[i].stksz,
        };
        ID tskid = tk_cre_tsk(&ctsk);
        if (check(s_tasks[i].name, tskid))
        {
            tk_sta_tsk(tskid, 0);
            tm_printf((UB *) "[MAIN] task %s started (id %d, priority %d)\n", s_tasks[i].name, tskid, s_tasks[i].pri);
        }
    }

    tk_slp_tsk(TMO_FEVR);
    return 0;
}
