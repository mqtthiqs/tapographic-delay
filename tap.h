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
#include "random_oscillator.h"

#include "stmlib/dsp/dsp.h"
#include "stmlib/dsp/filter.h"

using namespace stmlib;

namespace mtd 
{
  const uint8_t kMaxTaps = 16;
  const uint16_t kMaxBufferSize = 128;

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
      lfo_.Init();
      previous_lfo_sample_ = 0.0f;
    };

    void Process(DelayParameters* prev_params, DelayParameters* params, float* output, size_t size) {

      float velocity = tap_params_[tap_nr_].velocity;
      filter_.set_f<FREQUENCY_FAST>(velocity * velocity / 32.0f);

      float time_start = tap_params_[tap_nr_].time;
      float time_end = tap_params_[tap_nr_].time;

      /* add random LFO */
      lfo_.set_frequency(params->jitter_frequency * 7.0f * 32.0f
                         * (1.0f + 0.01f * tap_nr_)); /* small random shift to avoid unison */
      float lfo_sample = lfo_.Next();
      time_start += 0.1f * SAMPLE_RATE * previous_lfo_sample_ * prev_params->jitter_amount;
      time_end += 0.1f * SAMPLE_RATE * lfo_sample * params->jitter_amount;
      previous_lfo_sample_ = lfo_sample;

      if (time_start < 0.0f) time_start = 0.0f;
      if (time_end < 0.0f) time_end = 0.0f;

      float read_size = size + time_start - time_end;

      CONSTRAIN(read_size, -kMaxBufferSize, kMaxBufferSize);

      MAKE_INTEGRAL_FRACTIONAL(time_start);
      float time = -time_start_fractional; /* why "-"? */
      float time_increment = read_size / static_cast<float>(size);

      if (read_size < 0) {
        read_size = -read_size;
        time = read_size;
      }

      /* +1 for interpolation, +1 for rounding to the next */
      uint32_t buf_size = static_cast<uint32_t>(read_size) + 2;
      int16_t buf[kMaxBufferSize];

      buffer_->Read(buf, time_start_integral, buf_size);

      while(size--) {
        MAKE_INTEGRAL_FRACTIONAL(time);
        int16_t a = buf[time_integral];
        int16_t b = buf[time_integral + 1];
        float sample = static_cast<float>(a + (b - a) * time_fractional) / 32768.0f;

        sample = filter_.Process<FILTER_MODE_LOW_PASS>(sample);
        *output += sample * (2.0f - velocity) * velocity;

        time += time_increment;
        output++;
      }
    };

  private:

    uint8_t tap_nr_;
    RingBuffer<short>* buffer_;
    TapParameters* tap_params_;
    OnePole filter_;

    RandomOscillator lfo_;
    float previous_lfo_sample_;

    DISALLOW_COPY_AND_ASSIGN(Tap);
  };
}

