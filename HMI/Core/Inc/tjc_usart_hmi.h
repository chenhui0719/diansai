#ifndef __TJCUSARTHMI_H__
#define __TJCUSARTHMI_H__

#include <stdio.h>

#define TJC_UART huart1
#define TJC_UART_INS USART1
extern UART_HandleTypeDef huart1;

void tjc_send_string(char *str);
void tjc_send_txt(char *objname, char *attribute, char *txt);
void tjc_send_val(char *objname, char *attribute, int val);
void tjc_send_nstring(char *str, unsigned char str_length);

extern uint8_t RxBuffer[1];
extern uint32_t msTicks;

#endif

