/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    adc.c
 * @brief   This file provides code for the configuration
 *          of the ADC instances.
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
/* Includes ------------------------------------------------------------------*/
#include "adc.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

/* ADC1 init function */
void MX_ADC1_Init(void) {

	/* USER CODE BEGIN ADC1_Init 0 */

	/* USER CODE END ADC1_Init 0 */

	ADC_ChannelConfTypeDef sConfig = { 0 };

	/* USER CODE BEGIN ADC1_Init 1 */

	/* USER CODE END ADC1_Init 1 */

	/** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
	 */
	hadc1.Instance = ADC1;
	hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
	hadc1.Init.Resolution = ADC_RESOLUTION_12B;
	hadc1.Init.ScanConvMode = DISABLE;
	hadc1.Init.ContinuousConvMode = ENABLE;
	hadc1.Init.DiscontinuousConvMode = DISABLE;
	hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
	hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T3_TRGO;
	hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc1.Init.NbrOfConversion = 1;
	hadc1.Init.DMAContinuousRequests = DISABLE;
	hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
	if (HAL_ADC_Init(&hadc1) != HAL_OK) {
		Error_Handler();
	}

	/** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
	 */
	sConfig.Channel = ADC_CHANNEL_5;
	sConfig.Rank = 1;
	sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN ADC1_Init 2 */

	/* USER CODE END ADC1_Init 2 */
}

void HAL_ADC_MspInit(ADC_HandleTypeDef *adcHandle) {

	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	if (adcHandle->Instance == ADC1) {
		/* USER CODE BEGIN ADC1_MspInit 0 */

		/* USER CODE END ADC1_MspInit 0 */
		/* ADC1 clock enable */
		__HAL_RCC_ADC1_CLK_ENABLE();

		__HAL_RCC_GPIOA_CLK_ENABLE();
		/**ADC1 GPIO Configuration
		 PA5         ------> ADC1_IN5
		 */
		GPIO_InitStruct.Pin = GPIO_PIN_5;
		GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		/* ADC1 DMA Init */
		/* ADC1 Init */
		hdma_adc1.Instance = DMA2_Stream0;
		hdma_adc1.Init.Channel = DMA_CHANNEL_0;
		hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
		hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
		hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
		hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
		hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
		hdma_adc1.Init.Mode = DMA_NORMAL;
		hdma_adc1.Init.Priority = DMA_PRIORITY_LOW;
		hdma_adc1.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
		hdma_adc1.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
		hdma_adc1.Init.MemBurst = DMA_MBURST_SINGLE;
		hdma_adc1.Init.PeriphBurst = DMA_PBURST_SINGLE;
		if (HAL_DMA_Init(&hdma_adc1) != HAL_OK) {
			Error_Handler();
		}

		__HAL_LINKDMA(adcHandle, DMA_Handle, hdma_adc1);

		/* USER CODE BEGIN ADC1_MspInit 1 */

		/* USER CODE END ADC1_MspInit 1 */
	}
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef *adcHandle) {

	if (adcHandle->Instance == ADC1) {
		/* USER CODE BEGIN ADC1_MspDeInit 0 */

		/* USER CODE END ADC1_MspDeInit 0 */
		/* Peripheral clock disable */
		__HAL_RCC_ADC1_CLK_DISABLE();

		/**ADC1 GPIO Configuration
		 PA5         ------> ADC1_IN5
		 */
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_5);

		/* ADC1 DMA DeInit */
		HAL_DMA_DeInit(adcHandle->DMA_Handle);
		/* USER CODE BEGIN ADC1_MspDeInit 1 */

		/* USER CODE END ADC1_MspDeInit 1 */
	}
}

/* USER CODE BEGIN 1 */
// 锟斤拷志位
// 锟斤拷锟斤拷锟轿伙拷锟斤拷预锟斤拷 & 锟斤拷锟斤拷锟斤拷桑锟斤拷锟街疚伙拷锟斤拷锟斤拷锟斤拷刹锟斤拷尾锟斤拷锟斤拷锟斤拷锟绞撅拷锟斤拷锟�
// uint8_t Sign_readyDisplay = 0;
// uint8_t Sign_samplingOver = 0;
extern uint8_t Sign_readyDisplay;
extern uint8_t Sign_samplingOver;
extern uint16_t adc_cache[adc_cache_size];
// ADC_DMA锟截碉拷锟斤拷锟斤拷锟捷憋拷志位锟叫讹拷cpu锟斤拷前状态锟斤拷执锟叫ｏ拷锟斤拷锟斤拷锟斤拷锟斤拷ADC锟斤拷锟斤拷 or 停止锟斤拷锟斤拷锟斤拷锟饺达拷cpu锟斤拷图锟斤拷
// ADC_DMAConvCplt
// HAL_ADC_ConvCpltCallback
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
	// 锟斤拷锟斤拷DMA锟斤拷锟捷达拷锟斤拷锟斤拷希锟斤拷锟街疚伙拷锟�1
	Sign_samplingOver = 1;
	// 锟斤拷锟斤拷卸媳锟街疚�
	//__HAL_DMA_CLEAR_FLAG(&hdma_adc1, DMA_FLAG_TCIF0_4);
	// 锟叫讹拷cpu状态
	if (Sign_samplingOver == 1) {
		// cpu锟窖撅拷准锟斤拷锟矫伙拷图锟斤拷DMA锟斤拷锟劫达拷锟斤拷锟斤拷锟斤拷锟斤拷锟捷ｏ拷锟斤拷cpu锟斤拷图锟斤拷锟斤拷锟斤拷锟斤拷cpu锟劫次匡拷锟斤拷ADC-DMA
		// DMA选锟斤拷normal模式锟斤拷锟斤拷锟劫匡拷锟斤拷锟斤拷锟截憋拷
		;
	} else {
		// cpu未准锟斤拷锟矫伙拷图锟斤拷DMA锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
		// 锟斤拷志位锟斤拷0锟斤拷锟斤拷始锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
		Sign_samplingOver = 0;
		HAL_ADC_Start_DMA(&hadc1, (uint32_t*) adc_cache, adc_cache_size);
	}
}

/* USER CODE END 1 */
