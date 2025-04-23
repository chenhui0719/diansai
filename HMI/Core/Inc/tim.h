#ifndef __TIM_H__
#define __TIM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

extern TIM_HandleTypeDef htim2;

extern TIM_HandleTypeDef htim3;

void MX_TIM2_Init(void);
void MX_TIM3_Init(void);

#ifdef __cplusplus
}
#endif

#endif

