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
const int32_t kVeryLongPressDuration = 1000;

using namespace stmlib;

void Ui::Init(MultitapDelay* mtd, Parameters* parameters) {
  delay_ = mtd;
  parameters_ = parameters;

  leds_.Init();
  buttons_.Init();
  switches_.Init();
  persistent_.Init();
  cv_scaler_.Init();

  // copy and initialize settings
  for (int i=0; i<4; i++) {
    settings_item_[i] = persistent_.mutable_data()->settings[i];
  }
  ParseSettings();

  // initialization of rest of parameters
  parameters->velocity_type = VELOCITY_AMP;
  parameters->edit_mode = EDIT_NORMAL;
  // parameters->counter_running = true; // TODO

  mode_ = UI_MODE_SPLASH;
  ignore_releases_ = 0;
  velocity_meter_ = -1.0f;

  // calibration
  if (buttons_.pressed_immediate(BUTTON_REPEAT) &&
      buttons_.pressed_immediate(BUTTON_DELETE)) {
    cv_scaler_.Calibrate(&persistent_);
  }

  current_slot_ = -1;           // means no active slot
}

void Ui::PingGateLed() {
  if (ping_gate_led_counter_ == 0)   // TODO necessary?
    ping_gate_led_counter_ = 8;
}

void Ui::PingResetLed() {
  if (ping_reset_counter_ == 0)   // TODO necessary?
    ping_reset_counter_ = 24;
}

void Ui::PingSaveLed() {
  if (ping_save_led_counter_ == 0)   // TODO necessary?
    ping_save_led_counter_ = 512;
}

void Ui::PingMeter(float velocity, TapType type)
{
  if (velocity > 0.0f) {
    velocity_meter_ = velocity;
    velocity_meter_color_ =
      type == TAP_DRY ? COLOR_BLACK : // TODO
      type == TAP_NORMAL ? COLOR_WHITE :
      type == TAP_OVERWRITE ? COLOR_MAGENTA :
      type == TAP_OVERDUB ? COLOR_YELLOW :
      COLOR_RED;
  }
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

  bool tap = delay_->counter_running() ^ (ping_reset_counter_ > 0);
  leds_.set(LED_TAP, tap);
  leds_.set(LED_REPEAT, parameters_->repeat);
  leds_.set(LED_DELETE, ping_gate_led_counter_);

  switch (mode_) {
  case UI_MODE_SPLASH:
  {
    for (int i=0; i<kNumLeds; i++) {
      leds_.set(i, (animation_counter_ % kNumLeds) == i);
    }
  }
  break;

  case UI_MODE_CONFIRM_SAVE:
  {
    for (int i=0; i<6; i++) {
      leds_.set_rgb(i, COLOR_BLACK);
    }

    uint8_t slot = current_slot_ % 6;
    LedColor blink = animation_counter_ & 1 ? COLOR_RED : COLOR_BLACK;
    leds_.set_rgb(slot, blink);
  }
  break;

  case UI_MODE_NORMAL:
  {
    for (int i=0; i<6; i++) {
      int color = i <= velocity_meter_ * 6.0f ?
        velocity_meter_color_ : COLOR_BLACK;
      leds_.set_rgb(i, color);
    }

    if (current_slot_ >= 0) {
      uint8_t bank = current_slot_ / 6;
      uint8_t slot = current_slot_ % 6;
      if (bank == bank_) {
        LedColor c = ping_save_led_counter_ > 0 ? COLOR_RED : COLOR_GREEN;
        leds_.set_rgb(slot, c);
      }
    }
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

  if (ping_reset_counter_ > 0)
    ping_reset_counter_--;

  if (ping_gate_led_counter_ > 0)
    ping_gate_led_counter_--;

  if (ping_save_led_counter_ > 0)
    ping_save_led_counter_--;

  if (parameters_->slot_modified) {
    current_slot_ = -1;
    parameters_->slot_modified = false;
  }

  if (velocity_meter_ > 0.0f) {
    velocity_meter_ -= 0.01f;
  }
}

void Ui::Panic() {
  mode_ = UI_MODE_PANIC;
}

void Ui::ParseSettings() {
  for (int i=0; i<4; i++) {
    settings_page_ = i;
    ParseSettingsCurrentPage();
  }
}

void Ui::ParseSettingsCurrentPage() {
  int p = settings_item_[settings_page_];
  switch (settings_page_) {
  case PAGE_VELOCITY_PARAMETER: {
    parameters_->velocity_parameter = static_cast<float>(p) / 4.0f;
  } break;
  case PAGE_BANK: {
    bank_ = p;
  } break;
  case PAGE_PANNING_MODE: {
    parameters_->panning_mode = static_cast<PanningMode>(p);
    delay_->RepanTaps(parameters_->panning_mode);
  } break;
  case PAGE_QUALITY: {
    parameters_->quality = p;
  } break;
  }
}

void Ui::SaveSettings()
{
  for (int i=0; i<4; i++) {
    persistent_.mutable_data()->settings[i] = settings_item_[i];
  }
  persistent_.SaveSettings();
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
      ParseSettingsCurrentPage();
    }
  }
}

void Ui::OnButtonReleased(const Event& e) {
  // hack for double presses
  if (ignore_releases_ > 0) {
    ignore_releases_--;
    return;
  }

  if (mode_ == UI_MODE_CONFIRM_SAVE) {
    if (e.control_id + 6 * bank_ == current_slot_) {
        PingSaveLed();
        delay_->Save(persistent_.mutable_slot(current_slot_));
        persistent_.SaveSlot(current_slot_);
    }
    mode_ = UI_MODE_NORMAL;
    return;
  }

  if (mode_ == UI_MODE_SETTINGS) {
    if (e.control_id <= BUTTON_6) {
      if (e.data >= kVeryLongPressDuration) {
        current_slot_ = bank_ * 6 + e.control_id;
        mode_ = UI_MODE_CONFIRM_SAVE;
      }
      // upper buttons
      else if (e.data >= kLongPressDuration && e.control_id <= BUTTON_4) {
        // long press -> change page
        settings_page_ = e.control_id;
      } else if (e.control_id <= settings_page_) {
        // short press on the left -> change page
        settings_page_ = e.control_id;
      } else {
        // short press on the right -> change setting
        settings_item_[settings_page_] = e.control_id - settings_page_ - 1;
        ParseSettingsCurrentPage();
      }
    } else {
      // Delete and Repeat exit settings mode
      mode_ = UI_MODE_NORMAL;   // control flow continues below
    }
  }

  // normal mode (no else):
  if (mode_ == UI_MODE_NORMAL) {
    switch (e.control_id) {
    case BUTTON_DELETE:
      if (e.data >= kLongPressDuration) {
        delay_->Clear();
        parameters_->slot_modified = true;
      } else {
        if (delay_->RemoveLastTap()) {
          parameters_->slot_modified = true;
        }
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
      if (e.data >= kVeryLongPressDuration) {
        current_slot_ = bank_ * 6 + e.control_id;
        mode_ = UI_MODE_CONFIRM_SAVE;
      }
      else if (e.data >= kLongPressDuration) {
        if (e.control_id <= BUTTON_4) {
          mode_ = UI_MODE_SETTINGS;
          settings_page_ = e.control_id;
        }
      } else {
        current_slot_ = bank_ * 6 + e.control_id;
        delay_->Load(persistent_.mutable_slot(current_slot_));
      }
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

  if (queue_.idle_time() > 1500 &&
      mode_ == UI_MODE_CONFIRM_SAVE) {
    mode_ = UI_MODE_NORMAL;
    queue_.Touch();
  }

  if (queue_.idle_time() > 800 &&
      mode_ == UI_MODE_SETTINGS) {
    mode_ = UI_MODE_NORMAL;
    queue_.Touch();
    SaveSettings();
  }
}
