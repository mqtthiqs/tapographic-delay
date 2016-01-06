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
//  A single tap

#include "parameters.h"
#include "ring_buffer.h"

#include "stmlib/dsp/dsp.h"

namespace mtd 
{
  const uint8_t kMaxTaps = 1;
  const size_t kDelayMargin = 10;

  struct TapParameters {
    float time;               /* in samples */
    float velocity;           /* 0..1 */
  };

  class Tap
  {
  public:
    Tap() { }
    ~Tap() { }

    void Init(RingBuffer<short>* buffer, TapParameters* tap_params, uint8_t tap_nr) {
      tap_nr_ = tap_nr;
      buffer_ = buffer;
      tap_params_ = tap_params;
    };

    void Process(DelayParameters* prev_params, DelayParameters* params, float* output, size_t size) {
      int16_t buf[size+kDelayMargin];
      float time = tap_params_[tap_nr_].time * prev_params->scale;
      float time_end = tap_params_[tap_nr_].time * params->scale;
      float velocity = tap_params_[tap_nr_].velocity;

      int32_t orig_time_integral = static_cast<int32_t>(time);
      buffer_->Read(buf, orig_time_integral, size+kDelayMargin);

      float step = 1.0f / static_cast<float>(size);
      float time_increment = (time_end - time) * step;

      int16_t *s = buf;
      while(size--) {
        MAKE_INTEGRAL_FRACTIONAL(time);
        int16_t jumps = time_integral - orig_time_integral;
        int16_t a = *(s+kDelayMargin/2+jumps);
        int16_t b = *(s+kDelayMargin/2+1+jumps);
        float sample = static_cast<float>(a + (b - a) * time_fractional) / 32768.0f;
        *output += sample * velocity;
        time += time_increment;
        output++;
        s++;
      }
    };

  private:
    uint8_t tap_nr_;
    RingBuffer<short>* buffer_;
    TapParameters* tap_params_;

    /* DISALLOW_COPY_AND_ASSIGN(Tap); */
  };
}

