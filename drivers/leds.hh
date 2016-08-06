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

const uint8_t kNumLeds = 20;

enum LedNames {
  LED_BUT1_R,
  LED_BUT1_G,
  LED_BUT1_B,
  LED_BUT2_R,
  LED_BUT2_G,
  LED_BUT2_B,
  LED_BUT3_R,
  LED_BUT3_G,
  LED_BUT3_B,
  LED_BUT4_R,
  LED_BUT4_G,
  LED_BUT4_B,
  LED_BUT5_R,
  LED_BUT5_G,
  LED_BUT5_B,
  LED_BUT6_R,
  LED_BUT6_G,
  LED_BUT6_B,
  LED_DELETE,
  LED_REPEAT,
};

static uint16_t const LED_Pins[20] = {
    GPIO_Pin_6,
    GPIO_Pin_13,
    GPIO_Pin_14,
    GPIO_Pin_2,
		GPIO_Pin_5,
    GPIO_Pin_5,
    GPIO_Pin_13,
		GPIO_Pin_12,
		GPIO_Pin_11,
    GPIO_Pin_7,
		GPIO_Pin_6,
		GPIO_Pin_3,
    GPIO_Pin_9,
		GPIO_Pin_8,
		GPIO_Pin_7,
		GPIO_Pin_10,
    GPIO_Pin_9,
    GPIO_Pin_8,

    GPIO_Pin_10,
    GPIO_Pin_12,
};

static uint16_t const LED_PinSources[20] = {
    GPIO_PinSource6,
    GPIO_PinSource13,
    GPIO_PinSource14,
    GPIO_PinSource2,
		GPIO_PinSource5,
    GPIO_PinSource5,
    GPIO_PinSource13,
		GPIO_PinSource12,
		GPIO_PinSource11,
    GPIO_PinSource7,
		GPIO_PinSource6,
		GPIO_PinSource3,
    GPIO_PinSource9,
		GPIO_PinSource8,
		GPIO_PinSource7,
		GPIO_PinSource10,
    GPIO_PinSource9,
    GPIO_PinSource8,
    GPIO_PinSource10,
    GPIO_PinSource12,
};

static GPIO_TypeDef* const LED_GPIOs[20] = {
    GPIOE,
		GPIOC,
    GPIOC,
    GPIOB,
		GPIOC,
		GPIOE,
    GPIOD,
		GPIOD,
		GPIOD,
    GPIOG,
		GPIOG,
		GPIOG,
    GPIOC,
		GPIOC,
		GPIOC,
		GPIOA,
		GPIOA,
		GPIOA,

    GPIOC,
    GPIOC,
};

class Leds {
 public:
  Leds() { }
  ~Leds() { }

  void Init() {

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA
                           | RCC_AHB1Periph_GPIOB
                           | RCC_AHB1Periph_GPIOC
                           | RCC_AHB1Periph_GPIOD
                           | RCC_AHB1Periph_GPIOE
                           | RCC_AHB1Periph_GPIOG, ENABLE);

    GPIO_InitTypeDef gpio;
    GPIO_StructInit(&gpio);
    gpio.GPIO_Mode = GPIO_Mode_OUT;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;

    // LED pins configure
    for (int i=0; i<kNumLeds; i++) {
      gpio.GPIO_Pin = LED_Pins[i];
      GPIO_Init(LED_GPIOs[i], &gpio);
      values_[i] = 255;
    }

    Write();
  }

  void Write() {
    for (int i=0; i<kNumLeds; i++) {
      bool v = i<18 ? !values_[i] : values_[i];
      GPIO_WriteBit(LED_GPIOs[i], LED_Pins[i], static_cast<BitAction>(v));
    }
  }

  void set(uint8_t channel, bool value) {
    values_[channel] = value;
  }

 private:
  bool values_[kNumLeds];

  DISALLOW_COPY_AND_ASSIGN(Leds);
};

#endif
