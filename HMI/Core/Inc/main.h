/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define KEY2_toggleRoom_Pin GPIO_PIN_2
#define KEY2_toggleRoom_GPIO_Port GPIOE
#define KEY2_toggleRoom_EXTI_IRQn EXTI2_IRQn
#define KEY1_ToggleSelect_Pin GPIO_PIN_3
#define KEY1_ToggleSelect_GPIO_Port GPIOE
#define KEY1_ToggleSelect_EXTI_IRQn EXTI3_IRQn
#define KEY0_stopDrawUpdate_Pin GPIO_PIN_4
#define KEY0_stopDrawUpdate_GPIO_Port GPIOE
#define KEY0_stopDrawUpdate_EXTI_IRQn EXTI4_IRQn
#define KEY_ToggleWaveTable_Pin GPIO_PIN_0
#define KEY_ToggleWaveTable_GPIO_Port GPIOA
#define KEY_ToggleWaveTable_EXTI_IRQn EXTI0_IRQn
#define LCD_BL_Pin GPIO_PIN_15
#define LCD_BL_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

// 3.5寸屏幕分辨率320*480，横屏显示
// 波形绘制区域大小
// 为使图像尽可能精细，采样点独立对齐屏幕横向每个点，故设置为480
#define plot_size 480

  // 波形名称
  // 波形序号
  // 波形缩放倍数
  // 波形绘图数据
  typedef struct
  {
    char waveName;      //'t' | 'f'
    uint16_t waveOlder; // 只有t和f两个，waveOlder={1, 2}
    //	uint16_t waveZoom;//波形缩放倍数，默认为1。waveZoom={1, 2, 3, 4}
    uint16_t draw_cache[plot_size][2];
  } wave;
  /* USER CODE BEGIN Private defines */

  /* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
