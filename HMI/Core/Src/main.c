/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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

 * @brief This file provides code for the configuration
 *        of the UART peripheral.
 ******************************************************************************/
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dac.h"
#include "dma.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "arm_math.h"
#include "arm_const_structs.h"
#include "tjc_usart_hmi.h"
#define FRAME_LENGTH 7
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* 锟斤拷瑜般��锟斤拷��锟藉������锟�? */
#define RING_BUFFER_SIZE 256
#define PWM_MAX 100
#define SAMPLES 1000
#define SAMPLERATE 500

/* �癸拷娑�锟斤拷锟借ぐ��锟斤拷��锟借�癸拷锟斤拷娴ｏ拷 */

/* 锟斤拷瑜般��锟斤拷��锟藉��锟斤拷��锟�?锟斤拷锟斤拷 */
uint8_t Sign_samplingOver = 0;
uint8_t Sign_wave_exist = 0;
uint16_t adc_cache[adc_cache_size];
uint16_t i_trigger = 0;
uint32_t adc_fs = 51.2e3;

arm_cfft_radix4_instance_f32 scfft;
float FFT_INPUT[adc_cache_size * 2];
float FFT_OUTPUT[adc_cache_size];
float FFT_OUTPUT_MAX = 0;
uint32_t FFT_OUTPUT_MAX_index = 0;

uint16_t pwm[SAMPLES] = { 0 };
uint16_t spwm[SAMPLERATE] = { 0 };

void spwm_table() {
	for (int i = 0; i < SAMPLES; ++i) {
		double angle = 2 * M_PI * i / SAMPLES; // 计算当前样本点的角度
		double sine_value = cos(angle);        // 计算正弦值（范围：-1到1）
		pwm[i] = round((sine_value + 1.0) * (PWM_MAX / 2));
	}
}

void generateWave(int freq) {
	const int period = SAMPLERATE / freq; // 计算一个周期的采样点数

	for (int i = 0; i < SAMPLERATE; ++i) {
		int pwmIndex = (i % period) * (sizeof(pwm) / sizeof(pwm[0]) - 1)
				/ period; // 计算当前采样点对应的pwm索引
		spwm[i] = pwm[pwmIndex] * 4095 / 100;
	}
}

// 假设 adc_trigger_size 是 adc_cache 数组的有效长度
#define THRESHOLD 50 // 上升沿检测阈值

float calculate_frequency(uint16_t *adc_cache, uint32_t adc_fs) {
	uint16_t rising_edges = 0;
	uint8_t last_state = 0;

	for (uint16_t i = 1; i < adc_trigger_size; i++) {
		if (adc_cache[i] > THRESHOLD && adc_cache[i - 1] <= THRESHOLD) {
			rising_edges++;
		}
	}

	// 计算频率
	if (rising_edges == 0) {
		return 0.0; // 没有检测到上升沿，返回0频率
	}

	float frequency = (rising_edges / 2.0) * (adc_fs / adc_trigger_size);
	return frequency;
}

float calculate_average_peak_to_peak(uint16_t *adc_cache) {
	uint16_t rising_edges = 0;
	uint8_t last_state = 0;
	uint16_t start_index = 0;
	uint16_t end_index = 0;
	uint16_t period = 0;

	// 检测第一个上升沿
	for (uint16_t i = 1; i < adc_trigger_size; i++) {
		if (adc_cache[i] > THRESHOLD && adc_cache[i - 1] <= THRESHOLD) {
			start_index = i;
			rising_edges++;
			break;
		}
	}

	// 检测第二个上升沿以确定周期
	for (uint16_t i = start_index + 1; i < adc_trigger_size; i++) {
		if (adc_cache[i] > THRESHOLD && adc_cache[i - 1] <= THRESHOLD) {
			end_index = i;
			rising_edges++;
			break;
		}
	}

	if (rising_edges < 2) {
		return 0.0; // 没有检测到足够的上升沿，返回0
	}

	period = end_index - start_index;

	// 计算5个周期的信号
	uint16_t total_samples = 5 * period;
	if (start_index + total_samples > adc_trigger_size) {
		return 0.0; // 信号长度不足5个周期，返回0
	}

	float sum_peak_to_peak = 0.0;
	for (uint16_t j = 0; j < 5; j++) {
		uint16_t min_value = 65535;
		uint16_t max_value = 0;
		for (uint16_t i = start_index + j * period;
				i < start_index + (j + 1) * period; i++) {
			if (adc_cache[i] < min_value) {
				min_value = adc_cache[i];
			}
			if (adc_cache[i] > max_value) {
				max_value = adc_cache[i];
			}
		}
		sum_peak_to_peak += max_value - min_value;
	}

	float average_peak_to_peak = sum_peak_to_peak / 5.0;
	return average_peak_to_peak;
}

void perform_adc_sampling() {
	Sign_wave_exist = 0;
	while (!Sign_samplingOver)
		;
	i_trigger = adc_trigger_size;
	uint8_t i;
	for (i = 0; i < adc_trigger_size; i++) {
		if (adc_cache[i] < 50 && adc_cache[i + 1] > 50) {
			i_trigger = i;
			Sign_wave_exist = 1;
			break;
		}
	}
	if (!Sign_wave_exist) {
		Sign_samplingOver = 0;
		HAL_ADC_Start_DMA(&hadc1, (uint32_t*) adc_cache, adc_cache_size);
	}
}

void perform_fft() {
	arm_cfft_f32(&arm_cfft_sR_f32_len1024, FFT_INPUT, 0, 1);
	arm_cmplx_mag_f32(FFT_INPUT, FFT_OUTPUT, adc_cache_size);
	arm_max_f32(FFT_OUTPUT, adc_cache_size, &FFT_OUTPUT_MAX,
			&FFT_OUTPUT_MAX_index);
}
/* PRIVATE FUNCTIONS */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLM = 8;
	RCC_OscInitStruct.PLL.PLLN = 168;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 4;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
		Error_Handler();
	}
}
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint16_t S_readyDisplay = 0; // 锟斤拷锟斤拷�╋拷��锟介��锟芥０锟斤拷瑙�锟窖�锟藉�с��
float Center_freq = 10.3;    // 娑�锟借�锟芥０锟斤拷锟�
float Bandwith = 16.5;       // ��锟界�癸拷
float IMinA = 78.1;          // ��锟斤拷锟斤拷锟界��锟界���匡拷锟�
float t_freq = 200.0;        // 娣��筹拷�斤拷锟斤拷�斤拷锟斤拷
float single_freq = 300.0;

/* 锟斤拷瑜般��锟斤拷��锟藉��锟斤拷婵�锟斤拷锟� */

/* 锟斤拷锟斤拷瑜般��锟斤拷��锟界����锟斤拷锟界��锟斤拷 */

/* 娴�锟斤拷锟借ぐ��锟斤拷��锟介缚锟借�诧拷锟斤拷��锟斤拷 */

/* 锟藉嘲锟斤拷锟斤拷瑜般��锟斤拷��锟借桨��锟斤拷锟界��锟斤拷锟藉府��? */

/* 婢讹拷锟斤拷锟姐��锟借泛锟芥��锟斤拷锟界��锟斤拷 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
	/* USER CODE BEGIN 1 */
	HAL_Init();
	SystemClock_Config();
	MX_GPIO_Init();
	MX_DMA_Init();
	MX_ADC1_Init();
	MX_DAC_Init();
	MX_TIM2_Init();
	MX_TIM3_Init();
	MX_GPIO_Init();
	MX_USART1_UART_Init();

	HAL_TIM_Base_Start(&htim2);
	HAL_TIM_Base_Start(&htim3);
	htim3.Instance->ARR = 84e6 / adc_fs - 1;
	Sign_samplingOver = 0;
	spwm_table();
	generateWave(200);
	HAL_DAC_Start_DMA(&hdac, DAC1_CHANNEL_1, (uint32_t*) spwm, 50000,
	DAC_ALIGN_12B_R);

	/* USER CODE BEGIN 2 */                       // 锟斤拷婵�锟斤拷锟斤拷锟借ぐ��锟斤拷��锟斤拷
	HAL_UART_Receive_IT(&TJC_UART, RxBuffer, 1); // 锟斤拷缂�锟芥�锟斤拷锟斤拷��锟斤拷
	int a = 100;
	char str[100];
	//	uint32_t nowtime = HAL_GetTick();
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		char str[10];
		sprintf(str, "x3.val=%d", (int) (single_freq * 100));
		tjc_send_string(str);

		sprintf(str, "x0.val=%d", (int) (Center_freq * 100));
		tjc_send_string(str);
		sprintf(str, "x1.val=%d", (int) (Bandwith * 100));
		tjc_send_string(str);
		sprintf(str, "x2.val=%d", (int) (IMinA * 100));
		tjc_send_string(str);

		sprintf(str, "x4.val=%d", (int) (t_freq * 100));
		tjc_send_string(str);
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */

		/* USER CODE END 3 */
	}
}

/**
 * @brief System Clock Configuration
 * @retval None
 */

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
