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

#ifndef UI_H_
#define UI_H_

#include "stmlib/stmlib.h"
#include "stmlib/ui/event_queue.h"

#include "drivers/leds.hh"
#include "drivers/buttons.hh"
#include "drivers/switches.hh"
#include "parameters.hh"
#include "clock.hh"
#include "multitap_delay.hh"

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

 private:
  void OnButtonPressed(const stmlib::Event& e);
  void OnButtonReleased(const stmlib::Event& e);
  void OnSwitchSwitched(const stmlib::Event& e);

  void PaintLeds();

  stmlib::EventQueue<16> queue_;

  CvScaler* cv_scaler_;
  Clock* clock_;
  MultitapDelay* multitap_delay_;
  Parameters* parameters_;
  
  Leds leds_;
  uint16_t last_button_pressed_; // TODO temp

  Buttons buttons_;
  Switches switches_;
  uint32_t press_time_[kNumButtons];
  uint32_t long_press_time_[kNumButtons];
  UiMode mode_;
  uint16_t animation_counter_;

  uint16_t ignore_releases_;

  DISALLOW_COPY_AND_ASSIGN(Ui);
};

#endif
