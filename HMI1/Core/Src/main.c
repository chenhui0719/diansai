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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "tjc_usart_hmi.h"
#define FRAME_LENGTH 7
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* 环形缓冲区大小 */
#define RING_BUFFER_SIZE     256

/* 定义环形缓冲区结构体 */
typedef struct {
    uint8_t buffer[RING_BUFFER_SIZE];
    uint16_t head;
    uint16_t tail;
} RingBuffer;

/* 环形缓冲区全局变量 */
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
UART_HandleTypeDef huart1;

/* 用户代码区 */
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);

/* HAL用户代码区 */
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* 用户代码区 */
/* USER CODE BEGIN 0 */

/* 环形缓冲区初始化 */
void initRingBuffer(void) {
    rx_ring_buffer.head = 0;
    rx_ring_buffer.tail = 0;
}

/* 向环形缓冲区添加数据 */
void ringBufferAdd(uint8_t data) {
    uint16_t next_head = (rx_ring_buffer.head + 1) % RING_BUFFER_SIZE;

    /* 如果缓冲区已满，则覆盖旧数据 */
    if (next_head == rx_ring_buffer.tail) {
        /* 当缓冲区满时，移动尾指针以覆盖旧数据 */
        rx_ring_buffer.tail = (rx_ring_buffer.tail + 1) % RING_BUFFER_SIZE;
    }

    rx_ring_buffer.buffer[rx_ring_buffer.head] = data;
    rx_ring_buffer.head = next_head;
}

/* 从环形缓冲区读取数据 */
uint8_t ringBufferGet(void) {
    if (rx_ring_buffer.tail == rx_ring_buffer.head) {
        /* 缓冲区为空 */
        return 0;
    }

    uint8_t data = rx_ring_buffer.buffer[rx_ring_buffer.tail];
    rx_ring_buffer.tail = (rx_ring_buffer.tail + 1) % RING_BUFFER_SIZE;
    return data;
}

/* 获取环形缓冲区中的数据数量 */
uint16_t ringBufferSize(void) {
    return (RING_BUFFER_SIZE + rx_ring_buffer.head - rx_ring_buffer.tail) % RING_BUFFER_SIZE;
}

/* UART接收中断回调函数 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART1) {
        uint8_t received_data = RxBuffer[0];
        ringBufferAdd(received_data);  // 将接收的数据添加到环形缓冲区
        HAL_UART_Receive_IT(&TJC_UART, RxBuffer, 1); // 重新启动接收中断
    }
}

/* 处理接收到的数据 */
void processReceivedData(void) {
    // 在这里可以添加处理接收到的数据的代码
    // 例如检查缓冲区中的数据并进行相应处理
}

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
	MX_USART1_UART_Init();
	/* USER CODE BEGIN 2 */
	initRingBuffer();                            // 初始化环形缓冲区
	HAL_UART_Receive_IT(&TJC_UART, RxBuffer, 1); // 配置中断接收
	int a = 100;
	char str[100];
	//	uint32_t nowtime = HAL_GetTick();
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {

		sprintf(str, "n0.val=%d", a);
		tjc_send_string(str);
		HAL_Delay(100);
		//			sprintf(str, "t0.txt=\"%d\"", a);
		//			tjc_send_string(str);
		//			sprintf(str, "click b0,1");
		//			tjc_send_string(str);
		//			HAL_Delay(50);
		//			sprintf(str, "click b0,0");
		//			tjc_send_string(str);


		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
		//
//				while (usize >= FRAME_LENGTH) {
//					// 乌鸦�֡ͷ֡β�Ƿ� Holl��
//					if (usize >= FRAME_LENGTH && u(0) == 0x55 && u(4) == 0xff
//							&& u(5) == 0xff && u(6) == 0xff) {
//						//  Holl�䣬���н���
//						if (u(1) == 0x01) {
//							sprintf(str, "msg.txt=\"LED %d is %s\"", u(2),
//							u(3) ? "on" : "off");
//							tjc_send_string(str);
//						} else if (u(1) == 0x02) {
//							// �·�����h0����������Ϣ
//							sprintf(str, "msg.txt=\"h0.val is %d\"", u(2));
//							tjc_send_string(str);
//						} else if (u(1) == 0x03) {
//							// �·�����h1����������Ϣ
//							sprintf(str, "msg.txt=\"h1.val is %d\"", u(2));
//							tjc_send_string(str);
//						}
//
//						udelete(7); // ɾ��������������
//					} else {
//						// �� Holl�ɾ��1�ֽ�
//						udelete(1);
//						break;
//					}
//				}
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
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 25;
	RCC_OscInitStruct.PLL.PLLN = 336;
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

/**
 * @brief USART2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART1_UART_Init(void) {

	/* USER CODE BEGIN USART2_Init 0 */

	/* USER CODE END USART2_Init 0 */

	/* USER CODE BEGIN USART2_Init 1 */

	/* USER CODE END USART2_Init 1 */
	huart1.Instance = USART1;
	huart1.Init.BaudRate = 115200;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart1) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN USART2_Init 2 */

	/* USER CODE END USART2_Init 2 */
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
	/* USER CODE BEGIN MX_GPIO_Init_1 */
	/* USER CODE END MX_GPIO_Init_1 */

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	/* USER CODE BEGIN MX_GPIO_Init_2 */
	/* USER CODE END MX_GPIO_Init_2 */
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
