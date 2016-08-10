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

#include "ui.hh"

const int32_t kLongPressDuration = 400;
const int32_t kVeryLongPressDuration = 2000;

using namespace stmlib;

void Ui::Init(MultitapDelay* mtd, Parameters* parameters) {
  delay_ = mtd;
  parameters_ = parameters;

  cv_scaler_.Init();
  leds_.Init();
  buttons_.Init();
  switches_.Init();

  mode_ = UI_MODE_SPLASH;
  ignore_releases_ = 0;
  velocity_meter_ = -1.0f;

  // parameters initialization
  parameters->scale = 1.0f; //center to avoid slope on startup
  parameters->edit_mode = EDIT_NORMAL;
  parameters->quantize = QUANTIZE_NONE;
  parameters->panning_mode = PANNING_RANDOM;
  parameters->velocity_type = VELOCITY_AMP;
  parameters->edit_mode = EDIT_NORMAL;
  parameters->repeat = false;
}

void Ui::PingGateLed() {
  if (ping_led_counter_ == 0)   // TODO necessary?
    ping_led_counter_ = 3;
}


void Ui::Start() {
  mode_ = UI_MODE_NORMAL;
}

void Ui::Poll() {
  buttons_.Debounce();
  switches_.Read();
  
  for (uint8_t i=0; i<kNumButtons; i++) {
    if (buttons_.just_pressed(i)) {
      queue_.AddEvent(CONTROL_SWITCH, i, 0);
      press_time_[i] = system_clock.milliseconds();
      long_press_time_[i] = system_clock.milliseconds();
    }
    if (buttons_.pressed(i) && press_time_[i] != 0) {
      int32_t pressed_time = system_clock.milliseconds() - press_time_[i];
      if (pressed_time > kLongPressDuration) {
        queue_.AddEvent(CONTROL_SWITCH, i, pressed_time);
        press_time_[i] = 0;
      }
    }
    if (buttons_.pressed(i) && long_press_time_[i] != 0) {
      int32_t pressed_time = system_clock.milliseconds() - long_press_time_[i];
      if (pressed_time > kVeryLongPressDuration) {
        queue_.AddEvent(CONTROL_SWITCH, i, pressed_time);
        long_press_time_[i] = 0;
      }
    }
    
    if (buttons_.released(i) && press_time_[i] != 0) {
      queue_.AddEvent(
          CONTROL_SWITCH,
          i,
          system_clock.milliseconds() - press_time_[i] + 1);
      press_time_[i] = 0;
    }
  }

  for (uint8_t i=0; i<kNumSwitches; i++) {
    if (switches_.switched(i)) {
      queue_.AddEvent(CONTROL_ENCODER, i, switches_.state(i));
    }
  }

  PaintLeds();
}

inline void Ui::PaintLeds() {

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
    for (int i=0; i<6; i++) {
      int color = i <= velocity_meter_ * 6.0f ?
        velocity_meter_color_ : COLOR_BLACK;
      leds_.set_rgb(i, color);
    }

    leds_.set(LED_TAP, delay_->counter_running());
    leds_.set(LED_DELETE, ping_led_counter_);
    leds_.set(LED_REPEAT, parameters_->repeat);
  }
  break;

  case UI_MODE_SETTINGS:
  {
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

  // Update state variables
  if ((system_clock.milliseconds() & 63) == 0)
    animation_counter_++;

  if (ping_led_counter_ > 0)
    ping_led_counter_--;

  float v = delay_->tap_velocity_just_added();
  if (v > 0.0f) {
    velocity_meter_ = v;
    TapType t = delay_->last_tap_type();
    velocity_meter_color_ =
      t == TAP_DRY ? COLOR_BLACK : // TODO necessary?
      t == TAP_NORMAL ? COLOR_WHITE :
      t == TAP_OVERWRITE ? COLOR_MAGENTA :
      t == TAP_OVERDUB ? COLOR_YELLOW :
      COLOR_RED;
  } else if (velocity_meter_ > 0.0f) {
    velocity_meter_ -= 0.01f;
  }

}

void Ui::Panic() {
  mode_ = UI_MODE_PANIC;
}

void Ui::OnButtonPressed(const Event& e) {

  if (mode_ == UI_MODE_NORMAL) {
    switch (e.control_id) {
    }
  }
}

void Ui::OnButtonReleased(const Event& e) {
  // hack for double presses
  if (ignore_releases_ > 0) {
    ignore_releases_--;
    return;
  }

  // normal mode:
  if (mode_ == UI_MODE_NORMAL) {
    switch (e.control_id) {
    case BUTTON_DELETE:
      if (e.data >= kLongPressDuration) {
        delay_->Clear();
      } else {
        delay_->RemTap();
      }
      break;
    case BUTTON_REPEAT:
      parameters_->repeat = !parameters_->repeat;
      break;
    case BUTTON_1:
    case BUTTON_2:
    case BUTTON_3:
    case BUTTON_4:
    case BUTTON_5:
    case BUTTON_6:
      break;
    }
  }
}

void Ui::OnSwitchSwitched(const stmlib::Event& e) {
  if (mode_ == UI_MODE_NORMAL) {
    switch (e.control_id) {
    case SWITCH_EDIT:
      parameters_->edit_mode = static_cast<EditMode>(e.data);
      break;
    case SWITCH_VELO:
      parameters_->velocity_type = static_cast<VelocityType>(e.data);
      break;
    }
  }
}

void Ui::DoEvents() {
  while (queue_.available()) {
    Event e = queue_.PullEvent();
    if (e.control_type == CONTROL_SWITCH) {
      if (e.data == 0) {
        OnButtonPressed(e);
      } else {
        OnButtonReleased(e);
      }
    } else if (e.control_type == CONTROL_ENCODER) {
      OnSwitchSwitched(e);
    }
  }

  if (queue_.idle_time() > 1000) {
    queue_.Touch();
  }
}
