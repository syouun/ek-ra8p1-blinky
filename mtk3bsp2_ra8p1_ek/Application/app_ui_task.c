/*
 * app_ui_task.c - Privacy-preserving UI task (lowest application priority, APP_PRI_UI)
 *
 * Redraws the LCD every APP_UI_PERIOD_MS with only the monitoring state and an
 * abstract outline of where the person is. The camera image is never drawn.
 *
 *   SAFE (green)          : nobody in view
 *   MONITORING (amber)    : a person is in view
 *   CHECKING (orange)     : sudden drop detected, waiting for confirmation
 *   FALL DETECTED (red)   : fall confirmed
 *
 * Copyright (c) 2026 Makoto Tanaka (syouun)
 * SPDX-License-Identifier: MIT
 */
#include <stdio.h>
#include "app_common.h"
#include "rtos_to_mtk.h"            /* g_ai_app_event (sample event flag) */
#include "common_util.h"
#include "display_layer.h"
#include "display_layer_config.h"
#include "bg_font_18_full.h"
#include "ai_application_config.h"

#define FX(v)            ((d2_point) ((v) << 4))   /* D/AVE 2D uses 4-bit fixed point coordinates */

#define COLOR_SAFE       (0x2E7D4F)
#define COLOR_MONITOR    (0xB7791F)
#define COLOR_CHECK      (0xC75B12)
#define COLOR_ALERT      (0xB3261E)
#define COLOR_PANEL      (0x101010)
#define COLOR_VIEW       (0x202020)
#define COLOR_LINE       (0xFFFFFF)

/* Abstract view of the camera field (the model sees a square crop) */
#define VIEW_SIZE        (360)
#define VIEW_X           ((DISPLAY_SCREEN_WIDTH - VIEW_SIZE) / 2)
#define VIEW_Y           (165)

static void fill_box(int x, int y, int w, int h, d2_color color)
{
    d2_setcolor(d2_handle, 0, color);
    d2_renderbox(d2_handle, FX(x), FX(y), (d2_width) FX(w), (d2_width) FX(h));
}

static void outline_box(int x, int y, int w, int h, int t, d2_color color)
{
    fill_box(x, y, w, t, color);
    fill_box(x, y + h - t, w, t, color);
    fill_box(x, y, t, h, color);
    fill_box(x + w - t, y, t, h, color);
}

static void draw_screen(const app_status_t *st)
{
    d2_color    bg;
    const char *title;

    if (FJ_STATE_FALLEN == st->state)
    {
        bg = COLOR_ALERT;   title = "FALL DETECTED";
    }
    else if (FJ_STATE_SUSPICIOUS == st->state)
    {
        bg = COLOR_CHECK;   title = "CHECKING";
    }
    else if (st->person_present)
    {
        bg = COLOR_MONITOR; title = "MONITORING";
    }
    else
    {
        bg = COLOR_SAFE;    title = "SAFE";
    }

    /* Background = state colour, readable from across the room */
    fill_box(0, 0, DISPLAY_SCREEN_WIDTH, DISPLAY_SCREEN_HEIGHT, bg);

    /* Title panel */
    fill_box(40, 30, DISPLAY_SCREEN_WIDTH - 80, 110, COLOR_PANEL);
    print_bg_font_18(d2_handle, 70, 55, 3.0f, (char *) title);

    /* Abstract camera view: dark square + white outline of the detected person/face. No image. */
    fill_box(VIEW_X, VIEW_Y, VIEW_SIZE, VIEW_SIZE, COLOR_VIEW);
    outline_box(VIEW_X, VIEW_Y, VIEW_SIZE, VIEW_SIZE, 2, 0x606060);
    if (st->person_present && (st->box.m_w > 0) && (st->box.m_h > 0))
    {
        int x = VIEW_X + (st->box.m_x * VIEW_SIZE) / AI_INPUT_IMAGE_WIDTH;
        int y = VIEW_Y + (st->box.m_y * VIEW_SIZE) / AI_INPUT_IMAGE_HEIGHT;
        int w = (st->box.m_w * VIEW_SIZE) / AI_INPUT_IMAGE_WIDTH;
        int h = (st->box.m_h * VIEW_SIZE) / AI_INPUT_IMAGE_HEIGHT;
        outline_box(x, y, w, h, 4, COLOR_LINE);
    }

    /* Status line (the font supports letters, digits, space, %, - and /) */
    char line[64];
    snprintf(line, sizeof(line), "%s   Falls %u   Latency %u ms",
             st->person_present ? "Person in view" : "No person",
             (unsigned) st->fall_count, (unsigned) st->last_latency_ms);
    fill_box(40, 540, DISPLAY_SCREEN_WIDTH - 80, 50, COLOR_PANEL);
    print_bg_font_18(d2_handle, 60, 550, 1.5f, line);
}

void app_ui_task(INT stacd, void *exinf)
{
    UINT ptn;

    (void) stacd;
    (void) exinf;

    /* The camera task initialises GLCDC / D/AVE 2D; start drawing after that. */
    tk_wai_flg(g_ai_app_event, HARDWARE_DISPLAY_INIT_DONE, TWF_ANDW, &ptn, TMO_FEVR);
    app_log("[UI] privacy screen started (camera image is not displayed)\r\n");

    while (1)
    {
        app_status_t st;

        tk_dly_tsk(APP_UI_PERIOD_MS);
        app_heartbeat(APP_TASK_UI);
        app_status_get(&st);

        /* Draw in the vertical blanking period, as the original sample does. */
        tk_clr_flg(g_ai_app_event, ~(UINT) GLCDC_VSYNC);
        tk_wai_flg(g_ai_app_event, GLCDC_VSYNC, TWF_ORW, &ptn, 50);

        graphics_start_frame();
        draw_screen(&st);
        graphics_end_frame();
    }
}
