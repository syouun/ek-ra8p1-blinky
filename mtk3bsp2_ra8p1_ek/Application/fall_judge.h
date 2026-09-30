/*
 * fall_judge.h - Fall judgement from object-detection bounding boxes (OS independent)
 *
 * Copyright (c) 2026 Makoto Tanaka (syouun)
 * SPDX-License-Identifier: MIT
 *
 * The judge receives, for every AI inference, the largest detected bounding box
 * (or "no detection") and a timestamp in milliseconds, and runs a small state
 * machine:
 *
 *   NORMAL --(sudden drop into the low area / lying shape)--> SUSPICIOUS
 *   SUSPICIOUS --(back to upper area)--> NORMAL
 *   SUSPICIOUS --(stays for confirm_ms)--> FALLEN      => FJ_EVT_FALL_CONFIRMED
 *   FALLEN --(back to upper area for recover_ms)--> NORMAL => FJ_EVT_RECOVERED
 *
 * All positions are handled in permille (1/1000) of the frame height so that the
 * same parameters work for any model input size. No floating point is used.
 */
#ifndef FALL_JUDGE_H
#define FALL_JUDGE_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FJ_HISTORY_LEN  (64)   /* >= inference rate [Hz] x drop_window [s] */

typedef enum
{
    FJ_STATE_NORMAL = 0,
    FJ_STATE_SUSPICIOUS,
    FJ_STATE_FALLEN,
} fj_state_t;

typedef enum
{
    FJ_EVT_NONE = 0,
    FJ_EVT_SUSPECT,            /* NORMAL -> SUSPICIOUS */
    FJ_EVT_SUSPECT_CANCELLED,  /* SUSPICIOUS -> NORMAL */
    FJ_EVT_FALL_CONFIRMED,     /* SUSPICIOUS -> FALLEN */
    FJ_EVT_RECOVERED,          /* FALLEN -> NORMAL */
} fj_event_t;

typedef struct
{
    bool    valid;             /* false: nothing detected in this frame */
    int32_t x, y, w, h;        /* top-left corner and size [pixel of the model input] */
} fj_box_t;

typedef struct
{
    int32_t  frame_h;                   /* model input image height [pixel] */
    uint32_t drop_window_ms;            /* window to look for a sudden drop */
    int32_t  drop_permille;             /* required drop of the box centre within the window */
    int32_t  low_line_permille;         /* box centre below this line = "low" (0 = top, 1000 = bottom) */
    int32_t  recover_margin_permille;   /* hysteresis: must rise above (low_line - margin) to recover */
    uint32_t confirm_ms;                /* SUSPICIOUS must last this long to confirm a fall */
    uint32_t recover_ms;                /* upper position must last this long to leave FALLEN */
    uint32_t presence_hold_ms;          /* "person present" is kept this long after the last detection */
    bool     use_aspect;                /* true for person-detection models (box shape h/w is meaningful) */
    int32_t  aspect_fallen_permille;    /* h/w below this = lying shape */
    int32_t  aspect_standing_permille;  /* h/w above this = standing shape */
} fj_params_t;

typedef struct
{
    uint32_t t_ms;
    int32_t  cy_permille;
} fj_sample_t;

typedef struct
{
    fj_params_t prm;
    fj_state_t  state;
    uint32_t    t_state_ms;       /* time the current state was entered */
    uint32_t    t_recover_ms;     /* start of the recovering posture in FALLEN (valid if recovering) */
    bool        recovering;
    bool        seen_once;
    uint32_t    t_last_seen_ms;
    int32_t     last_cy_permille;
    int32_t     last_aspect_permille;
    fj_sample_t hist[FJ_HISTORY_LEN];
    uint32_t    hist_count;
    uint32_t    hist_head;
} fall_judge_t;

/* Fill *p with the default parameters for a model input of frame_h pixels. */
void        fall_judge_default_params(fj_params_t *p, int32_t frame_h);
void        fall_judge_init(fall_judge_t *fj, const fj_params_t *p);
fj_event_t  fall_judge_update(fall_judge_t *fj, uint32_t now_ms, const fj_box_t *box);
bool        fall_judge_person_present(const fall_judge_t *fj, uint32_t now_ms);
const char *fall_judge_state_name(fj_state_t s);

#ifdef __cplusplus
}
#endif

#endif /* FALL_JUDGE_H */
