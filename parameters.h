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

#ifndef MTD_DSP_PARAMETERS_H_
#define MTD_DSP_PARAMETERS_H_

#include "stmlib/stmlib.h"

namespace mtd {

const size_t kBlockSize = 32;

typedef struct { short l; short r; } ShortFrame;
typedef struct { float l; float r; } FloatFrame;

enum TimeDivision {
  TIME_DIVISION_1,
  TIME_DIVISION_2,
  TIME_DIVISION_3,
};

enum VelocityType {
  VELOCITY_AMPLITUDE,
  VELOCITY_LP,
};

struct DelayParameters {
  float time;
  float level;
  float feedback;
  float scale;
  float jitter_frequency;
  float jitter_amount;

  bool repeat;
  bool reverse;
  bool playing;
  TimeDivision time_division;
};

struct Parameters {
  DelayParameters delay[2];
  bool tap;
  VelocityType velocity_type;
};

}  // namespace mtd

#endif  // MTD_DSP_PARAMETERS_H_
