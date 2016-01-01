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

#include "drivers/adc.h"

#include <stm32f4xx_conf.h>

namespace multitap {
  
void Adc::Init() {

	// // Initialize peripherals
	// RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2 |
	// 											 RCC_APB2Periph_ADC1 |
	// 											 RCC_APB2Periph_ADC3 |
	// 											 RCC_AHB1Periph_GPIOA |
	// 											 RCC_AHB1Periph_GPIOB |
	// 											 RCC_AHB1Periph_GPIOC |
	// 											 RCC_AHB1Periph_GPIOF, ENABLE);


	// // Configure analog input pins
	// GPIO_InitTypeDef gpio_init;

	// gpio_init.GPIO_PuPd = GPIO_PuPd_NOPULL;
  // gpio_init.GPIO_Mode = GPIO_Mode_AN;

	// gpio_init.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_6 | GPIO_Pin_7;
	// GPIO_Init(GPIOA, &gpio_init);

	// gpio_init.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	// GPIO_Init(GPIOB, &gpio_init);

	// gpio_init.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4;
	// GPIO_Init(GPIOC, &gpio_init);

	// gpio_init.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10;
	// GPIO_Init(GPIOF, &gpio_init);

  // // Use DMA to automatically copy ADC data register to values_ buffer.
  // DMA_InitTypeDef dma_init;

	// dma_init.DMA_DIR = DMA_DIR_PeripheralToMemory;
	// dma_init.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  // dma_init.DMA_MemoryInc = DMA_MemoryInc_Enable; 
  // dma_init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  // dma_init.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  // dma_init.DMA_Mode = DMA_Mode_Circular;
  // dma_init.DMA_Priority = DMA_Priority_High;
  // dma_init.DMA_FIFOMode = DMA_FIFOMode_Disable;
  // dma_init.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  // dma_init.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  // dma_init.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;

	// // DMA2 Ch. 0 for pots
	// dma_init.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
	// dma_init.DMA_Memory0BaseAddr = (uint32_t)&values_[ADC_TIME1_POT];
	// dma_init.DMA_BufferSize = 8; // only up to last pot
	// dma_init.DMA_Channel = DMA_Channel_0;
	// DMA_Init(DMA2_Stream0, &dma_init);
  // DMA_Cmd(DMA2_Stream0, ENABLE);

	// // DMA2 Ch. 2 for pots
	// dma_init.DMA_PeripheralBaseAddr = (uint32_t)&ADC3->DR;
	// dma_init.DMA_Memory0BaseAddr = (uint32_t)&values_[ADC_TIME1_CV];
	// dma_init.DMA_BufferSize = 6; // the rest
	// dma_init.DMA_Channel = DMA_Channel_2;
	// DMA_Init(DMA2_Stream0, &dma_init);
  // DMA_Cmd(DMA2_Stream0, ENABLE);

	// // Common ADC init
  // ADC_CommonInitTypeDef adc_common_init;
	// adc_common_init.ADC_Mode = ADC_Mode_Independent;
  // adc_common_init.ADC_Prescaler = ADC_Prescaler_Div8;
  // adc_common_init.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
  // adc_common_init.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_20Cycles;
  // ADC_CommonInit(&adc_common_init);

	// // ADC init
	// ADC_InitTypeDef adc_init;
	// adc_init.ADC_Resolution = ADC_Resolution_12b;
  // adc_init.ADC_ScanConvMode = ENABLE;
  // adc_init.ADC_ContinuousConvMode = DISABLE;
  // adc_init.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
  // adc_init.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	// adc_init.ADC_DataAlign = ADC_DataAlign_Left;

	// // ADC1 for pots
	// adc_init.ADC_NbrOfConversion = ADC_TIME1_CV; // only up to last pot
  // ADC_Init(ADC1, &adc_init);

	// // ADC3 for CVs
	// adc_init.ADC_NbrOfConversion = ADC_CHANNEL_LAST;
  // ADC_Init(ADC1, &adc_init);
  
	// // ADC1 channel configuration (pots)
	// ADC_RegularChannelConfig(ADC1, ADC_Channel_6, 1, ADC_SampleTime_480Cycles); // time1_pot
	// ADC_RegularChannelConfig(ADC1, ADC_Channel_7, 2, ADC_SampleTime_480Cycles); // time2_pot
	// ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 3, ADC_SampleTime_480Cycles); // level1_pot
	// ADC_RegularChannelConfig(ADC1, ADC_Channel_9, 4, ADC_SampleTime_480Cycles); // level2_pot
	// ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 5, ADC_SampleTime_480Cycles); // regen1_pot
	// ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 6, ADC_SampleTime_480Cycles); // regen2_pot
	// ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 7, ADC_SampleTime_480Cycles); // mix1_pot
	// ADC_RegularChannelConfig(ADC1, ADC_Channel_14, 8, ADC_SampleTime_480Cycles); // mix2_pot

	// // ADC3 channel configuration (CVs)
	// ADC_RegularChannelConfig(ADC3, ADC_Channel_5, 1, ADC_SampleTime_480Cycles); //PF7
	// ADC_RegularChannelConfig(ADC3, ADC_Channel_6, 2, ADC_SampleTime_480Cycles); //PF8
	// ADC_RegularChannelConfig(ADC3, ADC_Channel_3, 3, ADC_SampleTime_480Cycles); //PA3
	// ADC_RegularChannelConfig(ADC3, ADC_Channel_4, 4, ADC_SampleTime_480Cycles); //PF6
	// ADC_RegularChannelConfig(ADC3, ADC_Channel_7, 5, ADC_SampleTime_480Cycles); //PF9
	// ADC_RegularChannelConfig(ADC3, ADC_Channel_8, 6, ADC_SampleTime_480Cycles); //PF10

  // ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);
  // ADC_Cmd(ADC1, ENABLE);
	// ADC_DMACmd(ADC1, ENABLE);

	// ADC_DMARequestAfterLastTransferCmd(ADC3, ENABLE);
	// ADC_DMACmd(ADC3, ENABLE);
	// ADC_Cmd(ADC3, ENABLE);

	// Convert();








	DMA_InitTypeDef DMA_InitStructure;
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	ADC_InitTypeDef ADC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	/* enable clocks for DMA2, ADC1 ----------------------------------*/
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

	/* DMA2 stream4 channel0 configuration ----------------------------------*/
	DMA_InitStructure.DMA_Channel = DMA_Channel_0;  
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&values_[0];
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_BufferSize = 8;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA2_Stream4, &DMA_InitStructure);
	DMA_Cmd(DMA2_Stream4, ENABLE);

	/* ADC Common Init ------------------------------------------------------*/
	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div8;
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
	ADC_CommonInit(&ADC_CommonInitStructure);

	/* ADC1 Init ------------------------------------------------------------*/
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfConversion = 8;
	ADC_Init(ADC1, &ADC_InitStructure);
	
	/* Configure analog input pins ------------------------------------------*/
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC, ENABLE);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7; //Channel 6, 7
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1; //Channel 8, 9
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4; //Channel 11, 12, 13, 14
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/* ADC1 regular channel configuration -----------------------------------*/ 

	ADC_RegularChannelConfig(ADC1, ADC_Channel_6, 1, ADC_SampleTime_144Cycles); // time1_pot
	ADC_RegularChannelConfig(ADC1, ADC_Channel_7, 2, ADC_SampleTime_144Cycles); // time2_pot
	ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 3, ADC_SampleTime_144Cycles); // level1_pot
	ADC_RegularChannelConfig(ADC1, ADC_Channel_9, 4, ADC_SampleTime_144Cycles); // level2_pot
	ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 5, ADC_SampleTime_144Cycles); // regen1_pot
	ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 6, ADC_SampleTime_144Cycles); // regen2_pot
	ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 7, ADC_SampleTime_144Cycles); // mix1_pot
	ADC_RegularChannelConfig(ADC1, ADC_Channel_14, 8, ADC_SampleTime_144Cycles); // mix2_pot


	//DMA_ITConfig(DMA2_Stream0, DMA_IT_TC, ENABLE);
   	//NVIC_EnableIRQ(DMA2_Stream0_IRQn);
	
	ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);
	ADC_DMACmd(ADC1, ENABLE);
	ADC_Cmd(ADC1, ENABLE);
	ADC_SoftwareStartConv(ADC1);



}

void Adc::Convert() {
  ADC_SoftwareStartConv(ADC1);
	ADC_SoftwareStartConv(ADC3);
}

void Adc::DeInit() {
  ADC_DeInit();
}

}  // namespace multitap
