// Copyright 2015 Matthias Puech.
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
// Driver for the status LEDs.

#ifndef LEDS_H_
#define LEDS_H_

#include "stmlib/stmlib.h"

#include <stm32f4xx_conf.h>

const uint8_t kNumLeds = 7;

enum LedNames {
	LED_PING,
	LED_REPEAT1,
	LED_REPEAT2,
	LED_CH1,
	LED_CH2,
	LED_REV1,
	LED_REV2,
};

class Leds {
 public:
  Leds() { }
  ~Leds() { }

  void Init() {
    GPIO_InitTypeDef gpio;
    GPIO_StructInit(&gpio);
    gpio.GPIO_Mode = GPIO_Mode_OUT;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
    gpio.GPIO_Pin = GPIO_Pin_4; // ping
    GPIO_Init(GPIOE, &gpio);

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    gpio.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_2; // inf1, rev2
    GPIO_Init(GPIOD, &gpio);

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    gpio.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11 |
      GPIO_Pin_12 | GPIO_Pin_15; // inf2, ch1, ch2, rev1
    GPIO_Init(GPIOA, &gpio);

    for (int i=0; i<kNumLeds; i++)
      values_[i] = 0;
    Write();
  }

  void Write() {
    GPIO_WriteBit(GPIOE, GPIO_Pin_4, static_cast<BitAction>(values_[0])); // ping
    GPIO_WriteBit(GPIOD, GPIO_Pin_4, static_cast<BitAction>(values_[1])); // inf1
    GPIO_WriteBit(GPIOA, GPIO_Pin_10, static_cast<BitAction>(values_[2])); // inf2
    GPIO_WriteBit(GPIOA, GPIO_Pin_11, static_cast<BitAction>(values_[3])); // ch1
    GPIO_WriteBit(GPIOA, GPIO_Pin_12, static_cast<BitAction>(values_[4])); // ch2
    GPIO_WriteBit(GPIOA, GPIO_Pin_15, static_cast<BitAction>(values_[5])); // rev1
    GPIO_WriteBit(GPIOD, GPIO_Pin_2, static_cast<BitAction>(values_[6])); // rev2
  }

  void set(uint8_t channel, bool value) {
		values_[channel] = value;
  }

 private:
  bool values_[kNumLeds];

  DISALLOW_COPY_AND_ASSIGN(Leds);
};

#endif