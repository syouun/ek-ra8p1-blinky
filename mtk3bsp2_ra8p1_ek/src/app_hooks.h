/*
 * app_hooks.h - Interface between the ported Renesas sample threads (src/) and the application tasks (Application/)
 *
 * Copyright (c) 2026 Makoto Tanaka (syouun)
 * SPDX-License-Identifier: MIT
 */
#ifndef APP_HOOKS_H
#define APP_HOOKS_H

#include <stdint.h>
#include "app_config.h"
#include "common_util.h"   /* st_ai_detection_point_t */

FSP_CPP_HEADER

typedef enum
{
    APP_TASK_CAMERA = 0,
    APP_TASK_AI,
    APP_TASK_JUDGE,
    APP_TASK_ALERT,
    APP_TASK_UI,
    APP_TASK_NUM
} app_task_idx_t;

/* Liveness counter of each task, read by the system monitor task. */
void app_heartbeat(app_task_idx_t task);

/* Camera task: the AI input image made from the frame captured at capture_cnt (TimeCounter, 0.1 ms) is ready. */
void app_on_ai_input_ready(uint32_t capture_cnt);

/* AI task: one inference finished. Sends the largest detection to the judge task (never blocks). */
void app_post_inference_result(const st_ai_detection_point_t *det, int num);

FSP_CPP_FOOTER

#endif /* APP_HOOKS_H */
