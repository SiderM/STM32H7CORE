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
#include "actions.h"
#include "tim.h"
#include "fdcan.h"
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
uint8_t can_data_msg = 0;
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
/* Definitions for canRxTask */
osThreadId_t canRxTaskHandle;
const osThreadAttr_t canRxTask_attributes = {
    .name = "canRxTask",
    .stack_size = 1024 * 4,
    .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for backlightQueue */
osMessageQueueId_t backlightQueueHandle;
const osMessageQueueAttr_t backlightQueue_attributes = {
    .name = "backlightQueue"
};
/* Definitions for canRxQueue */
osMessageQueueId_t canRxQueueHandle;
const osMessageQueueAttr_t canRxQueue_attributes = {
    .name = "canRxQueue"
};
/* Definitions for lvglMutex */
osMutexId_t lvglMutexHandle;
const osMutexAttr_t lvglMutex_attributes = {
    .name = "lvglMutex"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void my_timer(lv_timer_t *timer);

/* USER CODE END FunctionPrototypes */

void DebugTask(void *argument);

void LvglTask(void *argument);

void BacklightTask(void *argument);

void CanRxTask(void *argument);

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
    backlightQueueHandle = osMessageQueueNew(4, sizeof(uint16_t), &backlightQueue_attributes);

    /* creation of canRxQueue */
    canRxQueueHandle = osMessageQueueNew(16, sizeof(CAN_RxMessageTypeDef), &canRxQueue_attributes);

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

    /* creation of canRxTask */
    canRxTaskHandle = osThreadNew(CanRxTask, NULL, &canRxTask_attributes);

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
void DebugTask(void *argument) {
    /* USER CODE BEGIN DebugTask */
    FDCAN_TxHeaderTypeDef TxHeader;
    uint8_t TxBuffer[8];

    TxHeader.Identifier = 0x321;
    TxHeader.IdType = FDCAN_STANDARD_ID;
    TxHeader.TxFrameType = FDCAN_DATA_FRAME;
    TxHeader.DataLength = FDCAN_DLC_BYTES_8;
    TxHeader.ErrorStateIndicator = FDCAN_ESI_PASSIVE;
    TxHeader.BitRateSwitch = FDCAN_BRS_OFF;
    TxHeader.FDFormat = FDCAN_CLASSIC_CAN;
    TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    TxHeader.MessageMarker = 0;

    TxBuffer[0] = 0x00;
    /* Infinite loop */
    for (;;) {
        TxBuffer[0]++;
        HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, TxBuffer);
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
void LvglTask(void *argument) {
    /* USER CODE BEGIN LvglTask */
    lv_port_display_init();
    ui_init();
    // lv_timer_t *timer = lv_timer_create(my_timer, 100,  NULL);
    // lv_timer_set_repeat_count(timer, -1);
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
void BacklightTask(void *argument) {
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

/* USER CODE BEGIN Header_CanRxTask */
/**
* @brief Function implementing the canRxTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_CanRxTask */
void CanRxTask(void *argument) {
    /* USER CODE BEGIN CanRxTask */
    CAN_RxMessageTypeDef CAN_RxMessage;

    /* Infinite loop */
    for (;;) {
        if (osMessageQueueGet(canRxQueueHandle, &CAN_RxMessage, 0, osWaitForever) == osOK) {
            osMutexAcquire(lvglMutexHandle, osWaitForever);
            HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
            can_data_msg = CAN_RxMessage.Data[0];
            lv_label_set_text_fmt(objects.label, "%d", can_data_msg);
            if (can_data_msg == 128) {
                lv_obj_set_flag(objects.loading, LV_OBJ_FLAG_HIDDEN, true);
            }
            osMutexRelease(lvglMutexHandle);
        }
        osDelay(1);
    }
    /* USER CODE END CanRxTask */
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

void my_timer(lv_timer_t *timer) {
    lv_label_set_text_fmt(objects.label, "%d", can_data_msg);
}

void action_set_loading(lv_event_t *e) {
    lv_obj_set_flag(objects.loading, LV_OBJ_FLAG_HIDDEN, false);
}

/* USER CODE END Application */
