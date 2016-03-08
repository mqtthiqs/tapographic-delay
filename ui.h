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
// User interface

#ifndef MTD_UI_H_
#define MTD_UI_H_

#include "stmlib/stmlib.h"
#include "stmlib/ui/event_queue.h"

#include "drivers/leds.h"
#include "drivers/switches.h"
#include "parameters.h"
#include "clock.h"
#include "multitap_delay.h"

namespace mtd {

enum UiMode {
  UI_MODE_SPLASH,
  UI_MODE_NORMAL,
  UI_MODE_PANIC,
  UI_MODE_SETTINGS,
  UI_MODE_LAST
};

class CvScaler;
class Meter;
class Settings;

class Ui {
 public:
  Ui() { }
  ~Ui() { }
  
  void Init(CvScaler* cv_scaler, MultitapDelay* mtd, Clock* clock, Parameters* parameters);
  void Poll();
  void DoEvents();
  void Start();
  void Panic();

  inline void set_beat_led(bool value) {
    if (value) beat_led_counter_ = 20;
  }

 private:
  void OnSwitchPressed(const stmlib::Event& e);
  void OnSwitchReleased(const stmlib::Event& e);

  void PaintLeds();

  stmlib::EventQueue<16> queue_;

  CvScaler* cv_scaler_;
  Clock* clock_;
  MultitapDelay* multitap_delay_;
  Parameters* parameters_;
  
  Leds leds_;
  uint16_t ping_led_counter_;
  uint16_t beat_led_counter_;

  Switches switches_;
  uint32_t press_time_[kNumSwitches];
  uint32_t long_press_time_[kNumSwitches];
  UiMode mode_;
  uint16_t animation_counter_;

  uint16_t ignore_releases_;

  DISALLOW_COPY_AND_ASSIGN(Ui);
};

}  // namespace mtd

#endif  // MTD_UI_H_
