/*
 * Host unit test for Application/fall_judge.c
 *
 *   cd tests && make        (needs a host C compiler such as gcc or clang)
 *
 * Copyright (c) 2026 Makoto Tanaka (syouun)
 * SPDX-License-Identifier: MIT
 */
#include <stdio.h>
#include <stdlib.h>
#include "../mtk3bsp2_ra8p1_ek/Application/fall_judge.h"

#define FRAME_H    192
#define PERIOD_MS  50      /* 20 inferences per second (measured rate of the sample) */

static int g_fail = 0;

typedef struct
{
    int suspect, cancel, fall, recover;
    uint32_t t_fall;
} counts_t;

static fall_judge_t fj;
static uint32_t     now;
static counts_t     c;

static void reset(bool use_aspect, uint32_t t0)
{
    fj_params_t p;
    fall_judge_default_params(&p, FRAME_H);
    p.use_aspect = use_aspect;
    fall_judge_init(&fj, &p);
    now = t0;
    c = (counts_t){0};
}

/* Feed one frame whose face/person box centre is at cy_permille (or no detection if cy < 0). */
static void frame(int cy_permille, int w, int h)
{
    fj_box_t b = {0};
    if (cy_permille >= 0)
    {
        b.valid = true;
        b.w = w;
        b.h = h;
        b.x = 80;
        b.y = (cy_permille * FRAME_H) / 1000 - h / 2;
    }
    switch (fall_judge_update(&fj, now, &b))
    {
        case FJ_EVT_SUSPECT:           c.suspect++; break;
        case FJ_EVT_SUSPECT_CANCELLED: c.cancel++;  break;
        case FJ_EVT_FALL_CONFIRMED:    c.fall++; c.t_fall = now; break;
        case FJ_EVT_RECOVERED:         c.recover++; break;
        default: break;
    }
    now += PERIOD_MS;
}

/* Move linearly from cy0 to cy1 over ms milliseconds. */
static void move(int cy0, int cy1, uint32_t ms, int w, int h)
{
    uint32_t n = ms / PERIOD_MS;
    for (uint32_t i = 0; i < n; i++)
    {
        frame(cy0 + (int)(((long)(cy1 - cy0) * (long)i) / (long)n), w, h);
    }
}

static void stay(int cy, uint32_t ms, int w, int h) { move(cy, cy, ms, w, h); }

#define EXPECT(name, cond) do { if (cond) { printf("  ok   %s\n", name); } \
    else { printf("  FAIL %s  (suspect=%d cancel=%d fall=%d recover=%d state=%s)\n", name, c.suspect, c.cancel, c.fall, c.recover, \
    fall_judge_state_name(fj.state)); g_fail++; } } while (0)

int main(void)
{
    printf("fall_judge unit test (frame_h=%d, %d ms/frame)\n", FRAME_H, PERIOD_MS);

    reset(false, 0);
    stay(300, 10000, 30, 30);
    EXPECT("standing still: no event", c.suspect == 0 && c.fall == 0);

    reset(false, 0);
    for (int i = 0; i < 10; i++) { move(280, 340, 500, 30, 30); move(340, 280, 500, 30, 30); }
    EXPECT("walking: no event", c.suspect == 0 && c.fall == 0);

    reset(false, 0);
    stay(300, 2000, 30, 30); move(300, 650, 3000, 30, 30); stay(650, 6000, 30, 30);
    EXPECT("sitting down slowly: no fall", c.suspect == 0 && c.fall == 0);

    reset(false, 0);
    stay(300, 2000, 30, 30); move(300, 700, 500, 30, 30); stay(700, 1000, 30, 30); move(700, 300, 500, 30, 30); stay(300, 5000, 30, 30);
    EXPECT("quick crouch and stand up: suspect cancelled, no fall", c.suspect == 1 && c.cancel == 1 && c.fall == 0);

    reset(false, 0);
    stay(300, 2000, 30, 30); move(300, 750, 500, 30, 30); stay(750, 6000, 30, 30);
    EXPECT("fall: confirmed once", c.suspect == 1 && c.fall == 1 && fj.state == FJ_STATE_FALLEN);
    EXPECT("fall: confirmed about 3 s after the drop", c.t_fall >= 2000 + 3000 && c.t_fall <= 2000 + 500 + 3200);

    stay(300, 2000, 30, 30);
    EXPECT("recovery after 1 s in upper position", c.recover == 1 && fj.state == FJ_STATE_NORMAL);

    reset(false, 0);
    stay(300, 2000, 30, 30); move(300, 750, 500, 30, 30); stay(-1, 5000, 0, 0);
    EXPECT("fall and then face not visible: still confirmed", c.fall == 1);
    EXPECT("person_present becomes false when nothing is detected", !fall_judge_person_present(&fj, now));

    reset(false, 0);
    stay(-1, 10000, 0, 0);
    EXPECT("nobody in view: no event", c.suspect == 0 && c.fall == 0 && !fall_judge_person_present(&fj, now));

    reset(false, 0xFFFFF000u);   /* 32-bit millisecond counter wraps during the scenario */
    stay(300, 2000, 30, 30); move(300, 750, 500, 30, 30); stay(750, 6000, 30, 30);
    EXPECT("timer wrap-around: fall still confirmed", c.fall == 1);

    reset(true, 0);              /* person-detection model: box shape h/w is used */
    stay(450, 2000, 40, 100); stay(700, 5000, 100, 40);
    EXPECT("person model: lying shape confirms a fall", c.fall == 1);
    stay(450, 2000, 40, 100);
    EXPECT("person model: standing shape recovers", c.recover == 1);

    printf(g_fail ? "%d test(s) FAILED\n" : "all tests passed\n", g_fail);
    return g_fail ? EXIT_FAILURE : EXIT_SUCCESS;
}
