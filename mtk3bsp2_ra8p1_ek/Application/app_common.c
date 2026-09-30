/*
 * app_common.c - Objects and helpers shared by the application tasks, and the hooks called
 *                from the ported Renesas sample threads (camera / AI inference)
 *
 * Copyright (c) 2026 Makoto Tanaka (syouun)
 * SPDX-License-Identifier: MIT
 */
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <tm/tmonitor.h>
#include "app_common.h"
#include "time_counter.h"

ID g_app_flg     = 0;
ID g_app_mbf     = 0;
ID g_app_log_mtx = 0;

static volatile UW  s_heartbeat[APP_TASK_NUM];
static app_status_t s_status;
static volatile UW  s_last_input_capture_cnt;
static UW           s_inference_seq;
static char         s_log_buf[160];

/* ------------------------------------------------------------------------------------------------ */
void app_log(const char *fmt, ...)
{
    va_list ap;
    BOOL    locked = (g_app_log_mtx > 0) && (tk_loc_mtx(g_app_log_mtx, TMO_FEVR) == E_OK);

    va_start(ap, fmt);
    vsnprintf(s_log_buf, sizeof(s_log_buf), fmt, ap);
    va_end(ap);
    tm_putstring((UB *) s_log_buf);

    if (locked)
    {
        tk_unl_mtx(g_app_log_mtx);
    }
}

UW app_now_ms(void)
{
    SYSTIM t;
    tk_get_otm(&t);
    return t.lo;
}

/* ------------------------------------------------------------------------------------------------ */
/* Status: written by the judge / alert tasks, read by the UI / monitor tasks.
 * Copies are made with dispatching disabled so a reader never sees a half-updated structure. */
void app_status_get(app_status_t *out)
{
    tk_dis_dsp();
    *out = s_status;
    tk_ena_dsp();
}

void app_status_set_judge(fj_state_t state, bool person, const st_ai_detection_point_t *box)
{
    tk_dis_dsp();
    s_status.state          = state;
    s_status.person_present = person;
    if (NULL != box)
    {
        s_status.box = *box;
    }
    tk_ena_dsp();
}

void app_status_fall_confirmed(UW capture_cnt)
{
    tk_dis_dsp();
    s_status.fall_count++;
    s_status.fall_capture_cnt = capture_cnt;
    tk_ena_dsp();
}

void app_status_set_latency(UW latency_ms)
{
    tk_dis_dsp();
    s_status.last_latency_ms = latency_ms;
    tk_ena_dsp();
}

/* ------------------------------------------------------------------------------------------------ */
void app_heartbeat(app_task_idx_t task)
{
    if (task < APP_TASK_NUM)
    {
        s_heartbeat[task]++;
    }
}

UW app_heartbeat_get(app_task_idx_t task)
{
    return (task < APP_TASK_NUM) ? s_heartbeat[task] : 0;
}

const char *app_task_name(app_task_idx_t task)
{
    static const char * const names[APP_TASK_NUM] = { "camera", "ai", "judge", "alert", "ui" };
    return (task < APP_TASK_NUM) ? names[task] : "?";
}

/* ------------------------------------------------------------------------------------------------ */
/* Hook from camera_display_thread_entry(): called just before AI_INFERENCE_INPUT_IMAGE_READY is set. */
void app_on_ai_input_ready(uint32_t capture_cnt)
{
    s_last_input_capture_cnt = capture_cnt;
    s_status.frame_count++;      /* only the camera task writes this counter */
    app_heartbeat(APP_TASK_CAMERA);
}

/* Hook from ai_inference_thread_entry(): called after every inference. Never blocks the AI task. */
void app_post_inference_result(const st_ai_detection_point_t *det, int num)
{
    app_det_msg_t msg;
    int32_t       best_area = 0;

    memset(&msg, 0, sizeof(msg));
    msg.seq            = ++s_inference_seq;
    msg.capture_cnt    = s_last_input_capture_cnt;
    msg.infer_done_cnt = TimeCounter_CurrentCountGet();

    for (int i = 0; i < num; i++)
    {
        int32_t area = (int32_t) det[i].m_w * (int32_t) det[i].m_h;
        if ((det[i].m_w > 0) && (det[i].m_h > 0))
        {
            msg.num_det++;
            if (area > best_area)
            {
                best_area = area;
                msg.best  = det[i];
            }
        }
    }

    s_status.inference_count++;  /* only the AI task writes this counter */
    app_heartbeat(APP_TASK_AI);

    if (g_app_mbf > 0)
    {
        /* TMO_POL: if the judge task is behind, drop this result instead of delaying the AI pipeline. */
        (void) tk_snd_mbf(g_app_mbf, &msg, (INT) sizeof(msg), TMO_POL);
    }
}
