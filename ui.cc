// Copyright 2016 Matthias Puech.
//
// Author: Matthias Puech <matthias.puech@gmail.com>
// Based on code by: Olivier Gillet <ol.gillet@gmail.com>
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
#include "stmlib/utils/random.h"

const int32_t kLongPressDuration = 400;
const int32_t kVeryLongPressDuration = 1200;

using namespace stmlib;

Ui* Ui::instance_;

void reset_observer() {
    Ui::instance_->PingResetLed();
}

void tap_observer(TapType type, float velocity) {
  Ui::instance_->PingMeter(type, velocity);
}

void slot_modified_observer() {
  Ui::instance_->SlotModified();
}

void step_observer(float morph_time) {
  Ui::instance_->SequencerStep(morph_time);
}

void Ui::Init(MultitapDelay* delay, Parameters* parameters) {
  delay_ = delay;
  parameters_ = parameters;
  instance_ = this;

  delay_->reset_observable_.set_observer(&reset_observer);
  delay_->tap_observable_.set_observer(&tap_observer);
  delay_->slot_modified_observable_.set_observer(&slot_modified_observer);
  delay_->step_observable_.set_observer(&step_observer);

  leds_.Init();
  buttons_.Init();
  switches_.Init();
  persistent_.Init();
  control_.Init(delay_, &persistent_.mutable_data()->calibration_data);

  // copy and initialize settings
  for (int i=0; i<4; i++) {
    settings_item_[i] = persistent_.mutable_data()->settings[i];
  }
  ParseSettings();

  // initialization of rest of parameters
  parameters->velocity_type = VELOCITY_AMP;
  parameters->edit_mode = EDIT_NORMAL;

  mode_ = UI_MODE_SPLASH;
  ignore_releases_ = 0;
  velocity_meter_ = -1.0f;

  // calibration
  if (buttons_.pressed(BUTTON_DELETE)) {
    control_.Calibrate();
    persistent_.SaveData();
  }

  current_slot_ = -1;

  // load first slot on startup
  LoadSlot(0);
  sample_counter_to_next_slot_ = 10000.0f; // default fade time
}

void Ui::LoadSlot(uint8_t slot) {
  next_slot_ = slot;
  sample_counter_to_next_slot_ = parameters_->morph;
  delay_->Load(persistent_.mutable_slot(next_slot_));
}

void Ui::SequencerStep(float morph_time) {
  int current = next_slot_ < 0 ? current_slot_ : next_slot_;

  parameters_->morph = morph_time;

  if (current < 0) {
    LoadSlot(6 * bank_ + 0);
  } else {
    uint8_t next =
      parameters_->sequencer_direction == DIRECTION_FORWARD ?
      current + 1 :
      parameters_->sequencer_direction == DIRECTION_WALK ?
      Random::GetWord() & 1 ? current + 1 : current - 1 :
      parameters_->sequencer_direction == DIRECTION_RANDOM ?
      Random::GetWord() : 0;
    next = next % 6;
    LoadSlot(6 * bank_ + next);
  }
}

void Ui::PingGateLed() {
  if (ping_gate_led_counter_ == 0)   // TODO necessary?
    ping_gate_led_counter_ = 8;
}

void Ui::PingResetLed() {
  if (ping_reset_counter_ == 0)   // TODO necessary?
    ping_reset_counter_ = 48;
}

void Ui::PingSaveLed() {
  if (ping_save_led_counter_ == 0)   // TODO necessary?
    ping_save_led_counter_ = 512;
}

void Ui::SlotModified() {
  current_slot_ = -1;
  next_slot_ = -1;
}

void Ui::PingMeter(TapType type, float velocity)
{
  // avoids a "crossed tap" to hide a "added tap" info: "crossed tap"
  // pings are be ignored if another ping is already active
  if (velocity_meter_ > 0.0f &&
      type == TAP_CROSSED &&
      velocity_meter_color_ != COLOR_BLUE)
    return;

  if (velocity > 0.0f) {
    velocity_meter_ = velocity;
    velocity_meter_color_ =
      type == TAP_DRY ? COLOR_BLACK : // TODO
      type == TAP_ADDED ? COLOR_WHITE :
      type == TAP_ADDED_OVERWRITE ? COLOR_MAGENTA :
      type == TAP_ADDED_OVERDUB ? COLOR_YELLOW :
      type == TAP_CROSSED ? COLOR_BLUE :
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

  bool counter = delay_->counter_running() && parameters_->edit_mode != EDIT_OFF;
  bool tap = counter ^ (ping_reset_counter_ > 0);
  leds_.set(LED_TAP, tap);
  bool blink = (animation_counter_ % 4) == 0;
  bool repeat =
    delay_->repeat() <= 0.0f ? false :
    delay_->repeat() < 1.0f ? blink : true;
  leds_.set_repeat(repeat ? COLOR_WHITE : COLOR_BLACK);
  LedColor del = COLOR_BLACK;
  if (delay_->clocked()) del = COLOR_RED;
  if (ping_gate_led_counter_ > 0) del = COLOR_WHITE;
  leds_.set_delete(del);

  // with jumper on High, velocity normalled to High if not in
  // sequencer mode; in sequencer mode (where it controls Morph
  // speed), it is normalled to 0.
  leds_.set(OUT_VELNORM, !sequencer_mode_);

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

    uint8_t slot = save_candidate_slot_ % 6;
    LedColor blink = animation_counter_ & 2 ? COLOR_RED : COLOR_BLACK;
    leds_.set_rgb(slot, blink);
  }
  break;

  case UI_MODE_NORMAL:
  {
    // Tap-meter
    for (int i=0; i<6; i++) {
      if (sequencer_mode_) {
        leds_.set_rgb(i, COLOR_RED);
      } else {
        int color = i <= velocity_meter_ * 6.0f ?
          velocity_meter_color_ : COLOR_BLACK;
        leds_.set_rgb(i, color);
      }
    }

    // Current slot
    if (current_slot_ >= 0) {
      uint8_t bank = current_slot_ / 6;
      uint8_t slot = current_slot_ % 6;
      if (bank == bank_) {
        leds_.set_rgb(slot, COLOR_GREEN);
      }
    }

    // Next slot
    if (next_slot_ >= 0) {
      uint8_t bank = next_slot_ / 6;
      uint8_t slot = next_slot_ % 6;
      LedColor blink = (animation_counter_ % 4) == 0 ? COLOR_GREEN : COLOR_BLACK;
      if (bank == bank_) {
        leds_.set_rgb(slot, blink);
      }
    }

    // Save candidate
    if (save_candidate_slot_ >= 0) {
      uint8_t bank = save_candidate_slot_ / 6;
      uint8_t slot = save_candidate_slot_ % 6;
      if (bank == bank_ && ping_save_led_counter_ > 0) {
        leds_.set_rgb(slot, COLOR_RED);
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
        leds_.set_rgb(i, COLOR_MAGENTA);
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
      leds_.set(i, (animation_counter_ & 8) < 1);
    }
  }
  break;

    default:
      break;
  }

  leds_.Write();

  // Update state variables
  if ((system_clock.milliseconds() & 15) == 0)
    animation_counter_++;

  if (ping_reset_counter_ > 0)
    ping_reset_counter_--;

  if (ping_gate_led_counter_ > 0)
    ping_gate_led_counter_--;

  if (ping_save_led_counter_ > 0)
    ping_save_led_counter_--;

  if (velocity_meter_ > 0.0f) {
    velocity_meter_ -= 0.005f;
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
    parameters_->quality = !p;
  } break;
  }
}

void Ui::SaveSettings()
{
  for (int i=0; i<4; i++) {
    persistent_.mutable_data()->settings[i] = settings_item_[i];
  }
  persistent_.SaveData();
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
      settings_changed_ = true;
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
    if (e.control_id + 6 * bank_ == save_candidate_slot_) {
        PingSaveLed();
        delay_->Save(persistent_.mutable_slot(save_candidate_slot_));
        persistent_.SaveSlot(save_candidate_slot_);
        current_slot_ = save_candidate_slot_;
    }
    mode_ = UI_MODE_NORMAL;
    return;
  }

  if (mode_ == UI_MODE_SETTINGS) {
    if (e.control_id <= BUTTON_6) {
      if (e.data >= kVeryLongPressDuration && !settings_changed_) {
        mode_ = UI_MODE_CONFIRM_SAVE;
        save_candidate_slot_ = bank_ * 6 + e.control_id;
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
        delay_->set_clocked(!delay_->clocked());
      } else {
        delay_->RemoveLastTap();
      }
      break;
    case BUTTON_REPEAT:
      if (e.data >= kLongPressDuration) {
        if (!sequencer_mode_) {
          delay_->Clear();
        }
      } else {
        bool repeat = delay_->repeat() > 0.0f;
        delay_->set_repeat(!repeat);
      }
      break;
    case BUTTON_1:
    case BUTTON_2:
    case BUTTON_3:
    case BUTTON_4:
    case BUTTON_5:
    case BUTTON_6:
      if (e.data >= kVeryLongPressDuration) {
        if ((e.control_id == BUTTON_5 && buttons_.pressed(BUTTON_6)) ||
            (e.control_id == BUTTON_6 && buttons_.pressed(BUTTON_5))) {
          sequencer_mode_ = !sequencer_mode_;
          ignore_releases_ = 1;
        } else {
          save_candidate_slot_ = bank_ * 6 + e.control_id;
          mode_ = UI_MODE_CONFIRM_SAVE;
        }
      }
      else if (e.data >= kLongPressDuration) {
        if (e.control_id <= BUTTON_4) {
          mode_ = UI_MODE_SETTINGS;
          settings_page_ = e.control_id;
        }
      } else {
        LoadSlot(bank_ * 6 + e.control_id);
      }
      break;
    }
  }
}

void Ui::OnSwitchSwitched(const stmlib::Event& e) {
  switch (e.control_id) {
  case SWITCH_EDIT:
    parameters_->edit_mode = static_cast<EditMode>(e.data);
    parameters_->sequencer_direction = static_cast<SequencerDirection>(e.data);
    break;
  case SWITCH_VELO:
    parameters_->velocity_type = static_cast<VelocityType>(e.data);
    break;
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

  if (queue_.idle_time() > 1000 &&
      mode_ == UI_MODE_SETTINGS) {
    mode_ = UI_MODE_NORMAL;
    settings_changed_ = false;
    queue_.Touch();
    SaveSettings();
  }
}
