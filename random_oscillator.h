// Copyright 2015 Matthias Puech.
//
// Author: Matthias Puech (matthias.puech@gmail.com)
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
// Smoothed random oscillator

#include "resources.h"
#include "stmlib/utils/random.h"
#include "stmlib/dsp/dsp.h"

using namespace stmlib;

namespace mtd 
{
  class RandomOscillator
  {
  public:

    inline float getFloat() {
      return Random::GetFloat() * 2.0f - 1.0f;
    }

    void Init() {
      value_ = 0.0f;
      next_value_ = getFloat();
    }

    inline void set_frequency(float freq) {
      phase_increment_ = freq / SAMPLE_RATE;
    }

    float Next() {
      phase_ += phase_increment_;
      if (phase_ > 1.0f) {
        phase_--;
        value_ = next_value_;
        next_value_ = getFloat();
      }

      float sin = Interpolate(lut_raised_cos, phase_, LUT_RAISED_COS_SIZE-1);
      return value_ + (next_value_ - value_) * sin;
    }

  private:
    float phase_;
    float phase_increment_;
    float value_;
    float next_value_;
  };
}
