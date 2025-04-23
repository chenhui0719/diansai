/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    adc.h
  * @brief   This file contains all the function prototypes for
  *          the adc.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#ifndef __ADC_H__
#define __ADC_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */



/* USER CODE END Includes */

extern ADC_HandleTypeDef hadc1;

/* USER CODE BEGIN Private defines */

//3.5寸屏幕分辨率320*480，横屏显示
//实际用到的数据点小于缓存的点，因为屏幕UI会占据一些位置
//同时，寻找触发点绘图导致必然绘舍弃一些点，正好在这些“被UI藏起来”的点中寻找触发点
//本打算设置将缓存设置为480，用480作为触发区域，利用剩余点进行波形绘制，但这样会导致无触发点的断言过于草率，进而造成成不必要的数据损失
//干脆暴力点，将缓存设为960，用480作为触发区域，这样大的触发区域都无法找到触发点的波形，便直接绘制
//最新消息，FFT主要支持4的倍数作为采样点数量，因此缓存改为1024，触发区域改为1024-480=544
#define adc_trigger_size 544
#define adc_cache_size 1024


/* USER CODE END Private defines */

void MX_ADC1_Init(void);

/* USER CODE BEGIN Prototypes */

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __ADC_H__ */

