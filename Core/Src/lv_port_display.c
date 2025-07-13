//
// Created by maksim on 12.07.25.
//

#include "lv_port_display.h"
#include "bsp.h"
#include "ltdc.h"
#include "dma2d.h"
#include "lvgl.h"
#include "tim.h"

static void disp_flush_cb(lv_display_t *display, const lv_area_t *area,
                          uint8_t *color_p) {
    SCB_CleanInvalidateDCache();
    SCB_InvalidateICache();
    const uint32_t address = hltdc.LayerCfg[0].FBStartAdress
                             + (hltdc.LayerCfg[0].ImageWidth * area->y1 + area->x1) * 2;

    hdma2d.Init.OutputOffset = hltdc.LayerCfg[0].ImageWidth
                               - lv_area_get_width(area);
    HAL_DMA2D_Init(&hdma2d);
    HAL_DMA2D_Start_IT(&hdma2d, (uint32_t) color_p, address,
                       lv_area_get_width(area), lv_area_get_height(area));
    HAL_DMA2D_PollForTransfer(&hdma2d, HAL_MAX_DELAY);

    lv_display_flush_ready(display);
}

static void touch_read_cb(lv_indev_t *indev, lv_indev_data_t *data) {
    TS_StateTypeDef state;
    BSP_TS_GetState(&state);

    if (state.TouchDetected) {
        data->point.x = state.X;
        data->point.y = state.Y;
        data->state = LV_INDEV_STATE_PRESSED;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

void lv_port_display_init(void) {
    lv_init();
    lv_tick_set_cb(HAL_GetTick);
    HAL_TIM_PWM_Start(&htim12, TIM_CHANNEL_1);

    lv_display_t *display1 = lv_display_create(hltdc.LayerCfg[0].ImageWidth,
                                               hltdc.LayerCfg[0].ImageHeight);
    lv_display_set_buffers(display1, (void *) 0xC0200000, (void *) 0xC0300000,
                           800 * 480, LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(display1, disp_flush_cb);

    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, touch_read_cb);
}
