#include "main.h"
#include "adc.h"
#include "dac.h"
#include "dma.h"
#include "tim.h"
#include "gpio.h"
#include "usart.h"
#include "tjc_usart_hmi.h"
#include "stdio.h"

#include "arm_math.h"
#include "arm_const_structs.h"

#define PWM_MAX 100
#define SAMPLES 1000
#define SAMPLERATE 500
#define FRAME_LENGTH 7//数据包大小


uint8_t Sign_samplingOver = 0;//信号采样是否完成
uint8_t Sign_wave_exist = 0;//波形是否存在
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
uint16_t S_readyDisplay = 0;//是否进行幅频特性显示
float Center_freq = 10.3;//中心频率
float Bandwith = 16.5;//带宽
float IMinA = 78.1;//带内最小衰减
int t_freq = 200 ;//信号采样频率
int single_freq = 300;


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
		spwm[i] = pwm[pwmIndex]*4095/100;
	}
}

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

int main(void) {
	HAL_Init();
	SystemClock_Config();
	MX_GPIO_Init();
	MX_DMA_Init();
	MX_ADC1_Init();
	MX_DAC_Init();
	MX_TIM2_Init();
	MX_TIM3_Init();
	MX_USART1_UART_Init();

	HAL_TIM_Base_Start(&htim2);
	HAL_TIM_Base_Start(&htim3);
	htim3.Instance->ARR = 84e6 / adc_fs - 1;
	Sign_samplingOver = 0;
	spwm_table();
	generateWave(200);
	HAL_DAC_Start_DMA(&hdac, DAC1_CHANNEL_1, (uint32_t*) spwm, 50000,
	DAC_ALIGN_12B_R);

	HAL_UART_Receive_IT(&TJC_UART, RxBuffer, 1); 

	while (1) {
		char str[10];
        sprintf(str, "n0.val=%d", single_freq);
		tjc_send_string(str);

		sprintf(str, "x0.val=%d", (int)(Center_freq * 100));
		tjc_send_string(str);
		sprintf(str, "x1.val=%d", (int)(Bandwith * 100));
		tjc_send_string(str);
	    sprintf(str, "x2.val=%d", (int)(IMinA * 100));
		tjc_send_string(str);

	    sprintf(str, "n1.val=%d", t_freq);
		tjc_send_string(str);


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
			continue;
		}
		arm_cfft_f32(&arm_cfft_sR_f32_len1024, FFT_INPUT, 0, 1);
		arm_cmplx_mag_f32(FFT_INPUT, FFT_OUTPUT, adc_cache_size);
		arm_max_f32(FFT_OUTPUT, adc_cache_size, &FFT_OUTPUT_MAX,
				&FFT_OUTPUT_MAX_index);
	}

	return 0;
}
