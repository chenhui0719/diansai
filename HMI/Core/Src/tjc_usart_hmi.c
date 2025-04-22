/**
 ʹ��ע������:
 1.��tjc_usart_hmi.c��tjc_usart_hmi.h �ֱ��빤��
 2.����Ҫʹ�õĺ������ڵ�ͷ�ļ������� #include "tjc_usart_hmi.h"
 3.ʹ��ǰ�뽫 HAL_UART_Transmit_IT() ���������Ϊ��ĵ�Ƭ���Ĵ��ڷ��͵��ֽں���


 */

#include "main.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_uart.h"
#include <stdio.h>
#include "tjc_usart_hmi.h"

typedef struct {
	uint16_t Head;
	uint16_t Tail;
	uint16_t Length;
	uint8_t Ring_data[RINGBUFFER_LEN];
} RingBuffer_t;

RingBuffer_t ringBuffer; // ����һ��ringBuffer�Ļ�����
uint8_t RxBuffer[1];

/********************************************************
 ��������  		intToStr
 ���ڣ�    	2024.09.18
 ���ܣ�    	������ת��Ϊ�ַ���
 ���������		Ҫת������������,������ַ�������
 ����ֵ�� 		��
 �޸ļ�¼��
 **********************************************************/
void intToStr(int num, char *str) {
	int i = 0;
	int isNegative = 0;

	// ��������
	if (num < 0) {
		isNegative = 1;
		num = -num;
	}

	// ��ȡÿһλ����
	do {
		str[i++] = (num % 10) + '0';
		num /= 10;
	} while (num);

	// ����Ǹ��������Ӹ���
	if (isNegative) {
		str[i++] = '-';
	}

	// �����ַ�����ֹ��
	str[i] = '\0';

	// ��ת�ַ���
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
 ��������  		uart_send_char
 ���ڣ�    	2024.09.18
 ���ܣ�    	���ڷ��͵����ַ�
 ���������		Ҫ���͵ĵ����ַ�
 ����ֵ�� 		��
 �޸ļ�¼��
 **********************************************************/
void uart_send_char(char ch) {
	uint8_t ch2 = (uint8_t) ch;
	// ������0æ��ʱ��ȴ�����æ��ʱ���ٷ��ʹ��������ַ�
//    while (__HAL_UART_GET_FLAG(&TJC_UART, UART_FLAG_TXE) == RESET)
	; // �ȴ��������
	while (__HAL_UART_GET_FLAG(&TJC_UART, UART_FLAG_TC) == RESET)
		;
	// ���͵����ַ�
	HAL_UART_Transmit_IT(&TJC_UART, &ch2, 1);
	return;
}

void uart_send_string(char *str) {
	// ��ǰ�ַ�����ַ���ڽ�β ���� �ַ����׵�ַ��Ϊ��
	while (*str != 0 && str != 0) {
		// �����ַ����׵�ַ�е��ַ��������ڷ������֮���׵�ַ����
		uart_send_char(*str++);
	}
	return;
}

/********************************************************
 ��������  		tjc_send_string
 ���ڣ�    	2024.09.18
 ���ܣ�    	���ڷ����ַ����ͽ�����
 ���������		Ҫ���͵��ַ���
 ����ֵ�� 		��
 ʾ��:			tjc_send_val("n0", "val", 100); ���������ݾ��� n0.val=100
 �޸ļ�¼��
 **********************************************************/
void tjc_send_string(char *str) {
	// ��ǰ�ַ�����ַ���ڽ�β ���� �ַ����׵�ַ��Ϊ��
	while (*str != 0 && str != 0) {
		// �����ַ����׵�ַ�е��ַ��������ڷ������֮���׵�ַ����
		uart_send_char(*str++);
	}
	uart_send_char(0xff);
	uart_send_char(0xff);
	uart_send_char(0xff);
	return;
}

/********************************************************
 ��������  		tjc_send_txt
 ���ڣ�    	2024.09.18
 ���ܣ�    	���ڷ����ַ����ͽ�����
 ���������		Ҫ���͵��ַ���
 ����ֵ�� 		��
 ʾ��:			tjc_send_txt("t0", "txt", "ABC"); ���������ݾ���t0.txt="ABC"
 �޸ļ�¼��
 **********************************************************/
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

/********************************************************
 ��������  		tjc_send_val
 ���ڣ�    	2024.09.18
 ���ܣ�    	���ڷ����ַ����ͽ�����
 ���������		Ҫ���͵��ַ���
 ����ֵ�� 		��
 �޸ļ�¼��
 **********************************************************/
void tjc_send_val(char *objname, char *attribute, int val) {
	// ƴ���ַ���,����n0.val=123
	uart_send_string(objname);
	uart_send_char('.');
	uart_send_string(attribute);
	uart_send_char('=');
	// C���������ε�ȡֵ��Χ�ǣ���-2147483648 ~ 2147483647��, �Ϊ-2147483648,���Ͻ�����\0һ��12���ַ�
	char txt[12] = "";
	intToStr(val, txt);
	uart_send_string(txt);
	uart_send_char(0xff);
	uart_send_char(0xff);
	uart_send_char(0xff);
	return;
}

/********************************************************
 ��������  		tjc_send_nstring
 ���ڣ�    	2024.09.18
 ���ܣ�    	���ڷ����ַ����ͽ�����
 ���������		Ҫ���͵��ַ���,�ַ�������
 ����ֵ�� 		��
 �޸ļ�¼��
 **********************************************************/
void tjc_send_nstring(char *str, unsigned char str_length) {
	// ��ǰ�ַ�����ַ���ڽ�β ���� �ַ����׵�ַ��Ϊ��
	for (int var = 0; var < str_length; ++var) {
		// �����ַ����׵�ַ�е��ַ��������ڷ������֮���׵�ַ����
		uart_send_char(*str++);
	}
	uart_send_char(0xff);
	uart_send_char(0xff);
	uart_send_char(0xff);
	return;
}

/********************************************************
 ��������  	HAL_UART_RxCpltCallback
 ���ڣ�    	2022.10.08
 ���ܣ�    	���ڽ����ж�,�����յ�������д�뻷�λ�����
 ���������
 ����ֵ�� 		void
 �޸ļ�¼��
 **********************************************************/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {

	if (huart->Instance == TJC_UART_INS) // �ж������ĸ����ڴ������ж�
	{
		write1ByteToRingBuffer(RxBuffer[0]);
		HAL_UART_Receive_IT(&TJC_UART, RxBuffer, 1); // ����ʹ�ܴ���2�����ж�
	}
	return;
}

/********************************************************
 ��������  		initRingBuffer
 ���ڣ�    	2022.10.08
 ���ܣ�    	��ʼ�����λ�����
 ���������
 ����ֵ�� 		void
 �޸ļ�¼��
 **********************************************************/
void initRingBuffer(void) {
	// ��ʼ�������Ϣ
	ringBuffer.Head = 0;
	ringBuffer.Tail = 0;
	ringBuffer.Length = 0;
	return;
}

/********************************************************
 ��������  		write1ByteToRingBuffer
 ���ڣ�    	2022.10.08
 ���ܣ�    	�����λ�����д������
 ���������		Ҫд���1�ֽ�����
 ����ֵ�� 		void
 �޸ļ�¼��
 **********************************************************/
void write1ByteToRingBuffer(uint8_t data) {
	if (ringBuffer.Length >= RINGBUFFER_LEN) // �жϻ������Ƿ�����
	{
		return;
	}
	ringBuffer.Ring_data[ringBuffer.Tail] = data;
	ringBuffer.Tail = (ringBuffer.Tail + 1) % RINGBUFFER_LEN; // ��ֹԽ��Ƿ�����
	ringBuffer.Length++;
	return;
}

/********************************************************
 ��������  		deleteRingBuffer
 ���ߣ�
 ���ڣ�    	2022.10.08
 ���ܣ�    	ɾ�����ڻ���������Ӧ���ȵ�����
 ���������		Ҫɾ���ĳ���
 ����ֵ�� 		void
 �޸ļ�¼��
 **********************************************************/
void deleteRingBuffer(uint16_t size) {
	if (size >= ringBuffer.Length) {
		initRingBuffer();
		return;
	}
	for (int i = 0; i < size; i++) {
		ringBuffer.Head = (ringBuffer.Head + 1) % RINGBUFFER_LEN; // ��ֹԽ��Ƿ�����
		ringBuffer.Length--;
		return;
	}
}

/********************************************************
 ��������  		read1ByteFromRingBuffer
 ���ߣ�
 ���ڣ�    	2022.10.08
 ���ܣ�    	�Ӵ��ڻ�������ȡ1�ֽ�����
 ���������		position:��ȡ��λ��
 ����ֵ�� 		����λ�õ�����(1�ֽ�)
 �޸ļ�¼��
 **********************************************************/
uint8_t read1ByteFromRingBuffer(uint16_t position) {
	uint16_t realPosition = (ringBuffer.Head + position) % RINGBUFFER_LEN;

	return ringBuffer.Ring_data[realPosition];
}

/********************************************************
 ��������  		getRingBufferLength
 ���ߣ�
 ���ڣ�    	2022.10.08
 ���ܣ�    	��ȡ���ڻ���������������
 ���������
 ����ֵ�� 		���ڻ���������������
 �޸ļ�¼��
 **********************************************************/
uint16_t getRingBufferLength() {
	return ringBuffer.Length;
}

/********************************************************
 ��������  		isRingBufferOverflow
 ���ߣ�
 ���ڣ�    	2022.10.08
 ���ܣ�    	�жϻ��λ������Ƿ�����
 ���������
 ����ֵ�� 		0:���λ��������� , 1:���λ�����δ��
 �޸ļ�¼��
 **********************************************************/
uint8_t isRingBufferOverflow() {
	return ringBuffer.Length < RINGBUFFER_LEN;
}
