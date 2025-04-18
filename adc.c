#include "ti/devices/msp432e4/driverlib/rom_map.h"
#include <adc.h>
#include <ti/devices/msp432e4/driverlib/adc.h>
#include <ti/devices/msp432e4/driverlib/driverlib.h>
#include <ti/devices/msp432e4/driverlib/interrupt.h>
#include <ti/devices/msp432e4/driverlib/timer.h>
#include <ti/devices/msp432e4/driverlib/udma.h>
#include <ti/devices/msp432e4/inc/msp432e401y.h>

static uint32_t timerPeriph0 = SYSCTL_PERIPH_TIMER4;
static uint32_t timerBase0 = TIMER4_BASE;
static uint32_t adcPeriph0 = SYSCTL_PERIPH_ADC0;
static uint32_t adcBase0 = ADC0_BASE;
static uint32_t adcSeq0 = 2;
static uint32_t adcInt0 = INT_ADC0SS2;
static uint32_t adcIntFlag0 = ADC_INT_DMA_SS2;
static uint32_t dmaChann0 = UDMA_CH16_ADC0_2;
static uint32_t dmaArb0 = UDMA_ARB_4;
static uint32_t adcPriority0 = 0;
// hardware define

uint16_t adcData0[1024], adcData1[1024]; // buffers
uint16_t *adcIn = adcData0;

volatile bool AdcFlag = false; // finish flag

// DMA table fix to 1024
#if defined(__ICCARM__)
#pragma data_alignment = 1024
uint8_t pui8ControlTable[1024];
#elif defined(__TI_ARM__)
#pragma DATA_ALIGN(pui8ControlTable, 1024)
uint8_t pui8ControlTable[1024];
#else
uint8_t pui8ControlTable[1024] __attribute__((aligned(1024)));
#endif

// disturb
void AdcHandler() {
  if ((MAP_ADCIntStatusEx(adcBase0, true) & adcIntFlag0) == adcIntFlag0) {
    MAP_ADCIntClearEx(adcBase0, adcIntFlag0);
    MAP_ADCIntDisableEx(adcBase0, adcIntFlag0);
    MAP_IntDisable(adcInt0);
    MAP_TimerDisable(timerBase0, TIMER_A);
    AdcFlag = true;
  }
}

void ConfigureAdc() {
  MAP_SysCtlPeripheralEnable(timerPeriph0);
  MAP_SysCtlPeripheralEnable(adcPeriph0);
  MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA); // timer
  MAP_uDMAControlBaseSet(pui8ControlTable);
  MAP_uDMAEnable();                                // DMA
  MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE); // Out CLK
  GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3);
  GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_2);
  GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_1);
  GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_0); // Pins
}

// Get data
void StartAdcAMP1(int freq, int DataLen) {
  int channel = AMP1;
  MAP_ADCSequenceConfigure(adcBase0, adcSeq0, ADC_TRIGGER_TIMER, adcPriority0);
  MAP_ADCSequenceStepConfigure(adcBase0, adcSeq0, 0,
                               channel | ADC_CTL_IE | ADC_CTL_END);
  MAP_ADCSequenceDMAEnable(adcBase0, adcSeq0);
  MAP_ADCSequenceEnable(adcBase0, adcSeq0);
  MAP_ADCIntClearEx(adcBase0, adcIntFlag0);
  MAP_IntEnable(adcInt0);
  ADCIntRegister(adcBase0, adcSeq0, AdcHandler);

  MAP_uDMAChannelAssign(dmaChann0);
  MAP_uDMAChannelAttributeDisable(
      dmaChann0, UDMA_ATTR_ALTSELECT | UDMA_ATTR_USEBURST |
                     UDMA_ATTR_HIGH_PRIORITY | UDMA_ATTR_REQMASK);

  MAP_uDMAChannelControlSet(dmaChann0 | UDMA_PRI_SELECT,
                            UDMA_SIZE_16 | UDMA_SRC_INC_NONE | UDMA_DST_INC_16 |
                                dmaArb0);

  ADCClockConfigSet(adcBase0, ADC_CLOCK_SRC_PLL | ADC_CLOCK_RATE_FULL, 15);

  if (freq >= 75 * 20000)
    ADCClockConfigSet(ADC0_BASE, ADC_CLOCK_SRC_PLL | ADC_CLOCK_RATE_FULL, 12);
  else
    ADCClockConfigSet(ADC0_BASE, ADC_CLOCK_SRC_PLL | ADC_CLOCK_RATE_FULL, 15);

  MAP_TimerConfigure(timerBase0, TIMER_CFG_A_PERIODIC);
  MAP_TimerLoadSet(timerBase0, TIMER_A, 120000000 / freq);
  MAP_TimerADCEventSet(timerBase0, TIMER_ADC_TIMEOUT_A);
  MAP_TimerControlTrigger(timerBase0, TIMER_A, true);
  MAP_TimerEnable(timerBase0, TIMER_A);

  adcIn = (adcIn == adcData1) ? adcData0 : adcData1;

  MAP_uDMAChannelTransferSet(dmaChann0 | UDMA_PRI_SELECT, UDMA_MODE_BASIC,
                             (void *)&ADC0->SSFIFO2, (void *)adcIn, DataLen);
  MAP_uDMAChannelEnable(dmaChann0);
  MAP_ADCIntEnableEx(adcBase0, adcIntFlag0);

  while (!AdcFlag)
    ;
}
