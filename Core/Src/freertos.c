/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lv_port_display.h"
#include "ui.h"
#include "tim.h"
#include "actions.h"
#include "src/themes/lv_theme_private.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for debugTask */
osThreadId_t debugTaskHandle;
const osThreadAttr_t debugTask_attributes = {
  .name = "debugTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for lvglTask */
osThreadId_t lvglTaskHandle;
const osThreadAttr_t lvglTask_attributes = {
  .name = "lvglTask",
  .stack_size = 2048 * 4,
  .priority = (osPriority_t) osPriorityNormal1,
};
/* Definitions for backlightTask */
osThreadId_t backlightTaskHandle;
const osThreadAttr_t backlightTask_attributes = {
  .name = "backlightTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for backlightQueue */
osMessageQueueId_t backlightQueueHandle;
const osMessageQueueAttr_t backlightQueue_attributes = {
  .name = "backlightQueue"
};
/* Definitions for lvglMutex */
osMutexId_t lvglMutexHandle;
const osMutexAttr_t lvglMutex_attributes = {
  .name = "lvglMutex"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void DebugTask(void *argument);
void LvglTask(void *argument);
void BacklightTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */
  /* Create the mutex(es) */
  /* creation of lvglMutex */
  lvglMutexHandle = osMutexNew(&lvglMutex_attributes);

  /* USER CODE BEGIN RTOS_MUTEX */
    /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
    /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
    /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of backlightQueue */
  backlightQueueHandle = osMessageQueueNew (4, sizeof(uint16_t), &backlightQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
    /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of debugTask */
  debugTaskHandle = osThreadNew(DebugTask, NULL, &debugTask_attributes);

  /* creation of lvglTask */
  lvglTaskHandle = osThreadNew(LvglTask, NULL, &lvglTask_attributes);

  /* creation of backlightTask */
  backlightTaskHandle = osThreadNew(BacklightTask, NULL, &backlightTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
    /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
    /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_DebugTask */
/**
  * @brief  Function implementing the debugTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_DebugTask */
void DebugTask(void *argument)
{
  /* USER CODE BEGIN DebugTask */

    /* Infinite loop */
    for (;;) {
        HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
        osDelay(100);
    }
  /* USER CODE END DebugTask */
}

/* USER CODE BEGIN Header_LvglTask */
/**
* @brief Function implementing the lvglTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_LvglTask */
void LvglTask(void *argument)
{
  /* USER CODE BEGIN LvglTask */
    lv_port_display_init();
    ui_init();
    /* Infinite loop */
    for (;;) {
        osMutexAcquire(lvglMutexHandle, osWaitForever);
        lv_timer_handler();
        osMutexRelease(lvglMutexHandle);
        osDelay(5);
    }
  /* USER CODE END LvglTask */
}

/* USER CODE BEGIN Header_BacklightTask */
/**
* @brief Function implementing the backlightTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_BacklightTask */
void BacklightTask(void *argument)
{
  /* USER CODE BEGIN BacklightTask */
    uint16_t backlight_level;
    /* Infinite loop */
    for (;;) {
        if (osMessageQueueGet(backlightQueueHandle, &backlight_level, 0, osWaitForever) == osOK) {
            __HAL_TIM_SET_COMPARE(&htim12, TIM_CHANNEL_1, backlight_level);
        }
        osDelay(1);
    }
  /* USER CODE END BacklightTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void action_set_backlight(lv_event_t *e) {
    lv_obj_t *obj = lv_event_get_target(e);
    uint16_t backlight = lv_slider_get_value(obj);
    osMessageQueuePut(backlightQueueHandle, &backlight, 0, osWaitForever);
}

void action_reset_backlight(lv_event_t *e) {
    uint16_t backlight = 20;
    lv_slider_set_value(objects.backlight_slider, backlight, LV_ANIM_OFF);
    osMessageQueuePut(backlightQueueHandle, &backlight, 0, osWaitForever);
}

void action_change_theme(lv_event_t *e) {
    static bool is_dark = false;
    is_dark = !is_dark;
    lv_theme_t *theme = lv_theme_default_init(NULL, lv_palette_main(LV_PALETTE_BLUE),
                                              lv_palette_main(LV_PALETTE_BLUE_GREY), is_dark, &lv_font_montserrat_14);
    lv_display_set_theme(NULL, theme);
}

/* USER CODE END Application */

