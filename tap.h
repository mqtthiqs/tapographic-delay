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


#ifndef MTD_TAP_H_
#define MTD_TAP_H_

using namespace stmlib;

namespace mtd 
{
  const uint16_t kMaxBufferSize = 512;

  class Tap
  {
  public:
    Tap() { }
    ~Tap() { }

    void Init(RingBuffer<short>* buffer) {
      buffer_ = buffer;
      lfo_.Init();
      previous_lfo_sample_ = 0.0f;
      volume_ = 0.0f;
      time_ = 0.0f;
      velocity_ = 0.0f;
    };

    void set_time(float time) { time_ = time; }
    void set_velocity(float velocity) { velocity_ = velocity; }

    void set_busy_voices_counter(uint8_t *busy_voices) {
      busy_voices_ = busy_voices;
    }

    void fade_in(float length) {
      queued_ = true;
      volume_increment_ = 1.0f / length;
      if (volume_ == 0.0f)
        (*busy_voices_)++;
    }

    void fade_out(float length) {
      volume_increment_ = -1.0f / length;
    }

    void Process(Parameters* prev_params, Parameters* params, float* output) {

      /* compute volume increment */
      float volume = volume_;
      float volume_end;

      if (queued_ && *busy_voices_ == kMaxTaps) {
        volume_end = volume;
      } else {
        queued_ = false;
        volume_end = volume + volume_increment_;

        if (volume_end < 0.0f) {
          /* end of fade out */
          (*busy_voices_)--;
          volume_end = 0.0f;
          volume_increment_ = 0.0f;
        } else if (volume_end > 1.0f) {
          /* end of fade in */
          volume_end = 1.0f;
          volume_increment_ = 0.0f;
        }
      }

      float volume_increment = (volume_end - volume) / kBlockSize;

      /* set filter parameters */
      if (params->velocity_type == VELOCITY_LP) {
        filter_.set_f_q<FREQUENCY_FAST>(velocity_ * velocity_ / 8.0f, 0.6f);
      } else if (params->velocity_type == VELOCITY_BP) {
        filter_.set_f_q<FREQUENCY_FAST>(velocity_ * velocity_ / 24.0f, 3.0f);
      }

      float time_start = time_ * prev_params->scale;
      float time_end = time_ * params->scale;

      /* add random LFO */
      lfo_.set_frequency(params->jitter_frequency * 7.0f * 32.0f
                         * (1.0f + 0.1f * velocity_)); /* small random shift to avoid unison */
      float lfo_sample = lfo_.Next();
      time_start += 0.1f * SAMPLE_RATE * previous_lfo_sample_ * prev_params->jitter_amount;
      time_end += 0.1f * SAMPLE_RATE * lfo_sample * params->jitter_amount;
      previous_lfo_sample_ = lfo_sample;

      if (time_start < 0.0f) time_start = 0.0f;
      if (time_end < 0.0f) time_end = 0.0f;

      /* compute buffer read boundaries */
      float read_size = kBlockSize + time_start - time_end;

      CONSTRAIN(read_size, -kMaxBufferSize, kMaxBufferSize);

      MAKE_INTEGRAL_FRACTIONAL(time_start);
      float time = -time_start_fractional; /* why "-"? */
      float time_increment = read_size / static_cast<float>(kBlockSize);

      if (read_size < 0) {
        read_size = -read_size;
        time = read_size;
      }

      /* +1 for interpolation, +1 for rounding to the next */
      uint32_t buf_size = static_cast<uint32_t>(read_size) + 2;
      int16_t buf[kMaxBufferSize];

      buffer_->Read(buf, time_start_integral, buf_size);

      size_t size = kBlockSize;
      while(size--) {
        MAKE_INTEGRAL_FRACTIONAL(time);
        int16_t a = buf[time_integral];
        int16_t b = buf[time_integral + 1];
        float sample = static_cast<float>(a + (b - a) * time_fractional) / 32768.0f;

        if (params->velocity_type == VELOCITY_AMP) {
          sample *= velocity_ * velocity_;
        } else if (params->velocity_type == VELOCITY_LP) {
          sample = filter_.Process<FILTER_MODE_LOW_PASS>(sample);
          sample *= (2.0f - velocity_) * velocity_;
        } else if (params->velocity_type == VELOCITY_BP) {
          sample = filter_.Process<FILTER_MODE_BAND_PASS>(sample);
        }

        sample *= volume * volume;
        *output += sample;

        time += time_increment;
        volume += volume_increment;

        output++;
      }

      volume_ = volume;
    };

  private:

    RingBuffer<short>* buffer_;
    Svf filter_;

    float time_;
    float velocity_;

    float volume_, volume_increment_;
    bool queued_;

    uint8_t* busy_voices_;

    RandomOscillator lfo_;
    float previous_lfo_sample_;

    DISALLOW_COPY_AND_ASSIGN(Tap);
  };
}

#endif
