/**
 * 文件说明：
 * 1. 本文件为 tjc_usart_hmi.c 和 tjc_usart_hmi.h 的实现部分。
 * 2. 使用时需要包含头文件 #include "tjc_usart_hmi.h"。
 * 3. 使用前需确保 HAL_UART_Transmit_IT() 已正确配置为中断方式发送数据。
 */

#include "main.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_uart.h"
#include <stdio.h>
#include "tjc_usart_hmi.h"

// 定义环形缓冲区的大小
#define RINGBUFFER_LEN 128

typedef struct {
	uint16_t Head;          // 环形缓冲区的头部指针
	uint16_t Tail;          // 环形缓冲区的尾部指针
	uint16_t Length;        // 环形缓冲区中数据的长度
	uint8_t Ring_data[RINGBUFFER_LEN]; // 环形缓冲区数据存储
} RingBuffer_t;

RingBuffer_t ringBuffer;    // 定义一个环形缓冲区实例
uint8_t RxBuffer[1];        // 接收缓冲区

/********************************************************
 函数名称       : intToStr
 创建日期     : 2024.09.18
 功能描述     : 将整数转换为字符串
 输入参数     : 要转换的整数，目标字符串
 返回值       : 无
 修改记录：
 **********************************************************/
void intToStr(int num, char *str) {
	int i = 0;
	int isNegative = 0;

	// 检查是否为负数
	if (num < 0) {
		isNegative = 1;
		num = -num;
	}

	// 获取每一位数字
	do {
		str[i++] = (num % 10) + '0';
		num /= 10;
	} while (num);

	// 如果是负数，添加负号
	if (isNegative) {
		str[i++] = '-';
	}

	// 添加字符串结束符
	str[i] = '\0';

	// 反转字符串
	int start = 0;
	int end = i - 1;
	while (start < end) {
		char temp = str[start];
		str[start] = str[end];
		str[end] = temp;
		start++;
		end--;
	}
	return;
}

/********************************************************
 函数名称       : uart_send_char
 创建日期     : 2024.09.18
 功能描述     : 发送一个字符
 输入参数     : 要发送的字符
 返回值       : 无
 修改记录：
 **********************************************************/
void uart_send_char(char ch) {
	uint8_t ch2 = (uint8_t) ch;
	// 等待发送完成
	while (__HAL_UART_GET_FLAG(&TJC_UART, UART_FLAG_TC) == RESET)
		;
	// 发送字符
	HAL_UART_Transmit_IT(&TJC_UART, &ch2, 1);
	return;
}

/********************************************************
 函数名称       : uart_send_string
 创建日期     : 2024.09.18
 功能描述     : 发送一个字符串
 输入参数     : 要发送的字符串
 返回值       : 无
 修改记录：
 **********************************************************/
void uart_send_string(char *str) {
	// 遍历字符串直到遇到字符串结束符
	while (*str != 0 && str != 0) {
		// 逐个字符发送
		uart_send_char(*str++);
	}
	return;
}

/********************************************************
 函数名称       : tjc_send_string
 创建日期     : 2024.09.18
 功能描述     : 发送字符串并添加结束标志
 输入参数     : 要发送的字符串
 返回值       : 无
 示例：         tjc_send_string("n0.val=100"); 发送字符串 n0.val=100 并添加结束标志
 修改记录：
 **********************************************************/
void tjc_send_string(char *str) {
	// 遍历字符串直到遇到字符串结束符
	while (*str != 0 && str != 0) {
		// 逐个字符发送
		uart_send_char(*str++);
	}
	// 添加结束标志
	uart_send_char(0xff);
	uart_send_char(0xff);
	uart_send_char(0xff);
	return;
}

/********************************************************
 函数名称       : tjc_send_txt
 创建日期     : 2024.09.18
 功能描述     : 发送文本属性值
 输入参数     : 对象名称，属性名称，文本内容
 返回值       : 无
 示例：         tjc_send_txt("t0", "txt", "ABC"); 发送 t0.txt="ABC"
 修改记录：
 **********************************************************/
void tjc_send_txt(char *objname, char *attribute, char *txt) {
	// 发送对象名称
	uart_send_string(objname);
	uart_send_char('.');
	// 发送属性名称
	uart_send_string(attribute);
	uart_send_string("=\"");
	// 发送文本内容
	uart_send_string(txt);
	uart_send_char('\"');
	// 添加结束标志
	uart_send_char(0xff);
	uart_send_char(0xff);
	uart_send_char(0xff);
	return;
}

/********************************************************
 函数名称       : tjc_send_val
 创建日期     : 2024.09.18
 功能描述     : 发送数值属性值
 输入参数     : 对象名称，属性名称，数值
 返回值       : 无
 示例：         tjc_send_val("n0", "val", 100); 发送 n0.val=100
 修改记录：
 **********************************************************/
void tjc_send_val(char *objname, char *attribute, int val) {
	// 发送对象名称
	uart_send_string(objname);
	uart_send_char('.');
	// 发送属性名称
	uart_send_string(attribute);
	uart_send_char('=');
	// 将数值转换为字符串
	char txt[12] = "";
	intToStr(val, txt);
	// 发送数值字符串
	uart_send_string(txt);
	// 添加结束标志
	uart_send_char(0xff);
	uart_send_char(0xff);
	uart_send_char(0xff);
	return;
}

/********************************************************
 函数名称       : tjc_send_nstring
 创建日期     : 2024.09.18
 功能描述     : 发送指定长度的字符串
 输入参数     : 要发送的字符串，字符串长度
 返回值       : 无
 修改记录：
 **********************************************************/
void tjc_send_nstring(char *str, unsigned char str_length) {
	// 遍历字符串直到达到指定长度
	for (int var = 0; var < str_length; ++var) {
		// 逐个字符发送
		uart_send_char(*str++);
	}
	// 添加结束标志
	uart_send_char(0xff);
	uart_send_char(0xff);
	uart_send_char(0xff);
	return;
}

/********************************************************
 函数名称       : HAL_UART_RxCpltCallback
 创建日期     : 2022.10.08
 功能描述     : UART接收完成回调函数
 输入参数     : UART句柄
 返回值       : void
 修改记录：
 **********************************************************/
// void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
// {
//     if(huart->Instance == TJC_UART_INS) // 判断是否为指定的UART实例
//     {
//         // 将接收到的数据写入环形缓冲区
//         write1ByteToRingBuffer(RxBuffer[0]);
//         // 继续接收下一个字节
//         HAL_UART_Receive_IT(&TJC_UART, RxBuffer, 1);
//     }
//     return;
// }
/********************************************************
 函数名称       : initRingBuffer
 创建日期     : 2022.10.08
 功能描述     : 初始化环形缓冲区
 输入参数     : 无
 返回值       : void
 修改记录：
 **********************************************************/
void initRingBuffer(void) {
	// 初始化环形缓冲区信息
	ringBuffer.Head = 0;
	ringBuffer.Tail = 0;
	ringBuffer.Length = 0;
	return;
}

/********************************************************
 函数名称       : write1ByteToRingBuffer
 创建日期     : 2022.10.08
 功能描述     : 向环形缓冲区写入一个字节
 输入参数     : 要写入的数据
 返回值       : void
 修改记录：
 **********************************************************/
void write1ByteToRingBuffer(uint8_t data) {
	if (ringBuffer.Length >= RINGBUFFER_LEN) // 判断环形缓冲区是否已满
	{
		return; // 如果已满，直接返回
	}
}
