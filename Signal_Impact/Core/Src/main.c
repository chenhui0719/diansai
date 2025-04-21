#include "main.h"
#include "adc.h"
#include "dac.h"
#include "dma.h"
#include "tim.h"
#include "gpio.h"

#include "arm_math.h"
#include "arm_const_structs.h"

const uint32_t pwm[50] = {1, 2, 5, 8, 11, 16, 21, 26, 32, 38, 44, 50, 56, 62,
                          68, 74, 79, 84, 89, 92, 95, 98, 99, 100, 100, 99, 98, 95, 92, 89, 84,
                          79, 74, 68, 62, 56, 50, 44, 38, 32, 26, 21, 16, 11, 8, 5, 2, 1, 0, 0};
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

const uint16_t sampleRate = 1000; // 假设采样率为10kHz
uint16_t spwm[1000] = {0};
void generateWave(uint16_t freq)
{
    const uint16_t period = sampleRate / freq;
    if (period == 0)
        Error_Handler(); // 防止除零错误
    uint16_t i;
    for (i = 0; i < sampleRate; ++i)
    {
        uint16_t pwmIndex = (i % period) * (sizeof(pwm) / sizeof(pwm[0]) - 1) / period;
        if (pwmIndex >= sizeof(pwm) / sizeof(pwm[0]))
            Error_Handler(); // 防止越界
        spwm[i] = pwm[pwmIndex] / 100 * 4096;
    }
    // 算一个周期的采样点数
}

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

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_DMA_Init();
    MX_ADC1_Init();
    MX_DAC_Init();
    MX_TIM2_Init();
    MX_TIM3_Init();

    HAL_TIM_Base_Start(&htim2);
    HAL_TIM_Base_Start(&htim3);
    htim3.Instance->ARR = 84e6 / adc_fs - 1;
    Sign_samplingOver = 0;

    generateWave(200);
    while (1)
        HAL_DAC_Start_DMA(&hdac, DAC1_CHANNEL_1, (uint32_t *)spwm, 1000,
                          DAC_ALIGN_12B_R);

    while (1)
    {
        Sign_wave_exist = 0;
        while (!Sign_samplingOver)
            ;
        i_trigger = adc_trigger_size;
        uint8_t i;
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
        {
            Sign_samplingOver = 0;
            HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_cache, adc_cache_size);
            continue;
        }
        arm_cfft_f32(&arm_cfft_sR_f32_len1024, FFT_INPUT, 0, 1);
        arm_cmplx_mag_f32(FFT_INPUT, FFT_OUTPUT, adc_cache_size);
        arm_max_f32(FFT_OUTPUT, adc_cache_size, &FFT_OUTPUT_MAX,
                    &FFT_OUTPUT_MAX_index);
    }

    return 0;
}
