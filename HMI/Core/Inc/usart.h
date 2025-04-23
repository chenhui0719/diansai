#ifndef __USART_H__
#define __USART_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

extern UART_HandleTypeDef huart1;

#define BUFFER_SIZE  100
extern volatile uint8_t rx_len;  //接收�?帧数据的长度
extern volatile uint8_t recv_end_flag; //�?帧数据接收完成标�?
extern uint8_t rx_buffer[100];  //接收数据缓存数组

void MX_USART1_UART_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */

