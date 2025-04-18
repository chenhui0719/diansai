/* --COPYRIGHT--,BSD
 * Copyright (c) 2017, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/
/******************************************************************************
 * MSP432E4 Empty Project
 *
 * Description: An empty project that uses DriverLib
 *
 *                MSP432E401Y
 *             ------------------
 *         /|\|                  |
 *          | |                  |
 *          --|RST               |
 *            |                  |
 *            |                  |
 *            |                  |
 *            |                  |
 *            |                  |
 *             ------------------
 * Author:
 *******************************************************************************/
/* DriverLib Includes */
#include "uart.h"
#include <adc.h>
#include <algorithm>
#include <ti/devices/msp432e4/driverlib/driverlib.h>
/* Standard Includes */
#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#define maxx(a, b) (a > b ? a : b)
#define minx(a, b) (a < b ? a : b)

template <class T> void getMinMax(const T arr[], int n, T &min, T &max) {
  min = max = arr[0];
  for (int i = 1; i < n; i++) {
    if (arr[i] < min) {
      min = arr[i];
    }
    if (arr[i] > max) {
      max = arr[i];
    }
  }
  return;
} // min max

template <class T>
void getMinMax(const T arr[], int n, T &min, T &max, int &min_id, int &max_id) {
  min = max = arr[0];
  min_id = 0;
  max_id = 0;
  for (int i = 1; i < n; i++) {
    if (arr[i] < min) {
      min = arr[i];
      min_id = i;
    }
    if (arr[i] > max) {
      max = arr[i];
      max_id = i;
    }
  }
  return;
} // min max with id

float real[1024], image[1024];
float magnitude[1024]; // 1024 points FFT result
void DFT(int N) {      // N --> N points
  const float PI = 3.1415926535897932384f;
  for (int i = 0; i < N; i++) {
    real[i] = image[i] = 0;
    for (int j = 0; j < N; j++) {
      real[i] += cosf(2 * PI / N * i * j) * adcIn[j];
      image[i] -= sinf(2 * PI / N * i * j) * adcIn[j];
    }
    magnitude[i] = sqrtf(real[i] * real[i] + image[i] * image[i]);
  }
  return;
}

double
    peak_magnitude[5], // peak_magnitude --> highest magnitude -- selected wav
    wav_freq[5];       // wav_freq --> the frequcy of the wav that selected
int wav_id[5];         // the index of the selected wave in the fft result
void FindPeak(float fft_magnitude[], int n,
              double freq) { // freq --> sampling frequency
  for (int i = 0; i < 5; i++) {
    peak_magnitude[i] = wav_freq[i] = 0.0;
    wav_id[i] = 0;
  }
  for (int i = 2; i < n / 2; i++) {
    // start from 2 to avoid noises, search in the front half
    if (fft_magnitude[i] > peak_magnitude[0]) {
      peak_magnitude[0] = fft_magnitude[i];
      wav_id[0] = i;
    }
  } // find the base

  int range = wav_id[0] / 8; // smaller the range of the index
  if (range <= 1)
    range = 1;
  if (range % 2 != 0)
    range--;

  for (int i = 1; i < 5; i++) {
    wav_id[i] = (i + 1) * wav_id[0]; // harmonic i predict i --> 1 to 4
    peak_magnitude[i] = fft_magnitude[wav_id[i]];
    for (int j = (i + 1) * wav_id[0] - range / 2;
         j < (i + 1) * wav_id[0] + range / 2 + 1; j++) {
      // search the higher peak in the range
      if (fft_magnitude[j] > peak_magnitude[i]) {
        peak_magnitude[i] = fft_magnitude[j];
        wav_id[i] = j;
      }
    }
  }
  for (int i = 0; i < 5; i++)
    wav_freq[i] = wav_id[i] * 1.0 / n * freq;
  // calculate the frequency of the waves

  for (int i = 0; i < 5; i++) {
    double fix_real = 0, fix_image = 0;
    int fix_range = 1; // the var ctrls the fix range
    for (int j = wav_id[i] - fix_range; j < wav_id[i] + fix_range; j++)
      // fix the result of fft
      if (j > 0 && j < n) {
        fix_real += real[j];
        fix_image += image[j];
      }
    peak_magnitude[i] =
        sqrt(fix_real * fix_real +
             fix_image * fix_image); // cal the magnitude of each wav
  }
  return;
}

int WaveCut(int n, uint16_t triggerVal) {
  int StId = 0; // start of the wave
  for (int i = 4; i < n; i++) {
    if (adcIn[i - 4] <= triggerVal && adcIn[i] >= triggerVal) {
      for (int j = i - 4; j < i; j++) {
        if (adcIn[j + 1] >= triggerVal)
          return j;
      }
      return i;
    }
  }
  return StId;
}

int Base() {
  int CalPoints = 500; // points to calculate basefreq
  float Min_Magnitude, Max_Magnitude;
  int MinId, MaxId;
  int SampleFreq = 250 * 1000; // 250kHz
  StartAdcAMP1(SampleFreq, CalPoints);
  StartAdcAMP1(SampleFreq, CalPoints);
  DFT(CalPoints);
  getMinMax(magnitude + 1, CalPoints / 2 - 1, Min_Magnitude, Max_Magnitude,
            MinId, MaxId);
  MinId++;
  MaxId++;                                              // "from 0" to "from 1"
  int ans1 = (long long)MaxId * SampleFreq / CalPoints; // basefreq
  if (ans1 >= 20 * 1000)
    return ans1; // if answer above 20kHz

  SampleFreq = 50 * 1000; // adjust to 50kHz and recalculate
  StartAdcAMP1(SampleFreq, CalPoints);
  StartAdcAMP1(SampleFreq, CalPoints);
  DFT(CalPoints);
  getMinMax(magnitude + 1, CalPoints / 2 - 1, Min_Magnitude, Max_Magnitude,
            MinId, MaxId);
  MinId++;
  MaxId++;
  int ans2 = (long long)MaxId * SampleFreq / CalPoints;
  return ans2;
} // checked

double DoTHD(double mag_1, double mag_2, double mag_3, double mag_4,
             double mag_5) {
  return sqrt(mag_2 * mag_2 + mag_3 * mag_3 + mag_4 * mag_4 + mag_5 * mag_5) /
         mag_1;
}

void SendData(int BaseFreq, int SampleFreq, int Points, int WST,
              uint16_t adcMin, uint16_t adcMax) {
  FindPeak(magnitude, Points, SampleFreq);
  float value[5] = {1};
  for (int i = 1; i < 5; i++)
    value[i] = peak_magnitude[i] / peak_magnitude[0];
  float THD = DoTHD(peak_magnitude[0], peak_magnitude[1], peak_magnitude[2],
                    peak_magnitude[3], peak_magnitude[4]) *
              100;
  int WaveLen = Points / wav_id[0];
  BT_send(value, THD);
  HMI_Send(value, THD, WST, WaveLen, adcIn, adcMin, adcMax, BaseFreq);
}

void start() {

  StartAdcAMP1(250 * 1000, 500);
  StartAdcAMP1(250 * 1000, 500);
  StartAdcAMP1(250 * 1000, 500);
  StartAdcAMP1(250 * 1000, 500);

  int BaseFreq = Base();
  // UART part

  int DFT_Points = 500;
  int SampleFreq = BaseFreq * 20; // adjust to meet the wave freq
  uint16_t adcMin, adcMax;
  getMinMax(adcIn, DFT_Points, adcMin, adcMax);
  int WaveStart = WaveCut(DFT_Points / 2, (adcMin + adcMax) / 2);
  StartAdcAMP1(SampleFreq, DFT_Points);
  StartAdcAMP1(SampleFreq, DFT_Points);
  DFT(DFT_Points);
  SendData(BaseFreq, SampleFreq, DFT_Points, WaveStart, adcMin, adcMax);
  return;
}

int main(void) {

  // 启用 GPIOF 作为 LED 端口（假设 LED 连接在 GPIO_PIN_0）
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
  GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0);

  MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL |
                          SYSCTL_CFG_VCO_480),
                         120000000);
  FPUEnable();
  FPULazyStackingEnable();
  ConfigureAdc();
  BT_Config();
  HMI_Configure();

  while (1)
    start();
  return 0;
}
