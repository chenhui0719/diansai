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
#include "dma.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "tjc_usart_hmi.h"
#define FRAME_LENGTH 7
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* 锟斤拷瑜般��锟斤拷��锟藉������锟�? */
#define RING_BUFFER_SIZE     256

/* �癸拷娑�锟斤拷锟借ぐ��锟斤拷��锟借�癸拷锟斤拷娴ｏ拷 */
typedef struct {
    uint8_t buffer[RING_BUFFER_SIZE];
    uint16_t head;
    uint16_t tail;
} RingBuffer;

/* 锟斤拷瑜般��锟斤拷��锟藉��锟斤拷��锟�?锟斤拷锟斤拷 */
RingBuffer rx_ring_buffer;

/* PRIVATE FUNCTIONS */
void initRingBuffer(void);
void ringBufferAdd(uint8_t data);
uint8_t ringBufferGet(void);
uint16_t ringBufferSize(void);

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
uint16_t S_readyDisplay = 0;//锟斤拷锟斤拷�╋拷��锟介��锟芥０锟斤拷瑙�锟窖�锟藉�с��
float Center_freq = 10.3;//娑�锟借�锟芥０锟斤拷锟�
float Bandwith = 16.5;//��锟界�癸拷
float IMinA = 78.1;//��锟斤拷锟斤拷锟界��锟界���匡拷锟�
float t_freq = 200.0 ;//娣��筹拷�斤拷锟斤拷�斤拷锟斤拷
float single_freq = 300.0;

/* 锟斤拷瑜般��锟斤拷��锟藉��锟斤拷婵�锟斤拷锟� */
void initRingBuffer(void) {
    rx_ring_buffer.head = 0;
    rx_ring_buffer.tail = 0;
}

/* 锟斤拷锟斤拷瑜般��锟斤拷��锟界����锟斤拷锟界��锟斤拷 */
void ringBufferAdd(uint8_t data) {
    uint16_t next_head = (rx_ring_buffer.head + 1) % RING_BUFFER_SIZE;

    /* 婵★拷锟斤拷缂�锟斤拷��锟藉���″���锟斤拷锟界��锟斤拷锟斤拷�锟界��锟斤拷 */
    if (next_head == rx_ring_buffer.tail) {
        /* 瑜帮拷缂�锟斤拷��锟界��寮э拷璁癸拷缁�璇诧拷�ョ��锟斤拷锟斤拷娴���锟斤拷锟斤拷�锟界��锟斤拷 */
        rx_ring_buffer.tail = (rx_ring_buffer.tail + 1) % RING_BUFFER_SIZE;
    }

    rx_ring_buffer.buffer[rx_ring_buffer.head] = data;
    rx_ring_buffer.head = next_head;
}

/* 娴�锟斤拷锟借ぐ��锟斤拷��锟介缚锟借�诧拷锟斤拷��锟斤拷 */
uint8_t ringBufferGet(void) {
    if (rx_ring_buffer.tail == rx_ring_buffer.head) {
        /* 缂�锟斤拷��锟借桨璐���锟�? */
        return 0;
    }

    uint8_t data = rx_ring_buffer.buffer[rx_ring_buffer.tail];
    rx_ring_buffer.tail = (rx_ring_buffer.tail + 1) % RING_BUFFER_SIZE;
    return data;
}

/* 锟藉嘲锟斤拷锟斤拷瑜般��锟斤拷��锟借桨��锟斤拷锟界��锟斤拷锟藉府��? */
uint16_t ringBufferSize(void) {
    return (RING_BUFFER_SIZE + rx_ring_buffer.head - rx_ring_buffer.tail) % RING_BUFFER_SIZE;
}

/* UART锟姐��锟芥�佃��锟斤拷锟斤拷��锟斤拷�ワ拷锟� */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART1) {
        uint8_t received_data = RxBuffer[0];
        ringBufferAdd(received_data);  // ��锟斤拷��锟藉��锟斤拷锟界��锟斤拷濞ｈ�诧拷锟斤拷��锟斤拷瑜般��锟斤拷��锟斤拷
        HAL_UART_Receive_IT(&TJC_UART, RxBuffer, 1); // 锟斤拷锟芥�匡拷锟斤拷��锟姐��锟芥�佃��锟斤拷
    }
}

/* 婢讹拷锟斤拷锟姐��锟借泛锟芥��锟斤拷锟界��锟斤拷 */
void processReceivedData(void) {
    // 锟姐��锟斤拷锟斤拷锟芥�����锟斤拷婢讹拷锟斤拷锟姐��锟借泛锟芥��锟斤拷锟界��锟斤拷锟斤拷娴�锝�锟斤拷
    // 娓�锟芥俊锟介��锟�?锟姐�э拷锟芥��锟借桨��锟斤拷锟界��锟斤拷楠��帮拷��锟斤拷绋匡拷婢讹拷��锟�?
}

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
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
	initRingBuffer();                            // 锟斤拷婵�锟斤拷锟斤拷锟借ぐ��锟斤拷��锟斤拷
	HAL_UART_Receive_IT(&TJC_UART, RxBuffer, 1); // 锟斤拷缂�锟芥�锟斤拷锟斤拷��锟斤拷
	int a = 100;
	char str[100];
	//	uint32_t nowtime = HAL_GetTick();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1) {

		sprintf(str, "x3.val=%d", (int)(single_freq*100));
		tjc_send_string(str);

		sprintf(str, "x0.val=%d", (int)(Center_freq * 100));
		tjc_send_string(str);
		sprintf(str, "x1.val=%d", (int)(Bandwith * 100));
		tjc_send_string(str);
	    sprintf(str, "x2.val=%d", (int)(IMinA * 100));
		tjc_send_string(str);

	    sprintf(str, "x4.val=%d", (int)(t_freq*100));
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
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
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
