/*
 * fall_judge.c - Fall judgement from object-detection bounding boxes (OS independent)
 *
 * Copyright (c) 2026 Makoto Tanaka (syouun)
 * SPDX-License-Identifier: MIT
 */
#include "fall_judge.h"
#include <string.h>

void fall_judge_default_params(fj_params_t *p, int32_t frame_h)
{
    p->frame_h                  = frame_h;
    p->drop_window_ms           = 1000;  /* a fall happens within about 1 s            */
    p->drop_permille            = 200;   /* centre drops by >= 20 % of the frame height */
    p->low_line_permille        = 600;   /* and ends up in the lower 40 % of the frame  */
    p->recover_margin_permille  = 50;
    p->confirm_ms               = 3000;  /* stays down for 3 s -> fall confirmed        */
    p->recover_ms               = 1000;
    p->presence_hold_ms         = 1000;
    p->use_aspect               = false; /* face-detection model: box shape is not used */
    p->aspect_fallen_permille   = 800;   /* h/w < 0.8 : lying                            */
    p->aspect_standing_permille = 1200;  /* h/w > 1.2 : standing                         */
}

void fall_judge_init(fall_judge_t *fj, const fj_params_t *p)
{
    memset(fj, 0, sizeof(*fj));
    fj->prm   = *p;
    fj->state = FJ_STATE_NORMAL;
}

static void hist_clear(fall_judge_t *fj)
{
    fj->hist_count = 0;
    fj->hist_head  = 0;
}

static void hist_push(fall_judge_t *fj, uint32_t t_ms, int32_t cy)
{
    fj->hist[fj->hist_head].t_ms        = t_ms;
    fj->hist[fj->hist_head].cy_permille = cy;
    fj->hist_head = (fj->hist_head + 1U) % FJ_HISTORY_LEN;
    if (fj->hist_count < FJ_HISTORY_LEN)
    {
        fj->hist_count++;
    }
}

/* Highest position (= smallest centre y) seen within the last window_ms. */
static int32_t hist_min_cy(const fall_judge_t *fj, uint32_t now_ms, uint32_t window_ms, int32_t fallback)
{
    int32_t  min_cy = fallback;
    uint32_t idx    = fj->hist_head;

    for (uint32_t i = 0; i < fj->hist_count; i++)
    {
        idx = (idx + FJ_HISTORY_LEN - 1U) % FJ_HISTORY_LEN;
        if ((uint32_t)(now_ms - fj->hist[idx].t_ms) > window_ms)
        {
            break;  /* older samples are outside the window */
        }
        if (fj->hist[idx].cy_permille < min_cy)
        {
            min_cy = fj->hist[idx].cy_permille;
        }
    }
    return min_cy;
}

static bool is_upper_posture(const fall_judge_t *fj, int32_t cy, int32_t aspect)
{
    const fj_params_t *p = &fj->prm;
    bool upper = (cy < (p->low_line_permille - p->recover_margin_permille));

    if (p->use_aspect)
    {
        upper = upper && (aspect >= p->aspect_standing_permille);
    }
    return upper;
}

fj_event_t fall_judge_update(fall_judge_t *fj, uint32_t now_ms, const fj_box_t *box)
{
    const fj_params_t *p   = &fj->prm;
    fj_event_t         evt = FJ_EVT_NONE;
    int32_t            cy  = 0;
    int32_t            aspect = 0;
    bool               valid = (NULL != box) && box->valid && (box->w > 0) && (box->h > 0) && (p->frame_h > 0);

    if (valid)
    {
        cy     = ((box->y + (box->h / 2)) * 1000) / p->frame_h;
        aspect = (box->h * 1000) / box->w;
        fj->seen_once            = true;
        fj->t_last_seen_ms       = now_ms;
        fj->last_cy_permille     = cy;
        fj->last_aspect_permille = aspect;
    }

    switch (fj->state)
    {
        case FJ_STATE_NORMAL:
        {
            if (valid)
            {
                /* Compare with the highest position in the recent window (before adding this frame). */
                int32_t min_cy      = hist_min_cy(fj, now_ms, p->drop_window_ms, cy);
                bool    sudden_drop = ((cy - min_cy) >= p->drop_permille) && (cy >= p->low_line_permille);
                bool    lying_shape = p->use_aspect && (aspect < p->aspect_fallen_permille);

                hist_push(fj, now_ms, cy);

                if (sudden_drop || lying_shape)
                {
                    fj->state      = FJ_STATE_SUSPICIOUS;
                    fj->t_state_ms = now_ms;
                    evt            = FJ_EVT_SUSPECT;
                }
            }
            break;
        }

        case FJ_STATE_SUSPICIOUS:
        {
            if (valid && is_upper_posture(fj, cy, aspect))
            {
                /* Got up again quickly (e.g. crouched to pick something up). */
                fj->state      = FJ_STATE_NORMAL;
                fj->t_state_ms = now_ms;
                hist_clear(fj);
                hist_push(fj, now_ms, cy);
                evt = FJ_EVT_SUSPECT_CANCELLED;
            }
            else if ((uint32_t)(now_ms - fj->t_state_ms) >= p->confirm_ms)
            {
                /* Still down (or no longer visible after going down): confirm the fall. */
                fj->state      = FJ_STATE_FALLEN;
                fj->t_state_ms = now_ms;
                fj->recovering = false;
                evt            = FJ_EVT_FALL_CONFIRMED;
            }
            break;
        }

        case FJ_STATE_FALLEN:
        {
            if (valid && is_upper_posture(fj, cy, aspect))
            {
                if (!fj->recovering)
                {
                    fj->recovering   = true;
                    fj->t_recover_ms = now_ms;
                }
                else if ((uint32_t)(now_ms - fj->t_recover_ms) >= p->recover_ms)
                {
                    fj->state      = FJ_STATE_NORMAL;
                    fj->t_state_ms = now_ms;
                    fj->recovering = false;
                    hist_clear(fj);
                    hist_push(fj, now_ms, cy);
                    evt = FJ_EVT_RECOVERED;
                }
            }
            else
            {
                fj->recovering = false;
            }
            break;
        }

        default:
        {
            fall_judge_init(fj, p);
            break;
        }
    }

    return evt;
}

bool fall_judge_person_present(const fall_judge_t *fj, uint32_t now_ms)
{
    return fj->seen_once && ((uint32_t)(now_ms - fj->t_last_seen_ms) <= fj->prm.presence_hold_ms);
}

const char *fall_judge_state_name(fj_state_t s)
{
    switch (s)
    {
        case FJ_STATE_NORMAL:     return "NORMAL";
        case FJ_STATE_SUSPICIOUS: return "SUSPICIOUS";
        case FJ_STATE_FALLEN:     return "FALLEN";
        default:                  return "?";
    }
}
