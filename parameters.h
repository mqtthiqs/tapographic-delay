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

#ifndef MTD_PARAMETERS_H_
#define MTD_PARAMETERS_H_

#include "stmlib/stmlib.h"

namespace mtd {

const size_t kBlockSize = 32;
const uint8_t kMaxTaps = 9;

typedef struct { short l; short r; } ShortFrame;
typedef struct { float l; float r; } FloatFrame;

enum VelocityType {
  VELOCITY_AMP,
  VELOCITY_LP,
  VELOCITY_BP,
};

enum EditMode {
  EDIT_MODE_NORMAL,
  EDIT_MODE_OVERDUB,
  EDIT_MODE_OVERWRITE,
};

enum QuantizerMode {
  QUANTIZER_MODE_NONE,
  QUANTIZER_MODE_8,
  QUANTIZER_MODE_16,
};

struct Parameters {
  float time;
  float level;
  float feedback;
  float drywet;
  float morph;
  float jitter_amount;
  float jitter_frequency;
  float scale;

  bool repeat;

  bool ping;
  EditMode edit_mode;
  QuantizerMode quantize;
  VelocityType velocity_type;
};

}  // namespace mtd

#endif  // MTD_DSP_PARAMETERS_H_
