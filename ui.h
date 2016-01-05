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

#ifndef MULTITAP_UI_H_
#define MULTITAP_UI_H_

#include "stmlib/stmlib.h"
#include "stmlib/ui/event_queue.h"

#include "drivers/leds.h"
#include "drivers/switches.h"

namespace multitap {

enum UiMode {
  UI_MODE_SPLASH,
  UI_MODE_NORMAL,
  UI_MODE_PANIC,
  UI_MODE_LAST
};

class GranularProcessor;
class CvScaler;
class Meter;
class Settings;

class Ui {
 public:
  Ui() { }
  ~Ui() { }
  
  void Init(CvScaler* cv_scaler);
  void Poll();
  void DoEvents();
  void Start();
  void Panic();

 private:
  void OnSwitchPressed(const stmlib::Event& e);
  void OnSwitchReleased(const stmlib::Event& e);

  void SaveState();
  void PaintLeds();

  stmlib::EventQueue<16> queue_;

  CvScaler* cv_scaler_;
  
  Leds leds_;
  Switches switches_;
  uint32_t press_time_[kNumSwitches];
  uint32_t long_press_time_[kNumSwitches];
  UiMode mode_;
  uint16_t animation_counter_;

  DISALLOW_COPY_AND_ASSIGN(Ui);
};

}  // namespace multitap

#endif  // MULTITAP_UI_H_