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
// Persistent data

#ifndef PERSISTENT_H_
#define PERSISTENT_H_

#include "parameters.hh"
#include "resources.h"
#include "stmlib/system/storage.h"

const int kNumSlots = 6 * 4;    // 6 buttons, 4 banks

struct CalibrationData {
  float offset[4];
};

class Persistent 
{
public:
  Persistent() { };
  ~Persistent() { };

  struct Data {
    uint8_t settings[4];
    CalibrationData calibration_data;
  };

  void Init(size_t buffer_size) {

    if (!settings_storage_.ParsimoniousLoad(&data_, &settings_token_)) {
      for (size_t i=0; i<4; i++) {
        data_.calibration_data.offset[i] = 0.5f;
      }
      data_.settings[0] = 2;      // velocity parameter (0-4)
      data_.settings[1] = 0;      // bank (0-3)
      data_.settings[2] = 1;      // panning mode (0-2)
      data_.settings[3] = 0;      // quality (0-1)
      SaveData();
    }

    // sanitize settings
    CONSTRAIN(data_.settings[0], 0, 4);
    CONSTRAIN(data_.settings[1], 0, 3);
    CONSTRAIN(data_.settings[2], 0, 2);
    CONSTRAIN(data_.settings[3], 0, 1);

    if (!bank0_.ParsimoniousLoad(&slots_[6 * 0], 6 * sizeof(Slot), &token_[0]) ||
        !bank1_.ParsimoniousLoad(&slots_[6 * 1], 6 * sizeof(Slot), &token_[1]) ||
        !bank2_.ParsimoniousLoad(&slots_[6 * 2], 6 * sizeof(Slot), &token_[2]) ||
        !bank3_.ParsimoniousLoad(&slots_[6 * 3], 6 * sizeof(Slot), &token_[3])) {

      // clear slots
      memset(slots_, 0, sizeof(slots_));

      for (int slot=0; slot<kNumSlots; slot++) {
        ResetSlot(slot);
      }

      // save new slots
      bank0_.ParsimoniousSave(&slots_[6 * 0], 6 * sizeof(Slot), &token_[0]);
      bank1_.ParsimoniousSave(&slots_[6 * 1], 6 * sizeof(Slot), &token_[1]);
      bank2_.ParsimoniousSave(&slots_[6 * 2], 6 * sizeof(Slot), &token_[2]);
      bank3_.ParsimoniousSave(&slots_[6 * 3], 6 * sizeof(Slot), &token_[3]);
    }

    // sanitize slots
    for (int slot=0; slot<kNumSlots; slot++) {
      CONSTRAIN(slots_[slot].size, 0, kMaxTaps);
      for (int tap=0; tap<slots_[slot].size; tap++) {
        TapParameters *t = &slots_[slot].taps[tap];
        CONSTRAIN(t->velocity, 0.0f, 1.0f);
        CONSTRAIN(t->time, 0.0f, buffer_size);
        CONSTRAIN(t->velocity_type, VELOCITY_AMP, VELOCITY_BP);
        CONSTRAIN(t->panning, 0.0f, 1.0f);
      }
    }
  }

  void SaveData() {
    settings_storage_.ParsimoniousSave(data_, &settings_token_);
  }

  Data* mutable_data() { return &data_; }

  void SaveSlot(int slot_nr) {
    int bank = slot_nr / 6;
    if (bank == 0) bank0_.ParsimoniousSave(&slots_[6 * 0], 6 * sizeof(Slot), &token_[0]);
    if (bank == 1) bank1_.ParsimoniousSave(&slots_[6 * 1], 6 * sizeof(Slot), &token_[1]);
    if (bank == 2) bank2_.ParsimoniousSave(&slots_[6 * 2], 6 * sizeof(Slot), &token_[2]);
    if (bank == 3) bank3_.ParsimoniousSave(&slots_[6 * 3], 6 * sizeof(Slot), &token_[3]);
  }

  void ResetSlot(int slot) {
    slots_[slot].size = lut_preset_sizes[slot];

    for (int tap=0; tap<slots_[slot].size; tap++) {
      int index = tap + kMaxTaps * slot;
      TapParameters *t = &slots_[slot].taps[tap];
      t->time = lut_preset_times[index];
      t->velocity = lut_preset_velos[index];
      t->velocity_type = static_cast<VelocityType>(lut_preset_types[index]);
      t->panning = lut_preset_pans[index];
    }
  }

  void ResetBank(int bank) {
    for(int slot=bank*6; slot<(bank+1)*6; slot++)
      ResetSlot(slot);

    bank0_.ParsimoniousSave(&slots_[6 * bank], 6 * sizeof(Slot), &token_[0]);
  }

  uint8_t current_bank() { return data_.settings[1]; }

  void ResetCurrentBank() { ResetBank(current_bank()); }

  Slot* mutable_slot(int nr) { return &slots_[nr]; }

private:
  Data data_;
  uint16_t settings_token_;
  stmlib::Storage<7> settings_storage_;

  Slot slots_[kNumSlots];

  stmlib::Storage<8> bank0_;
  stmlib::Storage<9> bank1_;
  stmlib::Storage<10> bank2_;
  stmlib::Storage<11> bank3_;
  uint16_t token_[4];
};

#endif
