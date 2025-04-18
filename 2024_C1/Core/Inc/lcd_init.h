#ifndef __LCD_INIT_H
#define __LCD_INIT_H

#include "sys.h"
#include "main.h"
#define USE_HORIZONTAL 1  //���ú�������������ʾ 0��1Ϊ���� 2��3Ϊ����


#if USE_HORIZONTAL==0||USE_HORIZONTAL==1
#define LCD_W 128
#define LCD_H 160

#else
#define LCD_W 160
#define LCD_H 128
#endif



//-----------------LCD�˿ڶ���----------------
#define LCD_SCLK_Clr() HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
  //GPIO_ResetBits(GPIOA,GPIO_Pin_5)//SCL=SCLK
#define LCD_SCLK_Set()  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
// GPIO_SetBits(GPIOA,GPIO_Pin_5)

#define LCD_MOSI_Clr() HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);
//SDA=MOSI
#define LCD_MOSI_Set() HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);


#define LCD_RES_Clr()  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_RESET);
//RES
#define LCD_RES_Set()  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_SET);


#define LCD_DC_Clr()   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
//DC
#define LCD_DC_Set()   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);


#define LCD_BLK_Clr()  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
//BLK
#define LCD_BLK_Set()  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);


#define LCD_CS_Clr()   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
//CS
#define LCD_CS_Set()   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);


//#define LCD_SCLK_Clr() GPIO_ResetBits(GPIOA,GPIO_Pin_5)//SCL=SCLK
//#define LCD_SCLK_Set() GPIO_SetBits(GPIOA,GPIO_Pin_5)

//#define LCD_MOSI_Clr() GPIO_ResetBits(GPIOA,GPIO_Pin_7)//SDA=MOSI
//#define LCD_MOSI_Set() GPIO_SetBits(GPIOA,GPIO_Pin_7)

//#define LCD_RES_Clr()  GPIO_ResetBits(GPIOD,GPIO_Pin_2)//RES
//#define LCD_RES_Set()  GPIO_SetBits(GPIOD,GPIO_Pin_2)

//#define LCD_DC_Clr()   GPIO_ResetBits(GPIOB,GPIO_Pin_5)//DC
//#define LCD_DC_Set()   GPIO_SetBits(GPIOB,GPIO_Pin_5)
//
//#define LCD_CS_Clr()   GPIO_ResetBits(GPIOA,GPIO_Pin_4)//CS
//#define LCD_CS_Set()   GPIO_SetBits(GPIOA,GPIO_Pin_4)

//#define LCD_BLK_Clr()  GPIO_ResetBits(GPIOB,GPIO_Pin_6)//BLK
//#define LCD_BLK_Set()  GPIO_SetBits(GPIOB,GPIO_Pin_6)




//void LCD_GPIO_Init(void);//��ʼ��GPIO
void LCD_Writ_Bus(uint8_t dat);//ģ��SPIʱ��
void LCD_WR_DATA8(uint8_t dat);//д��һ���ֽ�
void LCD_WR_DATA(uint16_t dat);//д�������ֽ�
void LCD_WR_REG(uint8_t dat);//д��һ��ָ��
void LCD_Address_Set(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2);//�������꺯��
void LCD_Init(void);//LCD��ʼ��
#endif




