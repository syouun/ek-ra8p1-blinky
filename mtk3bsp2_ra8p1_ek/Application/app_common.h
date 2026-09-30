/*
 * app_common.h - Objects and helpers shared by the application tasks
 *
 * Copyright (c) 2026 Makoto Tanaka (syouun)
 * SPDX-License-Identifier: MIT
 */
#ifndef APP_COMMON_H
#define APP_COMMON_H

#include <tk/tkernel.h>
#include <stdbool.h>
#include "app_hooks.h"
#include "fall_judge.h"

/* Message AI task -> judge task (through the message buffer g_app_mbf) */
typedef struct
{
    UW                      seq;             /* inference sequence number */
    UW                      capture_cnt;     /* TimeCounter value when the source frame was captured */
    UW                      infer_done_cnt;  /* TimeCounter value when the inference finished */
    UH                      num_det;         /* number of detections in this frame */
    st_ai_detection_point_t best;            /* largest detection (all zero if none) */
} app_det_msg_t;

/* Application status shared with the UI / alert / monitor tasks (copy with app_status_get()) */
typedef struct
{
    fj_state_t              state;
    bool                    person_present;
    st_ai_detection_point_t box;             /* last detected box (model input coordinates) */
    UW                      fall_count;
    UW                      fall_capture_cnt; /* capture time of the frame that confirmed the last fall */
    UW                      last_latency_ms;  /* frame capture -> alert LED of the last fall */
    UW                      inference_count;
    UW                      frame_count;
} app_status_t;

/* Kernel objects created in usermain() */
extern ID g_app_flg;      /* application event flag (APP_EVT_*) */
extern ID g_app_mbf;      /* message buffer: AI -> judge */
extern ID g_app_log_mtx;  /* serialises console output */

/* Task entry functions */
void app_alert_task(INT stacd, void *exinf);
void app_sysmon_task(INT stacd, void *exinf);
void app_judge_task(INT stacd, void *exinf);
void app_ui_task(INT stacd, void *exinf);

/* Helpers */
void app_log(const char *fmt, ...);              /* thread-safe printf to the T-Monitor console */
UW   app_now_ms(void);                            /* μT-Kernel system time [ms] (lower 32 bit) */
void app_status_get(app_status_t *out);
void app_status_set_judge(fj_state_t state, bool person, const st_ai_detection_point_t *box);
void app_status_fall_confirmed(UW capture_cnt);
void app_status_set_latency(UW latency_ms);
UW   app_heartbeat_get(app_task_idx_t task);
const char *app_task_name(app_task_idx_t task);

#endif /* APP_COMMON_H */
