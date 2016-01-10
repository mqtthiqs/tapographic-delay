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

#ifndef MTD_DRIVERS_LEDS_H_
#define MTD_DRIVERS_LEDS_H_

#include "stmlib/stmlib.h"

namespace mtd {

const uint8_t kNumLeds = 5;

enum LedNames {
	LED_PING,
	LED_REPEAT1,
	LED_REPEAT2,
	LED_CH1,
	LED_CH2,
};

class Leds {
 public:
  Leds() { }
  ~Leds() { }
  
  void Init();

  void set(uint8_t channel, bool value) {
		values_[channel] = value;
  }

  void Write();
  
 private:
  bool values_[kNumLeds];

  DISALLOW_COPY_AND_ASSIGN(Leds);
};

}  // namespace mtd

#endif  // MTD_DRIVERS_LEDS_H_
