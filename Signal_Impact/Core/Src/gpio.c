/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
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
#include "gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins
*/
void MX_GPIO_Init(void)
{

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(LCD_BL_GPIO_Port, LCD_BL_Pin, GPIO_PIN_SET);

    /*Configure GPIO pins : PEPin PEPin PEPin */
    GPIO_InitStruct.Pin = KEY2_toggleRoom_Pin|KEY1_ToggleSelect_Pin|KEY0_stopDrawUpdate_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    /*Configure GPIO pin : PtPin */
    GPIO_InitStruct.Pin = KEY_ToggleWaveTable_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(KEY_ToggleWaveTable_GPIO_Port, &GPIO_InitStruct);

    /*Configure GPIO pin : PtPin */
    GPIO_InitStruct.Pin = LCD_BL_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(LCD_BL_GPIO_Port, &GPIO_InitStruct);

    /* EXTI interrupt init*/
    HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);

    HAL_NVIC_SetPriority(EXTI2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI2_IRQn);

    HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI3_IRQn);

    HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI4_IRQn);

}

/* USER CODE BEGIN 2 */
extern wave wave_t;
extern wave wave_f;
extern uint8_t Sign_dacToggleWave;
extern uint8_t Sign_stringUpdate;
extern uint8_t Sign_stop;
extern uint8_t Sign_function;
extern uint8_t valueData_type;
extern uint16_t wave_table_i;
extern uint16_t wave_frequency;
extern uint16_t zoomInMultiple;
extern char show_string_wave_frequency[8];
#include "tim.h"
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	switch(GPIO_Pin){
		case KEY_ToggleWaveTable_Pin:
			//进来先失能中断防止多次触发中断
			HAL_NVIC_DisableIRQ(KEY_ToggleWaveTable_EXTI_IRQn);
			switch(Sign_function){
			case 0:
				//拨动波形指针，切换DAC即将发生的波形
				if(wave_table_i < 3)wave_table_i++;
				else wave_table_i = 0;
				//切换DAC波形标志置位，留待主程序查询完成相关操作
				Sign_dacToggleWave = 1;
				break;
			case 1:
				//改变波形频率
				if(wave_frequency < 2000){
					wave_frequency+=100;
				}
				else{wave_frequency = 100;}
				htim2.Instance->ARR = 84e4/wave_frequency -1;
				sprintf(show_string_wave_frequency, "%dHz", wave_frequency);
				break;
			case 2:
				if(zoomInMultiple < 4)zoomInMultiple++;
				else zoomInMultiple = 1;
				break;
			case 3:
				//Toggle Sign_stop 来控制程序启停
				if(Sign_stop == 0)Sign_stop = 1;
				else Sign_stop = 0;
				break;
			case 4:
				//切换显示的数据类型（基波频率、幅值）
				if(valueData_type == 0)valueData_type = 1;
				else valueData_type = 0;
				break;
			default:
				break;
			}
			//处理完该中断对应事件后再次开启该中断
			//尝试了延迟软件消抖，效果很差，用定时器又感觉不如干两个电容上去
			//故不再在主程序中管理中断或添加中断开启前延迟
			HAL_NVIC_EnableIRQ(KEY_ToggleWaveTable_EXTI_IRQn);
			break;
		case KEY0_stopDrawUpdate_Pin:
			HAL_NVIC_DisableIRQ(KEY0_stopDrawUpdate_EXTI_IRQn);
			if(Sign_function == 0)Sign_function=4;
			else Sign_function--;
			HAL_NVIC_EnableIRQ(KEY0_stopDrawUpdate_EXTI_IRQn);
			break;
		case KEY1_ToggleSelect_Pin:
			HAL_NVIC_DisableIRQ(KEY1_ToggleSelect_EXTI_IRQn);
			switch(Sign_function){
			case 0:
				//拨动波形指针，切换DAC即将发生的波形
				if(wave_table_i > 0)wave_table_i--;
				else wave_table_i = 3;
				//切换DAC波形标志置位，留待主程序查询完成相关操作
				Sign_dacToggleWave = 1;
				break;
			case 1:
				//改变波形频率
				if(wave_frequency > 100){
					wave_frequency-=100;
				}
				else{wave_frequency = 2000;}
				htim2.Instance->ARR = 84e4/wave_frequency -1;
				sprintf(show_string_wave_frequency, "%dHz", wave_frequency);
				break;
			case 2:
				if(zoomInMultiple > 1)zoomInMultiple--;
				else zoomInMultiple = 4;
				break;
			case 3:
				//Toggle Sign_stop 来控制程序启停
				if(Sign_stop == 0)Sign_stop = 1;
				else Sign_stop = 0;
				break;
			case 4:
				//切换显示的数据类型（基波频率、幅值）
				if(valueData_type == 0)valueData_type = 1;
				else valueData_type = 0;
				break;
			default:
				break;
			}
			HAL_NVIC_EnableIRQ(KEY1_ToggleSelect_EXTI_IRQn);
			break;
		case KEY2_toggleRoom_Pin:
			HAL_NVIC_DisableIRQ(KEY2_toggleRoom_EXTI_IRQn);
			if(Sign_function == 4)Sign_function = 0;
			else Sign_function++;
			HAL_NVIC_EnableIRQ(KEY2_toggleRoom_EXTI_IRQn);
			break;
		default:break;
	}
	//置标志位，文本需要更新
	Sign_stringUpdate = 1;
}

/* USER CODE END 2 */
