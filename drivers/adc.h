// Copyright 2014 Olivier Gillet.
//
// Author: Olivier Gillet (ol.gillet@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
// 
// See http://creativecommons.org/licenses/MIT/ for more information.
//
// -----------------------------------------------------------------------------
//
// Driver for ADC.

#ifndef ADC_H_
#define ADC_H_

#include "stmlib/stmlib.h"

#include <stm32f4xx_conf.h>

enum AdcChannel {
	ADC_TIME1_POT,
	ADC_TIME2_POT,
	ADC_LEVEL1_POT,
	ADC_LEVEL2_POT,
	ADC_REGEN1_POT,
	ADC_REGEN2_POT,
	ADC_MIX1_POT,
	ADC_MIX2_POT,
	ADC_TIME1_CV,
	ADC_TIME2_CV,
	ADC_LEVEL1_CV,
	ADC_LEVEL2_CV,
	ADC_REGEN1_CV,
	ADC_REGEN2_CV,
	ADC_CHANNEL_LAST
};

class Adc {
public:
  Adc() { }
  ~Adc() { }

  void Init() {

    // Initialize peripherals
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2 |
                           RCC_AHB1Periph_GPIOA |
                           RCC_AHB1Periph_GPIOB |
                           RCC_AHB1Periph_GPIOC |
                           RCC_AHB1Periph_GPIOF, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 |
                           RCC_APB2Periph_ADC3, ENABLE);


    // Configure analog input pins
    GPIO_InitTypeDef gpio_init;

    gpio_init.GPIO_PuPd = GPIO_PuPd_NOPULL;
    gpio_init.GPIO_Mode = GPIO_Mode_AN;

    gpio_init.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_Init(GPIOA, &gpio_init);

    gpio_init.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_Init(GPIOB, &gpio_init);

    gpio_init.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4;
    GPIO_Init(GPIOC, &gpio_init);

    gpio_init.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10;
    GPIO_Init(GPIOF, &gpio_init);

    // // Use DMA to automatically copy ADC data register to values_ buffer.
    DMA_InitTypeDef dma_init;

    dma_init.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    dma_init.DMA_MemoryInc = DMA_MemoryInc_Enable;
    dma_init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    dma_init.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    dma_init.DMA_Mode = DMA_Mode_Circular;
    dma_init.DMA_Priority = DMA_Priority_High;
    dma_init.DMA_FIFOMode = DMA_FIFOMode_Disable;
    dma_init.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
    dma_init.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    dma_init.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;

    // DMA2 Stream 4 Channel 0 for pots
    dma_init.DMA_Channel = DMA_Channel_0;
    dma_init.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
    dma_init.DMA_Memory0BaseAddr = (uint32_t)&values_[0];
    dma_init.DMA_DIR = DMA_DIR_PeripheralToMemory;
    dma_init.DMA_BufferSize = 8;

    DMA_Init(DMA2_Stream4, &dma_init);
    DMA_Cmd(DMA2_Stream4, ENABLE);

    // // DMA2 Stream 0 Channel 2 for pots
    dma_init.DMA_PeripheralBaseAddr = (uint32_t)&ADC3->DR;
    dma_init.DMA_Memory0BaseAddr = (uint32_t)&values_[ADC_TIME1_CV];
    dma_init.DMA_BufferSize = 6;
    dma_init.DMA_Channel = DMA_Channel_2;
    DMA_Init(DMA2_Stream0, &dma_init);
    DMA_Cmd(DMA2_Stream0, ENABLE);

    // Common ADC init
    ADC_CommonInitTypeDef adc_common_init;
    adc_common_init.ADC_Mode = ADC_Mode_Independent;
    adc_common_init.ADC_Prescaler = ADC_Prescaler_Div8;
    adc_common_init.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
    adc_common_init.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_20Cycles;
    ADC_CommonInit(&adc_common_init);

    // ADC init
    ADC_InitTypeDef adc_init;
    adc_init.ADC_Resolution = ADC_Resolution_12b;
    adc_init.ADC_ScanConvMode = ENABLE;
    adc_init.ADC_ContinuousConvMode = DISABLE;
    adc_init.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
    adc_init.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
    adc_init.ADC_DataAlign = ADC_DataAlign_Right;

    // // ADC1 for pots
    adc_init.ADC_NbrOfConversion = 8;
    ADC_Init(ADC1, &adc_init);

    // ADC3 for CVs
    adc_init.ADC_NbrOfConversion = 6;
    ADC_Init(ADC3, &adc_init);

    // ADC1 channel configuration (pots)
    ADC_RegularChannelConfig(ADC1, ADC_Channel_6, 1, ADC_SampleTime_480Cycles); // time1_pot
    ADC_RegularChannelConfig(ADC1, ADC_Channel_7, 2, ADC_SampleTime_480Cycles); // time2_pot
    ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 3, ADC_SampleTime_480Cycles); // level1_pot
    ADC_RegularChannelConfig(ADC1, ADC_Channel_9, 4, ADC_SampleTime_480Cycles); // level2_pot
    ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 5, ADC_SampleTime_480Cycles); // regen1_pot
    ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 6, ADC_SampleTime_480Cycles); // regen2_pot
    ADC_RegularChannelConfig(ADC1, ADC_Channel_14, 7, ADC_SampleTime_480Cycles); // mix1_pot
    ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 8, ADC_SampleTime_480Cycles); // mix2_pot

    // ADC3 channel configuration (CVs)
    ADC_RegularChannelConfig(ADC3, ADC_Channel_5, 1, ADC_SampleTime_480Cycles); //PF7
    ADC_RegularChannelConfig(ADC3, ADC_Channel_6, 2, ADC_SampleTime_480Cycles); //PF8
    ADC_RegularChannelConfig(ADC3, ADC_Channel_3, 3, ADC_SampleTime_480Cycles); //PA3
    ADC_RegularChannelConfig(ADC3, ADC_Channel_4, 4, ADC_SampleTime_480Cycles); //PF6
    ADC_RegularChannelConfig(ADC3, ADC_Channel_7, 5, ADC_SampleTime_480Cycles); //PF9
    ADC_RegularChannelConfig(ADC3, ADC_Channel_8, 6, ADC_SampleTime_480Cycles); //PF10

    ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);
    ADC_DMACmd(ADC1, ENABLE);
    ADC_Cmd(ADC1, ENABLE);

    ADC_DMARequestAfterLastTransferCmd(ADC3, ENABLE);
    ADC_DMACmd(ADC3, ENABLE);
    ADC_Cmd(ADC3, ENABLE);

    Convert();
  }

  inline void DeInit() {
    ADC_DeInit();
  }

  inline void Convert() {
    ADC_SoftwareStartConv(ADC1);
    ADC_SoftwareStartConv(ADC3);
  }

	inline uint16_t value(uint8_t channel) const {
    return values_[channel] << 4; /* compensate for ADC max of 4096 */
	}
  inline float float_value(uint8_t channel) const {
		return static_cast<float>(values_[channel]) / 4096.0f; /* max is 4096 */
	}

 private:
  uint16_t values_[ADC_CHANNEL_LAST];
  
  DISALLOW_COPY_AND_ASSIGN(Adc);
};

#endif
