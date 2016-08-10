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

const int32_t kLongPressDuration = 300;
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
  parameters->counter_running = true;
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

    leds_.set(LED_TAP, parameters_->counter_running);
    leds_.set(LED_REPEAT, parameters_->repeat);
    leds_.set(LED_DELETE, ping_led_counter_);
  }
  break;

  case UI_MODE_SETTINGS:
  {
    for (int i=0; i<6; i++) {
      int page = settings_page_;
      int item = settings_item_[settings_page_];
      if (i == settings_page_) {
        leds_.set_rgb(i, COLOR_BLUE);
      } else if (i == page + item + 1) {
        leds_.set_rgb(i, COLOR_CYAN);
      } else {
        leds_.set_rgb(i, COLOR_BLACK);
      }
    }

    leds_.set(LED_TAP, parameters_->counter_running);
    leds_.set(LED_REPEAT, parameters_->repeat);
    leds_.set(LED_DELETE, ping_led_counter_);
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

  float v = parameters_->last_tap_velocity;
  parameters_->last_tap_velocity = 0.0f;

  if (v > 0.0f) {
    velocity_meter_ = v;
    TapType t = parameters_->last_tap_type;
    velocity_meter_color_ =
      t == TAP_DRY ? COLOR_BLACK : // TODO
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

void Ui::ParseSettings() {
  switch (settings_page_) {
  case PAGE_VELOCITY_PARAMETER: {
    float p = static_cast<float>(settings_item_[PAGE_VELOCITY_PARAMETER]) / 4.0f;
    parameters_->velocity_parameter = p;
  } break;
  case PAGE_BANK: {
    int b = settings_item_[PAGE_VELOCITY_PARAMETER];
    parameters_->bank = b;
  }
  case PAGE_PANNING_MODE: {
  }
  case PAGE_QUALITY: {
    bool q = settings_item_[PAGE_VELOCITY_PARAMETER];
    parameters_->quality = q;
  } break;
  }
}

void Ui::OnButtonPressed(const Event& e) {

  if (e.control_id <= BUTTON_6) {
    // scan other pressed buttons on the left
    int pressed = -1;
    for (int i=0; i<e.control_id; i++) {
      if (buttons_.pressed(i) && i<=BUTTON_4) pressed = i;
    }
    if (pressed != -1) {
      // double press
      mode_ = UI_MODE_SETTINGS;
      ignore_releases_ = 2;
      settings_page_ = pressed;
      settings_item_[pressed] = e.control_id - pressed - 1;
      ParseSettings();
    }
  }
}

void Ui::OnButtonReleased(const Event& e) {
  // hack for double presses
  if (ignore_releases_ > 0) {
    ignore_releases_--;
    return;
  }

  if (mode_ == UI_MODE_SETTINGS) {
    if (e.data >= kLongPressDuration
        && e.control_id <= BUTTON_4) {
      settings_page_ = e.control_id;
    } else if (e.control_id <= settings_page_) {
      settings_page_ = e.control_id;
    } else {
      settings_item_[settings_page_] = e.control_id - settings_page_ - 1;
    }
  }

  // normal mode:
  if (mode_ == UI_MODE_NORMAL) {
    switch (e.control_id) {
    case BUTTON_DELETE:
      if (e.data >= kLongPressDuration) {
        delay_->Clear(parameters_);
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
      if (e.data >= kLongPressDuration) {
        mode_ = UI_MODE_SETTINGS;
        settings_page_ = e.control_id;
      }
      break;
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

  if (queue_.idle_time() > 700) {
    queue_.Touch();
    if (mode_ == UI_MODE_SETTINGS) {
      mode_ = UI_MODE_NORMAL;
    }
  }
}
