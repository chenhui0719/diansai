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
//标志位
//（波形绘制准备&本轮ADC-DMA完成）标志位，配合完成波形采样及显示工作
uint8_t Sign_readyDisplay = 0;
uint8_t Sign_samplingOver = 0;
//DAC发生波形切换标志位
uint8_t Sign_dacToggleWave = 0;
//波形显示缩放标志位
uint8_t Sign_waveZoomIn = 0;
uint8_t Sign_waveZoomOut = 0;
//文本刷新标志位，默认为1，即第一次绘图刷新一次文本，将文本显示出来
uint8_t Sign_stringUpdate = 1;
//程序运行标志位，默认为0，为1则程序暂停运行，方便观察波形
uint8_t Sign_stop = 0;
//波形存在标志位,0不存在,1存在
uint8_t Sign_wave_exist = 0;
//程序当前功能{0:波形,1:调频,2:缩放,3:暂停}
uint8_t Sign_function = 0;
//程序当前数据类型{0:基波频率,1:信号幅值}
uint8_t valueData_type = 0;

//当前发生波频率
uint16_t wave_frequency = 1000;

//采样波基频
uint16_t base_frequency = 1000;
//采样波最值及其在采样点中的对应位置{最大值,最小值},{最大值位置,最小值位置}
float wave_maximum[2] = { 3.30, 0.00 };
uint32_t wave_maximum_index[2] = { 0, 0 };
//求解最值专用缓存，求解范围为绘图区域
float wave_maximum_Cache[plot_size] = { 0 };

//两个波，一个时域、一个频域
wave wave_t = { 't', 1, 1, { 0 } };
wave wave_f = { 'f', 2, 1, { 0 } };

//DAC波形选择指针，用来调整DAC当前生成的波形，默认为正弦波
uint16_t *wave_table[4] = { sin_table, square_table, triangle_table,
		sawtooth_table };
uint16_t wave_table_i = 0;

//ADC采样结果缓存
uint16_t adc_cache[adc_cache_size];

//ADC采样频率
//uint32_t adc_fs = 40e3;
uint32_t adc_fs = 51.2e3;

//lcd显示的字符信息
//ADC采样频率信息、当前选中的波形、放大倍数
char *show_string_fs = { "fs:51.2kHz" };
//char* show_string_choice[2] = {"choice:t","choice:f"};
//char* show_string_zoomIn[4] = {"倍数:1","倍数:2","倍数:3","倍数:4"};
//char* show_string_waveType[4] = {"正弦","方波","三角","锯齿"};
//char* show_string_function[4] = {"波形","调频","缩放","暂停"};
char *show_string_zoomIn[4] = { "Z1", "Z2", "Z3", "Z4" };
char *show_string_waveType[4] = { "sine", "square", "triangle", "sawtooth" };
char *show_string_function[5] =
		{ "wave", "genFre", "zoom", "section", "measure" };
char *show_string_stopORrun[2] = { "dynamic", "static" };
char *show_string_measureType[2] = { "baseFre", "voltage" };
//当前波形频率
char show_string_wave_frequency[10] = { "1000Hz" };
//基波频率
char show_string_base_frequency[16] = { "BF:1000Hz" };
//电压最值
char show_string_voltage_maximum[2][16] = { { "Vmax:3.30V" }, { "Vmin:0.00V" } };
//波形缩放(放大)倍数{1,2,3,4}
uint16_t zoomInMultiple = 1;

//时域图像绘图数据缓存
//plot_size|(480)个点，point_i(x,y)对应(draw_t_cache[i][0],draw_t_cache[i][1])
uint16_t draw_t_cache[plot_size][2];

//频域图像绘图数据缓存
//plot_size|(480)个点，point_i(x,y)对应(draw_f_cache[i][0],draw_f_cache[i][1])
uint16_t draw_f_cache[plot_size][2];

//FFT初始化
arm_cfft_radix4_instance_f32 scfft; //FFT结构体
//FFT输入输出
float FFT_INPUT[adc_cache_size * 2]; //对应每个实部创建虚部，所以2倍长
float FFT_OUTPUT[adc_cache_size]; //FFT输出进行了处理，每个float高低16位分别存放实部虚部，无需2倍
float FFT_OUTPUT_MAX = 0;
uint32_t FFT_OUTPUT_MAX_index = 0;

//计数专用i
uint16_t i = 0;
//防闪用图像刷新计数，为0刷新图像
#define CNT_drawUpdate 8
uint16_t i_drawUpdate = 1;

//触发位置
uint16_t i_trigger = 0;
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
	MX_FSMC_Init();
	MX_ADC1_Init();
	MX_DAC_Init();
	MX_TIM2_Init();
	MX_TIM3_Init();
	/* USER CODE BEGIN 2 */

	//开启LCD屏幕
	lcd_init();
	lcd_display_dir(1);

	//开启DAC
	HAL_TIM_Base_Start(&htim2);
	HAL_DAC_Start_DMA(&hdac, DAC1_CHANNEL_1,
			(uint32_t*) wave_table[wave_table_i], N_sampling__,
			DAC_ALIGN_12B_R);

	//开启ADC
	HAL_TIM_Base_Start(&htim3);
	//设置调整ADC采样率
	htim3.Instance->ARR = 84e6 / adc_fs - 1;
	//开始传输下轮数据，标志位置0
	Sign_samplingOver = 0;
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*) adc_cache, adc_cache_size);

	//绘图界面投影网格
	uint16_t delta_x[11] = { 0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100 };
	uint16_t delta_y[11] = { 0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100 };

	//初始化lcd UI
	lcdMaster_InitUI();

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		//准备绘图，先置一个标志位
		Sign_readyDisplay = 1;
		Sign_wave_exist = 0;
		//等待当前轮次ADC-DMA传输完成
		while (!Sign_samplingOver)
			;
		//寻找触发点
		i_trigger = adc_trigger_size; //找不到便显示最新采样结果
		for (i = 0; i < adc_trigger_size; i++) {
			if (adc_cache[i] < 50 && adc_cache[i + 1] > 50) {
				i_trigger = i;
				Sign_wave_exist = 1;
				break;
			}
		}
		if (!Sign_wave_exist) { //波形不存在则重新采样
			Sign_readyDisplay = 0;
			Sign_samplingOver = 0;
			HAL_ADC_Start_DMA(&hadc1, (uint32_t*) adc_cache, adc_cache_size);
			continue;
		}

		//先计算绘图所需参数，再清除上次图像，最后再开始绘图，这样可以尽可能压缩屏幕空白的时间，改善视觉效果
		//此时ADC-DMA处于关闭状态，不必担心缓存数据被新数据覆盖

		//时域
		//求解时域绘图所需点集
		for (i = 0; i < plot_size; i++) {
			wave_t.draw_cache[i][0] = (i) + delta_x[0];
			//0.0268554687 = 110/4096
			wave_t.draw_cache[i][1] = 160 - delta_y[3]
					- (adc_cache[i_trigger + (i)] * 0.0268554687);
		}

		//频域，FFT
		//处理输入数据
		for (i = 0; i < adc_cache_size; i++) {		//采样数据转换
			FFT_INPUT[i * 2] = (float) (adc_cache[i]);
			FFT_INPUT[i * 2 + 1] = 0;		//交替插零，添加数据的虚部
		}
		//FFT运算
		arm_cfft_f32(&arm_cfft_sR_f32_len1024, FFT_INPUT, 0, 1);
		//转化为实数并取模|计算 FFT 输出的幅度
		arm_cmplx_mag_f32(FFT_INPUT, FFT_OUTPUT, adc_cache_size);
		//是否去除直流分量，不注释那就是去掉
//		FFT_OUTPUT[0]=0;
		//取出最大值方便一会儿绘图映射坐标
		arm_max_f32(FFT_OUTPUT, adc_cache_size, &FFT_OUTPUT_MAX,
				&FFT_OUTPUT_MAX_index);
		FFT_OUTPUT[0] = 0;
		arm_max_f32(FFT_OUTPUT, adc_cache_size / 2, &FFT_OUTPUT_MAX,
				&FFT_OUTPUT_MAX_index);
		base_frequency = adc_fs * FFT_OUTPUT_MAX_index / 1024;
		//求解频域绘图所需点集
		for (i = 0; i < plot_size; i++) {
			wave_f.draw_cache[i][0] = (i) + delta_x[0];
			//**映射系数1.06875 = (512+1)/480
			//映射，完全绘出FFT结果
//			wave_f.draw_cache[i][1] = (uint16_t)(320-delta_y[5]-(FFT_OUTPUT[(uint16_t)(1.06875*i)]*110/FFT_OUTPUT_MAX));
			//不映射，绘制大部分FFT结果
			wave_f.draw_cache[i][1] = (uint16_t) (320 - delta_y[5]
					- (FFT_OUTPUT[(uint16_t) (i)] * 110 / FFT_OUTPUT_MAX));
//			draw_f_cache[i][1] = (uint16_t)(320-(FFT_OUTPUT[(uint16_t)(0.51*i)]*110/FFT_OUTPUT_MAX+delta_y[1]));
		}

		switch (valueData_type) {			//判断动态刷新的数值类型{基波频率，波形最值}
		case 0:
			//去除直流分量再求一次最值得到基波频率
			FFT_OUTPUT[0] = 0;
			arm_max_f32(FFT_OUTPUT, adc_cache_size / 2, &FFT_OUTPUT_MAX,
					&FFT_OUTPUT_MAX_index);
			base_frequency = adc_fs * FFT_OUTPUT_MAX_index / 1024;
			//得到基波频率字符串
			sprintf(show_string_base_frequency, "BF:%dHz", base_frequency);
			break;
		case 1:
			//为了简化运算量，不使用原始数据，使用绘图数据
			for (i = 0; i < plot_size; i++) {
				wave_maximum_Cache[i] = (float) adc_cache[i_trigger + i];
			}
			arm_max_f32(wave_maximum_Cache, plot_size, &wave_maximum[0],
					&wave_maximum_index[0]);
			arm_min_f32(wave_maximum_Cache, plot_size, &wave_maximum[1],
					&wave_maximum_index[1]);
			sprintf(show_string_voltage_maximum[0], "Vmax:%.3fV",
					wave_maximum[0] * 3.3 / 4095);
			sprintf(show_string_voltage_maximum[1], "Vmin:%.3fV",
					wave_maximum[1] * 3.3 / 4095);
			break;
		default:
			break;
		}

		//开绘~
		//横屏绘制 x_MAX = 480 & y_MAX = 320
		i_drawUpdate--;
		if (i_drawUpdate == 0) {		//慢速绘图，多次刷新数据，只绘制一帧
			i_drawUpdate = CNT_drawUpdate;
			//先清屏再绘图，为了良好的视觉体验，采用局部清屏，刷新哪些内容，就清哪部分
			//利用adc_cache[adc_trigger_size+i]绘图
			//绘图暂停位，为1则不再绘图，以此制作波形切片。
			if (!Sign_stop) {
				//首先绘制波形图
				//清除时域图像并重新绘制
				lcdMaster_Clear(lcdRoom_wave_t);
				for (i = 0; i < plot_size / zoomInMultiple - 1; i++) {
					lcd_draw_line(wave_t.draw_cache[i * zoomInMultiple][0],
							wave_t.draw_cache[i][1],
							wave_t.draw_cache[(i + 1) * zoomInMultiple][0],
							wave_t.draw_cache[i + 1][1], RED);
				}
				//清除频域图像并重新绘制
				lcdMaster_Clear(lcdRoom_wave_f);
				for (i = 0; i < plot_size / zoomInMultiple - 1; i++) {
					lcd_draw_line(wave_f.draw_cache[i * zoomInMultiple][0],
							wave_f.draw_cache[i][1],
							wave_f.draw_cache[(i + 1) * zoomInMultiple][0],
							wave_f.draw_cache[i + 1][1], RED);
				}
				//刷新动态显示的数值信息，切片时不再刷新
				switch (valueData_type) {
				case 0:
					//基波频率
					lcdMaster_Clear(lcdRoom_bf);
					lcd_show_string(362, 320 - delta_y[3], 110, 24, 24,
							show_string_base_frequency, DARK_YELLOW);
					break;
				case 1:
					//信号幅值
					lcdMaster_Clear(lcdRoom_bf);
					lcd_show_string(362, 320 - 33, 120, 16, 16,
							show_string_voltage_maximum[0], DARK_YELLOW);
					lcd_show_string(362, 320 - 17, 120, 16, 16,
							show_string_voltage_maximum[1], DARK_YELLOW);
					break;
				}
			}

			//选择性刷新各种信息
			if (Sign_stringUpdate) {
				lcdMaster_Clear(lcdRoom_string);
				//两边各距边缘6，480-6*2=468，468/3=156(实际上，我看着屏幕调的大致位置)
				//当前功能
				lcd_show_string(10, 320 - delta_y[3], 100, 24, 24,
						show_string_function[Sign_function], DARK_YELLOW);
				//功能参数
				switch (Sign_function) {
				case 0:				//当前波形
					lcd_show_string(120, 320 - delta_y[3], 120, 24, 24,
							show_string_waveType[wave_table_i], DARK_YELLOW);
					break;
				case 1:				//当前波形频率
					lcd_show_string(120, 320 - delta_y[3], 120, 24, 24,
							show_string_wave_frequency, DARK_YELLOW);
					break;
				case 2:				//当前波形的缩放倍数
					lcd_show_string(120, 320 - delta_y[3], 120, 24, 24,
							show_string_zoomIn[zoomInMultiple - 1],
							DARK_YELLOW);
					break;
				case 3:				//停止或运行
					lcd_show_string(120, 320 - delta_y[3], 120, 24, 24,
							show_string_stopORrun[Sign_stop], DARK_YELLOW);
					break;
				case 4:				//显示的数据类型{基波频率,电压}
					lcd_show_string(120, 320 - delta_y[3], 120, 24, 24,
							show_string_measureType[valueData_type],
							DARK_YELLOW);
					break;
				default:
					break;
				}
				//采样频率
				lcd_show_string(230, 320 - delta_y[3], 110, 24, 24,
						show_string_fs, DARK_YELLOW);
				//信息已刷新，复位标志位
				Sign_stringUpdate = 0;
			}
		}

		//绘图完成，复位标志位，重新开启ADC-DMA
		Sign_readyDisplay = 0;
		Sign_samplingOver = 0;
		HAL_ADC_Start_DMA(&hadc1, (uint32_t*) adc_cache, adc_cache_size);

		//DAC波形是否需要切换
		if (Sign_dacToggleWave == 1) {				//需要
			//诶，我1ms都不消，就是贪
//    		//延迟10ms消抖
//    		HAL_Delay(10);
			//刷新DAC产生的波形
			HAL_DAC_Stop_DMA(&hdac, DAC1_CHANNEL_1);
			HAL_DAC_Start_DMA(&hdac, DAC1_CHANNEL_1,
					(uint32_t*) wave_table[wave_table_i], N_sampling__,
					DAC_ALIGN_12B_R);
			Sign_dacToggleWave = 0;    		//复位标志位
			HAL_NVIC_EnableIRQ(KEY_ToggleWaveTable_EXTI_IRQn);    	//再次开启切换波形中断
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

#ifdef  USE_FULL_ASSERT
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
