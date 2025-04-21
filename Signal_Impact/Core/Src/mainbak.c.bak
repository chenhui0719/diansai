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
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dac.h"
#include "dma.h"
#include "tim.h"
#include "gpio.h"
#include "fsmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "../../Drivers/BSP/LCD/lcd.h"

#include "waveform_table.h"
#include "arm_math.h"
#include "arm_const_structs.h"
#include "lcdMaster.h"
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

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// ��־λ
// �����λ���׼��&����ADC-DMA��ɣ���־λ�������ɲ��β�������ʾ����
uint8_t Sign_readyDisplay = 0;
uint8_t Sign_samplingOver = 0;
// DAC���������л���־λ
uint8_t Sign_dacToggleWave = 0;
// ������ʾ���ű�־λ
uint8_t Sign_waveZoomIn = 0;
uint8_t Sign_waveZoomOut = 0;
// �ı�ˢ�±�־λ��Ĭ��Ϊ1������һ�λ�ͼˢ��һ���ı������ı���ʾ����
uint8_t Sign_stringUpdate = 1;
// �������б�־λ��Ĭ��Ϊ0��Ϊ1�������ͣ���У�����۲첨��
uint8_t Sign_stop = 0;
// ���δ��ڱ�־λ,0������,1����
uint8_t Sign_wave_exist = 0;
// ����ǰ����{0:����,1:��Ƶ,2:����,3:��ͣ}
uint8_t Sign_function = 0;
// ����ǰ��������{0:����Ƶ��,1:�źŷ�ֵ}
uint8_t valueData_type = 0;

// ��ǰ������Ƶ��
uint16_t wave_frequency = 1000;

// ��������Ƶ
uint16_t base_frequency = 1000;
// ��������ֵ�����ڲ������еĶ�Ӧλ��{���ֵ,��Сֵ},{���ֵλ��,��Сֵλ��}
float wave_maximum[2] = {3.30, 0.00};
uint32_t wave_maximum_index[2] = {0, 0};
// �����ֵר�û��棬��ⷶΧΪ��ͼ����
float wave_maximum_Cache[plot_size] = {0};

// ��������һ��ʱ��һ��Ƶ��
wave wave_t = {'t', 1, 1, {0}};
wave wave_f = {'f', 2, 1, {0}};

// DAC����ѡ��ָ�룬��������DAC��ǰ���ɵĲ��Σ�Ĭ��Ϊ���Ҳ�
uint16_t *wave_table[4] = {sin_table, square_table, triangle_table, sawtooth_table};
uint16_t wave_table_i = 0;

// ADC�����������
uint16_t adc_cache[adc_cache_size];

// ADC����Ƶ��
// uint32_t adc_fs = 40e3;
uint32_t adc_fs = 51.2e3;

// lcd��ʾ���ַ���Ϣ
// ADC����Ƶ����Ϣ����ǰѡ�еĲ��Ρ��Ŵ���
char *show_string_fs = {"fs:51.2kHz"};
// char* show_string_choice[2] = {"choice:t","choice:f"};
// char* show_string_zoomIn[4] = {"����:1","����:2","����:3","����:4"};
// char* show_string_waveType[4] = {"����","����","����","���"};
// char* show_string_function[4] = {"����","��Ƶ","����","��ͣ"};
char *show_string_zoomIn[4] = {"Z1", "Z2", "Z3", "Z4"};
char *show_string_waveType[4] = {"sine", "square", "triangle", "sawtooth"};
char *show_string_function[5] = {"wave", "genFre", "zoom", "section", "measure"};
char *show_string_stopORrun[2] = {"dynamic", "static"};
char *show_string_measureType[2] = {"baseFre", "voltage"};
// ��ǰ����Ƶ��
char show_string_wave_frequency[10] = {"1000Hz"};
// ����Ƶ��
char show_string_base_frequency[16] = {"BF:1000Hz"};
// ��ѹ��ֵ
char show_string_voltage_maximum[2][16] = {{"Vmax:3.30V"}, {"Vmin:0.00V"}};
// ��������(�Ŵ�)����{1,2,3,4}
uint16_t zoomInMultiple = 1;

// ʱ��ͼ���ͼ���ݻ���
// plot_size|(480)���㣬point_i(x,y)��Ӧ(draw_t_cache[i][0],draw_t_cache[i][1])
uint16_t draw_t_cache[plot_size][2];

// Ƶ��ͼ���ͼ���ݻ���
// plot_size|(480)���㣬point_i(x,y)��Ӧ(draw_f_cache[i][0],draw_f_cache[i][1])
uint16_t draw_f_cache[plot_size][2];

// FFT��ʼ��
arm_cfft_radix4_instance_f32 scfft; // FFT�ṹ��
// FFT�������
float FFT_INPUT[adc_cache_size * 2]; // ��Ӧÿ��ʵ�������鲿������2����
float FFT_OUTPUT[adc_cache_size];	 // FFT��������˴�����ÿ��float�ߵ�16λ�ֱ���ʵ���鲿������2��
float FFT_OUTPUT_MAX = 0;
uint32_t FFT_OUTPUT_MAX_index = 0;

// ����ר��i
uint16_t i = 0;
// ������ͼ��ˢ�¼�����Ϊ0ˢ��ͼ��
#define CNT_drawUpdate 8
uint16_t i_drawUpdate = 1;

// ����λ��
uint16_t i_trigger = 0;
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
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
	MX_FSMC_Init();
	MX_ADC1_Init();
	MX_DAC_Init();
	MX_TIM2_Init();
	MX_TIM3_Init();
	/* USER CODE BEGIN 2 */

	// ����LCD��Ļ
	lcd_init();
	lcd_display_dir(1);

	// ����DAC
	HAL_TIM_Base_Start(&htim2);
	HAL_DAC_Start_DMA(&hdac, DAC1_CHANNEL_1, (uint32_t *)wave_table[wave_table_i], N_sampling__, DAC_ALIGN_12B_R);

	// ����ADC
	HAL_TIM_Base_Start(&htim3);
	// ���õ���ADC������
	htim3.Instance->ARR = 84e6 / adc_fs - 1;
	// ��ʼ�����������ݣ���־λ��0
	Sign_samplingOver = 0;
	HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_cache, adc_cache_size);

	// ��ͼ����ͶӰ����
	uint16_t delta_x[11] = {0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
	uint16_t delta_y[11] = {0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

	// ��ʼ��lcd UI
	lcdMaster_InitUI();

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1)
	{
		// ׼����ͼ������һ����־λ
		Sign_readyDisplay = 1;
		Sign_wave_exist = 0;
		// �ȴ���ǰ�ִ�ADC-DMA�������
		while (!Sign_samplingOver)
			;
		// Ѱ�Ҵ�����
		i_trigger = adc_trigger_size; // �Ҳ�������ʾ���²������
		for (i = 0; i < adc_trigger_size; i++)
		{
			if (adc_cache[i] < 50 && adc_cache[i + 1] > 50)
			{
				i_trigger = i;
				Sign_wave_exist = 1;
				break;
			}
		}
		if (!Sign_wave_exist)
		{ // ���β����������²���
			Sign_readyDisplay = 0;
			Sign_samplingOver = 0;
			HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_cache, adc_cache_size);
			continue;
		}

		// �ȼ����ͼ���������������ϴ�ͼ������ٿ�ʼ��ͼ���������Ծ�����ѹ����Ļ�հ׵�ʱ�䣬�����Ӿ�Ч��
		// ��ʱADC-DMA���ڹر�״̬�����ص��Ļ������ݱ������ݸ���

		// ʱ��
		// ���ʱ���ͼ����㼯
		for (i = 0; i < plot_size; i++)
		{
			wave_t.draw_cache[i][0] = (i) + delta_x[0];
			// 0.0268554687 = 110/4096
			wave_t.draw_cache[i][1] = 160 - delta_y[3] - (adc_cache[i_trigger + (i)] * 0.0268554687);
		}

		// Ƶ��FFT
		// ������������
		for (i = 0; i < adc_cache_size; i++)
		{ // ��������ת��
			FFT_INPUT[i * 2] = (float)(adc_cache[i]);
			FFT_INPUT[i * 2 + 1] = 0; // ������㣬�������ݵ��鲿
		}
		// FFT����
		arm_cfft_f32(&arm_cfft_sR_f32_len1024, FFT_INPUT, 0, 1);
		// ת��Ϊʵ����ȡģ|���� FFT ����ķ���
		arm_cmplx_mag_f32(FFT_INPUT, FFT_OUTPUT, adc_cache_size);
		// �Ƿ�ȥ��ֱ����������ע���Ǿ���ȥ��
		//		FFT_OUTPUT[0]=0;
		// ȡ�����ֵ����һ�����ͼӳ������
		arm_max_f32(FFT_OUTPUT, adc_cache_size, &FFT_OUTPUT_MAX, &FFT_OUTPUT_MAX_index);

		// ���Ƶ���ͼ����㼯
		for (i = 0; i < plot_size; i++)
		{
			wave_f.draw_cache[i][0] = (i) + delta_x[0];
			//**ӳ��ϵ��1.06875 = (512+1)/480
			// ӳ�䣬��ȫ���FFT���
			//			wave_f.draw_cache[i][1] = (uint16_t)(320-delta_y[5]-(FFT_OUTPUT[(uint16_t)(1.06875*i)]*110/FFT_OUTPUT_MAX));
			// ��ӳ�䣬���ƴ󲿷�FFT���
			wave_f.draw_cache[i][1] = (uint16_t)(320 - delta_y[5] - (FFT_OUTPUT[(uint16_t)(i)] * 110 / FFT_OUTPUT_MAX));
			//			draw_f_cache[i][1] = (uint16_t)(320-(FFT_OUTPUT[(uint16_t)(0.51*i)]*110/FFT_OUTPUT_MAX+delta_y[1]));
		}

		switch (valueData_type)
		{ // �ж϶�̬ˢ�µ���ֵ����{����Ƶ�ʣ�������ֵ}
		case 0:
			// ȥ��ֱ����������һ����ֵ�õ�����Ƶ��
			FFT_OUTPUT[0] = 0;
			arm_max_f32(FFT_OUTPUT, adc_cache_size / 2, &FFT_OUTPUT_MAX, &FFT_OUTPUT_MAX_index);
			base_frequency = adc_fs * FFT_OUTPUT_MAX_index / 1024;
			// �õ�����Ƶ���ַ���
			sprintf(show_string_base_frequency, "BF:%dHz", base_frequency);
			break;
		case 1:
			// Ϊ�˼�����������ʹ��ԭʼ���ݣ�ʹ�û�ͼ����
			for (i = 0; i < plot_size; i++)
			{
				wave_maximum_Cache[i] = (float)adc_cache[i_trigger + i];
			}
			arm_max_f32(wave_maximum_Cache, plot_size, &wave_maximum[0], &wave_maximum_index[0]);
			arm_min_f32(wave_maximum_Cache, plot_size, &wave_maximum[1], &wave_maximum_index[1]);
			sprintf(show_string_voltage_maximum[0], "Vmax:%.3fV", wave_maximum[0] * 3.3 / 4095);
			sprintf(show_string_voltage_maximum[1], "Vmin:%.3fV", wave_maximum[1] * 3.3 / 4095);
			break;
		default:
			break;
		}

		// ����~
		// �������� x_MAX = 480 & y_MAX = 320
		i_drawUpdate--;
		if (i_drawUpdate == 0)
		{ // ���ٻ�ͼ�����ˢ�����ݣ�ֻ����һ֡
			i_drawUpdate = CNT_drawUpdate;
			// �������ٻ�ͼ��Ϊ�����õ��Ӿ����飬���þֲ�������ˢ����Щ���ݣ������Ĳ���
			// ����adc_cache[adc_trigger_size+i]��ͼ
			// ��ͼ��ͣλ��Ϊ1���ٻ�ͼ���Դ�����������Ƭ��
			if (!Sign_stop)
			{
				// ���Ȼ��Ʋ���ͼ
				// ���ʱ��ͼ�����»���
				lcdMaster_Clear(lcdRoom_wave_t);
				for (i = 0; i < plot_size / zoomInMultiple - 1; i++)
				{
					lcd_draw_line(wave_t.draw_cache[i * zoomInMultiple][0], wave_t.draw_cache[i][1],
								  wave_t.draw_cache[(i + 1) * zoomInMultiple][0], wave_t.draw_cache[i + 1][1], RED);
				}
				// ���Ƶ��ͼ�����»���
				lcdMaster_Clear(lcdRoom_wave_f);
				for (i = 0; i < plot_size / zoomInMultiple - 1; i++)
				{
					lcd_draw_line(wave_f.draw_cache[i * zoomInMultiple][0], wave_f.draw_cache[i][1],
								  wave_f.draw_cache[(i + 1) * zoomInMultiple][0], wave_f.draw_cache[i + 1][1], RED);
				}
				// ˢ�¶�̬��ʾ����ֵ��Ϣ����Ƭʱ����ˢ��
				switch (valueData_type)
				{
				case 0:
					// ����Ƶ��
					lcdMaster_Clear(lcdRoom_bf);
					lcd_show_string(362, 320 - delta_y[3], 110, 24, 24, show_string_base_frequency, DARK_YELLOW);
					break;
				case 1:
					// �źŷ�ֵ
					lcdMaster_Clear(lcdRoom_bf);
					lcd_show_string(362, 320 - 33, 120, 16, 16, show_string_voltage_maximum[0], DARK_YELLOW);
					lcd_show_string(362, 320 - 17, 120, 16, 16, show_string_voltage_maximum[1], DARK_YELLOW);
					break;
				}
			}

			// ѡ����ˢ�¸�����Ϣ
			if (Sign_stringUpdate)
			{
				lcdMaster_Clear(lcdRoom_string);
				// ���߸����Ե6��480-6*2=468��468/3=156(ʵ���ϣ��ҿ�����Ļ���Ĵ���λ��)
				// ��ǰ����
				lcd_show_string(10, 320 - delta_y[3], 100, 24, 24, show_string_function[Sign_function], DARK_YELLOW);
				// ���ܲ���
				switch (Sign_function)
				{
				case 0: // ��ǰ����
					lcd_show_string(120, 320 - delta_y[3], 120, 24, 24, show_string_waveType[wave_table_i], DARK_YELLOW);
					break;
				case 1: // ��ǰ����Ƶ��
					lcd_show_string(120, 320 - delta_y[3], 120, 24, 24, show_string_wave_frequency, DARK_YELLOW);
					break;
				case 2: // ��ǰ���ε����ű���
					lcd_show_string(120, 320 - delta_y[3], 120, 24, 24, show_string_zoomIn[zoomInMultiple - 1], DARK_YELLOW);
					break;
				case 3: // ֹͣ������
					lcd_show_string(120, 320 - delta_y[3], 120, 24, 24, show_string_stopORrun[Sign_stop], DARK_YELLOW);
					break;
				case 4: // ��ʾ����������{����Ƶ��,��ѹ}
					lcd_show_string(120, 320 - delta_y[3], 120, 24, 24, show_string_measureType[valueData_type], DARK_YELLOW);
					break;
				default:
					break;
				}
				// ����Ƶ��
				lcd_show_string(230, 320 - delta_y[3], 110, 24, 24, show_string_fs, DARK_YELLOW);
				// ��Ϣ��ˢ�£���λ��־λ
				Sign_stringUpdate = 0;
			}
		}

		// ��ͼ��ɣ���λ��־λ�����¿���ADC-DMA
		Sign_readyDisplay = 0;
		Sign_samplingOver = 0;
		HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_cache, adc_cache_size);

		// DAC�����Ƿ���Ҫ�л�
		if (Sign_dacToggleWave == 1)
		{ // ��Ҫ
			// ������1ms������������̰
			//    		//�ӳ�10ms����
			//    		HAL_Delay(10);
			// ˢ��DAC�����Ĳ���
			HAL_DAC_Stop_DMA(&hdac, DAC1_CHANNEL_1);
			HAL_DAC_Start_DMA(&hdac, DAC1_CHANNEL_1, (uint32_t *)wave_table[wave_table_i], N_sampling__, DAC_ALIGN_12B_R);
			Sign_dacToggleWave = 0;							   // ��λ��־λ
			HAL_NVIC_EnableIRQ(KEY_ToggleWaveTable_EXTI_IRQn); // �ٴο����л������ж�
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
void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

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
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
	{
		Error_Handler();
	}
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1)
	{
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
