/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
#include "main.h"
#include "adc.h"
#include "dac.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "arm_math.h"
#include "arm_const_structs.h"
#include "stdio.h"
#include "tjc_usart_hmi.h"
#include "waveform_table.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define PWM_MAX 100
#define SAMPLES 20000
#define SAMPLERATE 10000
#define BUFFER_SIZE 100

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint16_t pwm[SAMPLES] = { 0 };
uint16_t spwm[SAMPLERATE] = { 0 };

uint32_t adc_fs = 51.2e3;
uint8_t Sign_samplingOver = 0;
uint16_t adc_cache[adc_cache_size];
uint16_t i_trigger = 0;
uint8_t Sign_wave_exist = 0;

arm_cfft_radix4_instance_f32 scfft;	 // FFT�ṹ??
float FFT_INPUT[adc_cache_size * 2]; // ��Ӧÿ��ʵ�������鲿����??2����
float FFT_OUTPUT[adc_cache_size]; // FFT�����????�˴�����ÿ��float�ߵ�16λ��??���ʵ���鲿������?2??
float FFT_OUTPUT_MAX = 0;
uint32_t FFT_OUTPUT_MAX_index = 0;

float Center_freq = 100;
float Bandwith = 16.5;
float IMinA = 78.1;
int page = 0;
int mode = 0;
int waveform = 0;
float freq = 100.0;
uint8_t Buffer[BUFFER_SIZE];
uint32_t singlefreq;
uint8_t freqins[10];
int top = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void spwm_table() {
	for (int i = 0; i < SAMPLES; ++i) {
		double angle = 2 * M_PI * i / SAMPLES; // ���㵱ǰ������ĽǶ�?
		double sine_value = cos(angle);		   // ��������ֵ����Χ??-1??1??
		pwm[i] = round((sine_value + 1.0) * (PWM_MAX / 2));
	}
}

void generateWave(uint32_t freq) {
	const int period = SAMPLERATE / freq; // ����һ??���ڵĲ�����??

	for (int i = 0; i < SAMPLERATE; ++i) {
		int pwmIndex = (i % period) * (sizeof(pwm) / sizeof(pwm[0]) - 1)
				/ period; // ���㵱ǰ������???Ӧ��pwm����
		spwm[i] = pwm[pwmIndex] * (4095 * 2.9 / 3.3) / 100;
	}
}

float findfreq() {
	Sign_wave_exist = 0;
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*) adc_cache, adc_cache_size);
	while (!Sign_samplingOver)
		;
	i_trigger = adc_trigger_size;
	uint16_t i;
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

	for (i = 0; i < adc_cache_size; i++) { // ��������ת��
		FFT_INPUT[i * 2] = (float) (adc_cache[i]);
		FFT_INPUT[i * 2 + 1] = 0; // �������?������ݵ���?
	}
	// FFT����
	arm_cfft_f32(&arm_cfft_sR_f32_len1024, FFT_INPUT, 0, 1);
	// ת��Ϊʵ����ȡģ|���� FFT ����ķ���?
	arm_cmplx_mag_f32(FFT_INPUT, FFT_OUTPUT, adc_cache_size);
	// �Ƿ�ȥ��ֱ����������ע���Ǿ���ȥ��
	//		FFT_OUTPUT[0]=0;
	// ȡ�����ֵ����һ�����ͼӳ������
	arm_max_f32(FFT_OUTPUT, adc_cache_size, &FFT_OUTPUT_MAX,
			&FFT_OUTPUT_MAX_index);
	FFT_OUTPUT[0] = 0;
	arm_max_f32(FFT_OUTPUT, adc_cache_size / 2, &FFT_OUTPUT_MAX,
			&FFT_OUTPUT_MAX_index);
	float base_frequency = adc_fs * FFT_OUTPUT_MAX_index / 1024;
	return base_frequency;
}
float findvpp() {
	Sign_wave_exist = 0;
	// 等待当前????ADC-DMA传输完成
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*) adc_cache, adc_cache_size);
	while (!Sign_samplingOver)
		;
	// 寻找触发??
	i_trigger = adc_trigger_size; // 找不到便显示最新采样结??
	uint16_t i;
	for (i = 0; i < adc_trigger_size; i++) {
		if (adc_cache[i] < 50 && adc_cache[i + 1] > 50) {
			i_trigger = i;
			Sign_wave_exist = 1;
			break;
		}
	}
	if (!Sign_wave_exist) { // 波形不存在则重新采样
		Sign_samplingOver = 0;
		HAL_ADC_Start_DMA(&hadc1, (uint32_t*) adc_cache, adc_cache_size);
		//    continue;
	}
	float wave[adc_cache_size], vmax, vmin, vpp;
	int max_pos, min_pos;
	for (i = 0; i < adc_cache_size; i++) {
		wave[i] = adc_cache[i] * 3.3 / 4095; // �?换电�?
	}
	arm_max_f32(wave, 1024, &vmax, &max_pos);
	arm_min_f32(wave, 1024, &vmin, &min_pos);
	vpp = vmax - vmin;
	return vpp;
}
float findvmax() {
	Sign_wave_exist = 0;
	// 等待当前????ADC-DMA传输完成
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*) adc_cache, adc_cache_size);
	while (!Sign_samplingOver)
		;
	// 寻找触发??
	i_trigger = adc_trigger_size; // 找不到便显示最新采样结??
	uint16_t i;
	for (i = 0; i < adc_trigger_size; i++) {
		if (adc_cache[i] < 50 && adc_cache[i + 1] > 50) {
			i_trigger = i;
			Sign_wave_exist = 1;
			break;
		}
	}
	if (!Sign_wave_exist) { // 波形不存在则重新采样
		Sign_samplingOver = 0;
		HAL_ADC_Start_DMA(&hadc1, (uint32_t*) adc_cache, adc_cache_size);
		//    continue;
	}
	float wave[adc_cache_size], vmax, vmin, vpp;
	int max_pos, min_pos;
	for (i = 0; i < adc_cache_size; i++) {
		wave[i] = adc_cache[i] * 3.3 / 4095; // �?换电�?
	}
	arm_max_f32(wave, 1024, &vmax, &max_pos);
	arm_min_f32(wave, 1024, &vmin, &min_pos);
	return vmax;
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART1) {
		if (*Buffer == 0x01)
			page = 1; // SPWM������Ҳ�ģ��?
		if (*Buffer == 0x02)
			page = 2; // ��Ƶ���Բ���ģ��
		if (*Buffer == 0x03)
			page = 3; // ������ʾģ��
		if (*Buffer == 0x06)
			page = 0; // ���г���ȫ��ֹͣ
		if (*Buffer == 0x05)
			mode = 2; // ɨƵ����
		if (*Buffer == 0x04) {
			mode = 1; // 信号发生模式
			top = 0;
		}
		if (*Buffer == 0x11)
			freqins[top++] = 1;
		if (*Buffer == 0x12)
			freqins[top++] = 2;
		if (*Buffer == 0x13)
			freqins[top++] = 3;
		if (*Buffer == 0x14)
			freqins[top++] = 4;
		if (*Buffer == 0x15)
			freqins[top++] = 5;
		if (*Buffer == 0x16)
			freqins[top++] = 6;
		if (*Buffer == 0x17)
			freqins[top++] = 7;
		if (*Buffer == 0x18)
			freqins[top++] = 8;
		if (*Buffer == 0x19)
			freqins[top++] = 9;
		if (*Buffer == 0x10)
			freqins[top++] = 0;
		if (*Buffer == 0x99) {
			uint8_t i;
			for (i = 0; i < top; i++) {
				singlefreq = singlefreq * 10 + freqins[i];
			}
		}
		if (*Buffer == 0x98) {
			top--;
			if (top < 0)
				top = 0;
		}
	}

	HAL_UART_Receive_IT(&huart1, (uint8_t*) Buffer, 1);
}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_DMA_Init();
	MX_ADC1_Init();
	MX_DAC_Init();
	MX_TIM2_Init();
	MX_TIM3_Init();
	MX_USART1_UART_Init();
	/* USER CODE BEGIN 2 */
	HAL_TIM_Base_Start(&htim2);
	spwm_table();

	HAL_TIM_Base_Start(&htim3);
	htim3.Instance->ARR = 84e6 / adc_fs - 1;
	Sign_samplingOver = 0;
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*) adc_cache, adc_cache_size);

	HAL_UART_RxCpltCallback(USART1);
	arm_cfft_radix4_init_f32(&scfft, adc_cache_size, 0, 1);
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	uint8_t disflag = 0;
	singlefreq = 0;
	while (1) {
		char str[20];
//		freq = findfreq();
//		float vpeakpeak = findvpp();
		if (page == 1) {
			switch (mode) {
			case 1:
				if (singlefreq == 0) {
					mode = 0;
					break;
				}
				generateWave(singlefreq / 10);
				HAL_DAC_Start_DMA(&hdac, DAC1_CHANNEL_1, (uint32_t*) spwm,
				SAMPLERATE,
				DAC_ALIGN_12B_R);
				singlefreq = 0;
				mode = 0;
				break;
			case 2:
				uint16_t i;
				for (i = 1; i <= 100; i++) {
					generateWave(i);
					HAL_DAC_Start_DMA(&hdac, DAC1_CHANNEL_1, (uint32_t*) spwm,
					SAMPLERATE,
					DAC_ALIGN_12B_R);
					HAL_Delay(50);
				}
				mode = 0;
				break;
			}
			// SPWM�������?
		}
		if (page == 2) {
			float Au[100];
			uint16_t i;
			for (i = 0; i < 100; i++) {
				generateWave(i + 1);
				HAL_DAC_Start_DMA(&hdac, DAC1_CHANNEL_1, (uint32_t*) spwm,
				SAMPLERATE,
				DAC_ALIGN_12B_R);
				float vmaxget = findvmax();
				Au[i] = findvmax() / (3 / 2.0);
			}
			float freqget = findfreq();
			float maxabs = 0;
			uint8_t st, ed;
			for (i = 0; i < 100; i++) {
				if (Au[i] <= 0.707 && Au[i + 1] > 0.707)
					st = i;
				if (Au[i] > 0.707 && Au[i + 1] <= 0.707)
					ed = i;
				if (Au[i] > maxabs)
					maxabs = Au[i];
			}
			Bandwith = (ed - st) * 10;
			IMinA = maxabs;
			// ��Ƶ��������
			sprintf(str, "x0.val=%d", (int) (Center_freq * 100));
			tjc_send_string(str);
			sprintf(str, "x1.val=%d", (int) (Bandwith * 100));
			tjc_send_string(str);
			sprintf(str, "x2.val=%d", (int) (IMinA * 100));
			tjc_send_string(str);
		}
		if (page == 3) {
			freq = findfreq();
			//			peak_to_peak = FFT_OUTPUT_MAX * 3.3 / 4096;
			float vpp, vmax, vmin;
			uint16_t max_pos, min_pos;
			arm_max_f32(adc_cache, 1024, &vmax, &max_pos); // DSP库自带的求最值函�?
			arm_min_f32(adc_cache, 1024, &vmin, &min_pos);
			vpp = findvpp();
			sprintf(str, "x3.val=%d", (int) (vpp * 100));
			tjc_send_string(str);
			sprintf(str, "x4.val=%d", (int) (freq * 100));
			tjc_send_string(str);
			uint16_t i;
			for (i = 0; i < adc_cache_size; i++) { // ��������ת��
				FFT_INPUT[i * 2] = (float) (adc_cache[i]);
				FFT_INPUT[i * 2 + 1] = 0; // �������?������ݵ���?
			}
			// FFT����
			arm_cfft_f32(&arm_cfft_sR_f32_len1024, FFT_INPUT, 0, 1);
			// ת��Ϊʵ����ȡģ|���� FFT ����ķ���?
			arm_cmplx_mag_f32(FFT_INPUT, FFT_OUTPUT, adc_cache_size);
			// �Ƿ�ȥ��ֱ����������ע���Ǿ���ȥ��
			//		FFT_OUTPUT[0]=0;
			// ȡ�����ֵ����һ�����ͼӳ������
			arm_max_f32(FFT_OUTPUT, adc_cache_size, &FFT_OUTPUT_MAX,
					&FFT_OUTPUT_MAX_index);
			FFT_OUTPUT[0] = 0;
			arm_max_f32(FFT_OUTPUT, adc_cache_size / 2, &FFT_OUTPUT_MAX,
					&FFT_OUTPUT_MAX_index);

			float k = FFT_OUTPUT[FFT_OUTPUT_MAX_index]
					/ FFT_OUTPUT[3 * FFT_OUTPUT_MAX_index];

			if (k < 6 && k > 1) // 方波�? 基波幅度 �?三�?�谐波幅度的 三倍�? 实际测量�?2~4之间�?�?
				waveform = 2;	// 方波

			if (k < 20 && k > 6) // 三�?�波的基波幅度是三�?�谐波幅度的 九倍�?  实际测量�? 6 ~ 12 间浮�?
				waveform = 3;	 // 三�?�波

			if (k > 20)
				waveform = 1; // 正弦�?    //正弦波理论上三�?�谐波幅度是0，k值会非常大�?

			//			int Npoints = adc_fs / freq;
			//			uint16_t i;
			//			for (i = 0; i < Npoints; i++)
			//				adc_cache[i] = adc_cache[i] - vmin - vpp / 2; //将波形平移到与时间轴对称的位�?
			//			arm_rms_f32(adc_cache, Npoints, &RMS); // DSP库封装好的求rms值函�? 不再手动实现 原型�? arm_rms_f32 (const float32_t *pSrc, uint32_t blockSize, float32_t *pResult)
			//			RMS = RMS / (vpp / 2);

			//			判断波类�?
			//			if (RMS > 0.64 && RMS < 0.8)
			//				waveform = 1; //正弦�?
			//			if (RMS > 0.8)
			//				waveform = 2; //方波
			//			if (RMS < 0.63)
			//				waveform = 3; //三�?�波

			// 生成稳定波形
			if (waveform == 1) {
				uint16_t i;
				if (disflag == 0) {
					for (i = 0; i < 420; i++) {
						// 向曲线s0的通道0传输1�?数据,add指令不支持跨页面
						uint16_t sinval = sin_table[i];
						sprintf(str, "add s0.id,0,%d\xff\xff\xff",
								(int) sinval);
						tjc_send_string(str);
					}
					disflag = 1;
				}
			}

			if (waveform == 2) {
				uint16_t i;
				if (disflag == 0) {
					for (i = 0; i < 420; i++) {
						// 向曲线s0的通道0传输1�?数据,add指令不支持跨页面
						uint16_t squareval = square_table[i];
						sprintf(str, "add s0.id,0,%d\xff\xff\xff",
								(int) squareval);
						tjc_send_string(str);
					}
					disflag = 1;
				}
			}

			if (waveform == 3) {
				uint16_t i;
				if (disflag == 0) {
					for (i = 0; i < 420; i++) {
						// 向曲线s0的通道0传输1�?数据,add指令不支持跨页面
						uint16_t triangleval = triangle_table[i];
						sprintf(str, "add s0.id,0,%d\xff\xff\xff",
								(int) triangleval);
						tjc_send_string(str);
					}
					disflag = 1;
				}
			}

			if (page == 0) {
				disflag = 0;
			}
		}
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
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
/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
	/* USER CODE END Error_Handler_Debug */
}

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
