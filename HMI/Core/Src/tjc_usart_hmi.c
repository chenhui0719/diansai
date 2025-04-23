#include "main.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_uart.h"
#include <stdio.h>
#include "tjc_usart_hmi.h"

uint8_t RxBuffer[1];

void intToStr(int num, char *str) {
	int i = 0;
	int isNegative = 0;

	if (num < 0) {
		isNegative = 1;
		num = -num;
	}

	do {
		str[i++] = (num % 10) + '0';
		num /= 10;
	} while (num);

	if (isNegative) {
		str[i++] = '-';
	}

	str[i] = '\0';

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

void uart_send_char(char ch) {
	uint8_t ch2 = (uint8_t) ch;
//    while (__HAL_UART_GET_FLAG(&TJC_UART, UART_FLAG_TXE) == RESET)
	;
	while (__HAL_UART_GET_FLAG(&TJC_UART, UART_FLAG_TC) == RESET)
		;
	HAL_UART_Transmit_IT(&TJC_UART, &ch2, 1);
	return;
}

void uart_send_string(char *str) {
	while (*str != 0 && str != 0) {
		uart_send_char(*str++);
	}
	return;
}

void tjc_send_string(char *str) {
	while (*str != 0 && str != 0) {
		uart_send_char(*str++);
	}
	uart_send_char(0xff);
	uart_send_char(0xff);
	uart_send_char(0xff);
	return;
}

void tjc_send_txt(char *objname, char *attribute, char *txt) {

	uart_send_string(objname);
	uart_send_char('.');
	uart_send_string(attribute);
	uart_send_string("=\"");
	uart_send_string(txt);
	uart_send_char('\"');
	uart_send_char(0xff);
	uart_send_char(0xff);
	uart_send_char(0xff);
	return;
}

void tjc_send_val(char *objname, char *attribute, int val) {
	uart_send_string(objname);
	uart_send_char('.');
	uart_send_string(attribute);
	uart_send_char('=');
	char txt[12] = "";
	intToStr(val, txt);
	uart_send_string(txt);
	uart_send_char(0xff);
	uart_send_char(0xff);
	uart_send_char(0xff);
	return;
}

void tjc_send_nstring(char *str, unsigned char str_length) {
	for (int var = 0; var < str_length; ++var) {
		uart_send_char(*str++);
	}
	uart_send_char(0xff);
	uart_send_char(0xff);
	uart_send_char(0xff);
	return;
}
