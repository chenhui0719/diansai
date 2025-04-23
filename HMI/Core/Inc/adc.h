#ifndef __ADC_H__
#define __ADC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

extern ADC_HandleTypeDef hadc1;

#define adc_trigger_size 544
#define adc_cache_size 1024

void MX_ADC1_Init(void);

#ifdef __cplusplus
}
#endif

#endif

