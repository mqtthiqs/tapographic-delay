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

#include "cv_scaler.hh"
#include "ui.hh"

const int32_t kLongPressDuration = 1000;
const int32_t kVeryLongPressDuration = 2500;

using namespace stmlib;

  void Ui::Init(CvScaler* cv_scaler, MultitapDelay* mtd, Clock* clock, Parameters* parameters) {
  cv_scaler_ = cv_scaler;
  multitap_delay_ = mtd;
  parameters_ = parameters;
  clock_ = clock;

  leds_.Init();
  switches_.Init();
  
  mode_ = UI_MODE_SPLASH;
  ignore_releases_ = 0;

  // parameters initialization
  parameters->scale = 1.0f; //center to avoid slope on startup
  parameters->edit_mode = EDIT_NORMAL;
  parameters->quantize = QUANTIZE_NONE;
  parameters->panning = PANNING_RANDOM;
  parameters->velocity_type = VELOCITY_AMP;
  parameters->edit_mode = EDIT_NORMAL;
  parameters->repeat = false;
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

  if (mode_ == UI_MODE_NORMAL) {
    parameters_->edit_mode = static_cast<EditMode>(switches_.state1());
    parameters_->quantize = static_cast<Quantize>(switches_.state2());
  } else if (mode_ == UI_MODE_SETTINGS) {
    parameters_->panning = static_cast<Panning>(switches_.state1());
    parameters_->velocity_type = static_cast<VelocityType>(switches_.state2());
  }

  PaintLeds();
}

inline void Ui::PaintLeds() {

  if ((system_clock.milliseconds() & 63) == 0)
    animation_counter_++;

  switch (mode_) {
  case UI_MODE_SPLASH:
  {
    for (int i=0; i<kNumLeds; i++) {
      leds_.set(i, (animation_counter_ % kNumLeds) == i);
    }
  }
  break;

  case UI_MODE_NORMAL:
  {
    if (clock_->running() && clock_->reset()) {
      ping_led_counter_ = 40;
    }

    if (ping_led_counter_ > 0)
      ping_led_counter_--;

    if (beat_led_counter_ > 0)
      beat_led_counter_--;

    // leds_.set(LED_PING, ping_led_counter_);
    // leds_.set(LED_REPEAT1, beat_led_counter_);
    // leds_.set(LED_REPEAT2, parameters_->repeat);
  }
  break;

  case UI_MODE_SETTINGS:
  {
    // leds_.set(LED_PING, animation_counter_ & 4);

    // bool pan_led =
    //   parameters_->panning == 0 ? false :
    //   parameters_->panning == 1 ? animation_counter_ & 1 :
    //   true;

    // bool vel_led =
    //   parameters_->velocity_type == 0 ? false :
    //   parameters_->velocity_type == 1 ? animation_counter_ & 1 :
    //   true;

    // leds_.set(LED_REPEAT1, pan_led);
    // leds_.set(LED_REPEAT2, vel_led);

    break;
  }

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

inline void Ui::OnSwitchPressed(const Event& e) {

  // double press -> feature switch mode
  if ((e.control_id == SWITCH_REPEAT1
       && switches_.pressed(SWITCH_REPEAT2)) ||
      (e.control_id == SWITCH_REPEAT2
       && switches_.pressed(SWITCH_REPEAT1))) {
    if (mode_ == UI_MODE_NORMAL)
      mode_ = UI_MODE_SETTINGS;
    else if (mode_ == UI_MODE_SETTINGS)
      mode_ = UI_MODE_NORMAL;
    ignore_releases_ = 2;
  } else if (mode_ == UI_MODE_NORMAL) {
    // normal mode:
    switch (e.control_id) {
    case SWITCH_PING:
      clock_->Tap();
      break;
    case SWITCH_REPEAT1:
      multitap_delay_->AddTap(parameters_->velocity,
                              parameters_->edit_mode,
                              parameters_->quantize,
                              parameters_->panning);
      break;
    }
  }
}

inline void Ui::OnSwitchReleased(const Event& e) {
  // hack for double presses
  if (ignore_releases_ > 0) {
    ignore_releases_--;
    return;
  }

  // normal mode:
  if (mode_ == UI_MODE_NORMAL) {
    switch (e.control_id) {
    case SWITCH_PING:
      if (e.data >= kLongPressDuration) {
        clock_->Stop();
      } else {
        clock_->RecordLastTap();
      }
      break;
    case SWITCH_REV1:
      if (e.data >= kLongPressDuration) {
        multitap_delay_->Clear();
      } else {
        multitap_delay_->RemTap();
      }
      break;
    case SWITCH_REPEAT2:
      parameters_->repeat = !parameters_->repeat;
      break;
    case SWITCH_REPEAT1:
      break;
    }
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
