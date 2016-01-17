// Copyright 2016 Matthias Puech.
//
// Author: Matthias Puech (matthias.puech@gmail.com)
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
// Driver for the front panel switches.

#include "drivers/switches.h"

#include <algorithm>

namespace mtd {

using namespace std;

struct PinAssign {
	GPIO_TypeDef* gpio;
	uint16_t pin;
};

const PinAssign pins[kNumSwitches] = {
	{GPIOE, GPIO_Pin_5},
	{GPIOD, GPIO_Pin_5},
	{GPIOA, GPIO_Pin_9},
	{GPIOG, GPIO_Pin_10},
	{GPIOA, GPIO_Pin_1},
	{GPIOG, GPIO_Pin_11},
	{GPIOG, GPIO_Pin_12},
	{GPIOA, GPIO_Pin_2},
	{GPIOB, GPIO_Pin_2}
};

void Switches::Init() {
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);

	GPIO_InitTypeDef gpio;
	gpio.GPIO_Mode = GPIO_Mode_IN;
  gpio.GPIO_OType = GPIO_OType_PP;
  gpio.GPIO_Speed = GPIO_Speed_25MHz;
  gpio.GPIO_PuPd = GPIO_PuPd_UP;

	for (uint8_t i=0; i<kNumSwitches; i++) {
		gpio.GPIO_Pin = pins[i].pin;
		GPIO_Init(pins[i].gpio, &gpio);
	}

  fill(&switch_state_[0], &switch_state_[kNumSwitches], 0xff);
}

void Switches::Debounce() {
	for (uint8_t i=0; i<kNumSwitches; i++) {
    switch_state_[i] = (switch_state_[i] << 1) | \
				GPIO_ReadInputDataBit(pins[i].gpio, pins[i].pin);
	}
}

}  // namespace mtd
