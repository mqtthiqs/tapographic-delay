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
// Parameters.

#ifndef PARAMETERS_H_
#define PARAMETERS_H_

#include "stmlib/stmlib.h"

const size_t kBlockSize = 128;
const uint8_t kMaxTaps = 36;

typedef struct { short l; short r; } ShortFrame;
typedef struct { float l; float r; } FloatFrame;

enum VelocityType {
  VELOCITY_AMP,
  VELOCITY_LP,
  VELOCITY_BP,
};

enum EditMode {
  EDIT_NORMAL,
  EDIT_OVERDUB,
  EDIT_OVERWRITE,
};

enum PanningMode {
  PANNING_LEFT,
  PANNING_RANDOM,
  PANNING_ALTERNATE,
};

struct Parameters {
  // ui -> delay
  float gain;
  float scale;
  float feedback;
  float modulation_amount;
  float modulation_frequency;
  float morph;
  float drywet;

  float velocity;
  bool tap;
  bool repeat;

  EditMode edit_mode;
  VelocityType velocity_type;

  // Settings
  float velocity_parameter;
  PanningMode panning_mode;
  bool quality;

  // delay -> ui
  // TODO refactor this!
  bool slot_modified;           // reset to false after read
};

struct TapParameters {
  float time;
  float velocity;
  VelocityType velocity_type;
  float panning;
};

struct Slot {
  uint8_t size;
  TapParameters taps[kMaxTaps];
};

#endif  // MTD_DSP_PARAMETERS_H_
