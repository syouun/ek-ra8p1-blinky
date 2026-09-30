#ifndef RTOS_TO_MTK_H
#define RTOS_TO_MTK_H

#include "tk/tkernel.h"
#include "hal_data.h"

// FreeRTOS Mappings
typedef int BaseType_t;
typedef UINT EventBits_t;
#define pdFALSE 0
#define pdTRUE  1
#define pdFAIL  0
#define pdPASS  1

#ifndef portMAX_DELAY
#define portMAX_DELAY         TMO_FEVR
#endif

#define portYIELD_FROM_ISR(x) (void)(x)
#define vTaskDelay(x)         tk_dly_tsk(x)

// Event flags (FreeRTOS EventGroup -> uT-Kernel event flag)
// g_ai_app_event is created in app_main.c with TA_WMUL.
extern ID g_ai_app_event;

static inline EventBits_t mtk_evg_get_bits(ID ev)
{
    T_RFLG rflg;
    return (tk_ref_flg(ev, &rflg) == E_OK) ? (EventBits_t) rflg.flgptn : 0U;
}

static inline EventBits_t mtk_evg_clear_bits(ID ev, UINT bits)
{
    EventBits_t prev = mtk_evg_get_bits(ev);
    tk_clr_flg(ev, ~bits);                      /* tk_clr_flg ANDs the pattern */
    return prev;
}

/* FreeRTOS semantics: returns the bits at release (or current bits on timeout).
 * xClearOnExit -> TWF_BITCLR (clears only the waited bits). ticks are 1ms. */
static inline EventBits_t mtk_evg_wait_bits(ID ev, UINT bits, BaseType_t clear, BaseType_t wait_all, TMO tmo)
{
    UINT ptn  = 0;
    UINT mode = (wait_all ? TWF_ANDW : TWF_ORW) | (clear ? TWF_BITCLR : 0U);
    if (tk_wai_flg(ev, bits, mode, &ptn, tmo) == E_OK)
    {
        return (EventBits_t) ptn;
    }
    return mtk_evg_get_bits(ev);
}

/* Calls from FSP callbacks (= interrupt context). uT-Kernel must be told it is in the
 * task-independent part, exactly like mtk3_bsp2's own HAL drivers do with
 * ENTER_TASK_INDEPENDENT / LEAVE_TASK_INDEPENDENT (knl_taskindp++ / --). */
extern W knl_taskindp;
static inline BaseType_t mtk_evg_set_bits_isr(ID ev, UINT bits)
{
    knl_taskindp++;
    ER er = tk_set_flg(ev, bits);
    knl_taskindp--;
    return (er == E_OK) ? pdPASS : pdFAIL;
}
static inline EventBits_t mtk_evg_clear_bits_isr(ID ev, UINT bits)
{
    knl_taskindp++;
    EventBits_t prev = mtk_evg_clear_bits(ev, bits);
    knl_taskindp--;
    return prev;
}
#define xEventGroupSetBitsFromISR(ev, bits, woken) mtk_evg_set_bits_isr((ev), (UINT)(bits))
#define xEventGroupSetBits(ev, bits)               tk_set_flg((ev), (bits))
#define xEventGroupGetBits(ev)                     mtk_evg_get_bits(ev)
#define xEventGroupGetBitsFromISR(ev)              mtk_evg_get_bits(ev)
#define xEventGroupClearBits(ev, bits)             mtk_evg_clear_bits((ev), (UINT)(bits))
#define xEventGroupClearBitsFromISR(ev, bits)      mtk_evg_clear_bits_isr((ev), (UINT)(bits))
#define xEventGroupWaitBits(ev, bits, clear, wait_all, ticks) \
        mtk_evg_wait_bits((ev), (UINT)(bits), (clear), (wait_all), (TMO)(ticks))

// Board pins (EK-RA8P1). Prefer the symbolic names generated in bsp_pin_cfg.h.
#ifndef CAM_RST
#define CAM_RST BSP_IO_PORT_07_PIN_09   /* Camera reset (was wrongly P309 = Ethernet RGMII1_TXC) */
#endif

#ifndef LCD_BLEN
#define LCD_BLEN BSP_IO_PORT_05_PIN_14
#endif

#ifndef MIPI_IF_EN
#define MIPI_IF_EN BSP_IO_PORT_01_PIN_08
#endif

#endif // RTOS_TO_MTK_H
