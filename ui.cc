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
// User interface.

#include "cv_scaler.h"
#include "ui.h"

namespace mtd {

const int32_t kLongPressDuration = 1000;
const int32_t kVeryLongPressDuration = 2500;

using namespace stmlib;

  void Ui::Init(CvScaler* cv_scaler, Clock* clock, Parameters* parameters) {
  cv_scaler_ = cv_scaler;
  parameters_ = parameters;
  clock_ = clock;

  leds_.Init();
  switches_.Init();
  
  mode_ = UI_MODE_SPLASH;
}

void Ui::Start() {
  mode_ = UI_MODE_NORMAL;
}

void Ui::Poll() {
  switches_.Debounce();
  
  for (uint8_t i = 0; i < kNumSwitches; ++i) {
    if (switches_.just_pressed(i)) {
      queue_.AddEvent(CONTROL_SWITCH, i, 0);
      press_time_[i] = system_clock.milliseconds();
      long_press_time_[i] = system_clock.milliseconds();
    }
    if (switches_.pressed(i) && press_time_[i] != 0) {
      int32_t pressed_time = system_clock.milliseconds() - press_time_[i];
      if (pressed_time > kLongPressDuration) {
        queue_.AddEvent(CONTROL_SWITCH, i, pressed_time);
        press_time_[i] = 0;
      }
    }
    if (switches_.pressed(i) && long_press_time_[i] != 0) {
      int32_t pressed_time = system_clock.milliseconds() - long_press_time_[i];
      if (pressed_time > kVeryLongPressDuration) {
        queue_.AddEvent(CONTROL_SWITCH, i, pressed_time);
        long_press_time_[i] = 0;
      }
    }
    
    if (switches_.released(i) && press_time_[i] != 0) {
      queue_.AddEvent(
          CONTROL_SWITCH,
          i,
          system_clock.milliseconds() - press_time_[i] + 1);
      press_time_[i] = 0;
    }
  }
  PaintLeds();
}

void Ui::PaintLeds() {

  if ((system_clock.milliseconds() & 63) == 0)
    animation_counter_++;

  switch (mode_) {
  case UI_MODE_SPLASH:
  {
    for (int i=0; i<kNumLeds; i++){
      leds_.set(i, (animation_counter_ % kNumLeds) == i);
    }
  }
  break;

  case UI_MODE_NORMAL:
  {
    if (clock_->running() && clock_->reset()) {
      ping_led_counter_ = 20;
    }
    if (ping_led_counter_ > 0)
      ping_led_counter_--;

    leds_.set(LED_PING, ping_led_counter_);
    leds_.set(LED_REPEAT1, parameters_->delay[0].repeat);
    leds_.set(LED_REPEAT2, parameters_->delay[1].repeat);
  }
  break;

  case UI_MODE_PANIC:
  {
    for (int i=0; i<kNumLeds; i++){
      leds_.set(i, (animation_counter_ & 7) < 1);
    }
  }
  break;

    default:
      break;
  }

  leds_.Write();
}

void Ui::Panic() {
  mode_ = UI_MODE_PANIC;
}

void Ui::OnSwitchPressed(const Event& e) {
  switch (e.control_id) {
  case SWITCH_PING:
    clock_->Tap();
    break;
  }
}

void Ui::OnSwitchReleased(const Event& e) {
  switch (e.control_id) {

  case SWITCH_PING:
    if (e.data >= kLongPressDuration) {
      clock_->Stop();
    } else {
      clock_->RecordLastTap();
    }
    break;
  case SWITCH_REPEAT1:
    parameters_->delay[0].repeat = !parameters_->delay[0].repeat;
    break;
  case SWITCH_REPEAT2:
    parameters_->delay[1].repeat = !parameters_->delay[1].repeat;
    break;

  }
}

void Ui::DoEvents() {
  while (queue_.available()) {
    Event e = queue_.PullEvent();
    if (e.control_type == CONTROL_SWITCH) {
      if (e.data == 0) {
        OnSwitchPressed(e);
      } else {
        OnSwitchReleased(e);
      }
    }
  }

  if (queue_.idle_time() > 1000) {
    queue_.Touch();
  }
}

}  // namespace mtd
