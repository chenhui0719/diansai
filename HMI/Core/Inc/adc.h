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

//3.5����Ļ�ֱ���320*480��������ʾ
//ʵ���õ������ݵ�С�ڻ���ĵ㣬��Ϊ��ĻUI��ռ��һЩλ��
//ͬʱ��Ѱ�Ҵ������ͼ���±�Ȼ������һЩ�㣬��������Щ����UI���������ĵ���Ѱ�Ҵ�����
//���������ý���������Ϊ480����480��Ϊ������������ʣ�����в��λ��ƣ��������ᵼ���޴�����Ķ��Թ��ڲ��ʣ�������ɳɲ���Ҫ��������ʧ
//�ɴ౩���㣬��������Ϊ960����480��Ϊ��������������Ĵ��������޷��ҵ�������Ĳ��Σ���ֱ�ӻ���
//������Ϣ��FFT��Ҫ֧��4�ı�����Ϊ��������������˻����Ϊ1024�����������Ϊ1024-480=544
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

