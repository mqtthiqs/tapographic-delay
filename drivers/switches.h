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
// Driver for the front panel switches.

#ifndef MTD_DRIVERS_SWITCHES_H_
#define MTD_DRIVERS_SWITCHES_H_

#include "stmlib/stmlib.h"

#include <stm32f4xx_conf.h>
#include <algorithm>

namespace mtd {

using namespace std;

const uint8_t kNumSwitches = 9;

enum SwitchNames {
	SWITCH_PING,
	SWITCH_REPEAT1,
	SWITCH_REPEAT2,
	SWITCH_REV1,
	SWITCH_REV2,
	SWITCH_TIME1_1,
	SWITCH_TIME1_2,
	SWITCH_TIME2_1,
	SWITCH_TIME2_2
};

struct PinAssign {
	GPIO_TypeDef* gpio;
	uint16_t pin;
};

const PinAssign pins[kNumSwitches] = {
	{GPIOE, GPIO_Pin_5},
	{GPIOC, GPIO_Pin_13},
	{GPIOA, GPIO_Pin_9},
	{GPIOG, GPIO_Pin_10},
	{GPIOA, GPIO_Pin_1},
	{GPIOG, GPIO_Pin_11},
	{GPIOG, GPIO_Pin_12},
	{GPIOA, GPIO_Pin_2},
	{GPIOB, GPIO_Pin_2}
};

class Switches {
 public:
  Switches() { }
  ~Switches() { }
  

void Init() {
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
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

void Debounce() {
	for (uint8_t i=0; i<kNumSwitches; i++) {
    switch_state_[i] = (switch_state_[i] << 1) | \
				GPIO_ReadInputDataBit(pins[i].gpio, pins[i].pin);
	}
}

  inline bool released(uint8_t index) const {
    return switch_state_[index] == 0x7f;
  }
  
  inline bool just_pressed(uint8_t index) const {
    return switch_state_[index] == 0x80;
  }

  inline bool pressed(uint8_t index) const {
    return switch_state_[index] == 0x00;
  }

  inline uint8_t state1() {
    return pressed(SWITCH_TIME1_1) << 1 | pressed(SWITCH_TIME1_2);
  }

  inline uint8_t state2() {
    return pressed(SWITCH_TIME2_1) << 1 | pressed(SWITCH_TIME2_2);
  }

 private:
  uint8_t switch_state_[kNumSwitches];
  
  DISALLOW_COPY_AND_ASSIGN(Switches);
};

}  // namespace mtd

#endif  // MTD_DRIVERS_SWITCHES_H_
