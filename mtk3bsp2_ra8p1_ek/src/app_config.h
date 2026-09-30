/*
 * app_config.h - Configuration of the fall-detection application (6 μT-Kernel tasks)
 *
 * Copyright (c) 2026 Makoto Tanaka (syouun)
 * SPDX-License-Identifier: MIT
 *
 * Placed in src/ so that both Application/ and the ported Renesas sources in src/ can include it.
 */
#ifndef APP_CONFIG_H
#define APP_CONFIG_H

/* 1: privacy UI (status + abstract box only, the camera image is never shown)
 * 0: original Renesas demo screen (camera image + face boxes), for debugging only */
#define APP_PRIVACY_UI_ENABLE        (1)

/* 1: run the fall judgement / alert pipeline */
#define APP_FALL_DETECTION_ENABLE    (1)

/* ---- Task priorities (μT-Kernel: smaller number = higher priority) ---- */
#define APP_PRI_ALERT                (5)
#define APP_PRI_SYSMON               (8)
#define APP_PRI_CAMERA               (10)
#define APP_PRI_JUDGE                (11)
#define APP_PRI_AI                   (12)
#define APP_PRI_UI                   (14)

/* ---- Task stack sizes [byte] ---- */
#define APP_STK_ALERT                (2048)
#define APP_STK_SYSMON               (2048)
#define APP_STK_CAMERA               (8192)
#define APP_STK_JUDGE                (4096)   /* fall_judge_t holds the position history */
#define APP_STK_AI                   (32768)
#define APP_STK_UI                   (4096)

/* ---- Application event flag (g_app_flg) ---- */
#define APP_EVT_FALL_CONFIRMED       (1U << 0)
#define APP_EVT_FALL_RECOVERED       (1U << 1)

/* ---- Periods ---- */
#define APP_UI_PERIOD_MS             (200)    /* 5 screen updates per second is enough for a status screen */
#define APP_SYSMON_CYCLE_MS          (1000)   /* cyclic handler wakes the monitor every second */
#define APP_SYSMON_REPORT_SEC        (10)     /* status line on the serial console */
#define APP_SYSMON_STALL_SEC         (5)      /* heartbeat unchanged this long -> warning */

/* ---- Message buffer AI task -> judge task ---- */
#define APP_MBF_MSG_NUM              (8)

/* ---- Fall judgement (see Application/fall_judge.h, values in permille of the frame height) ---- */
#define APP_FJ_DROP_WINDOW_MS        (1000)
#define APP_FJ_DROP_PERMILLE         (200)
#define APP_FJ_LOW_LINE_PERMILLE     (600)
#define APP_FJ_CONFIRM_MS            (3000)
#define APP_FJ_RECOVER_MS            (1000)
#define APP_FJ_USE_ASPECT            (0)      /* 0: face-detection model (YOLO-fastest), 1: person-detection model */

#endif /* APP_CONFIG_H */
