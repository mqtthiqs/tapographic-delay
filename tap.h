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
  class Tap
  {
  public:
    Tap() { }
    ~Tap() { }

    void Init(RingBuffer* buffer) {
      buffer_ = buffer;
      lfo_.Init();
      previous_lfo_sample_ = 0.0f;
      volume_ = 0.0f;
      time_ = kBlockSize;
      velocity_ = 0.0f;
    };
    /* minimum time is block size */
    void set_time(float time) { time_ = time + kBlockSize; }
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

    /* Dispatch function */
    void Process(Parameters* prev_params, Parameters* params, float* output) {
      if (params->velocity_type == VELOCITY_AMP)
        Process<VELOCITY_AMP>(prev_params, params, output);
      else if (params->velocity_type == VELOCITY_LP)
        Process<VELOCITY_LP>(prev_params, params, output);
      else if (params->velocity_type == VELOCITY_BP)
        Process<VELOCITY_BP>(prev_params, params, output);
    }

    template<VelocityType velocity_type>
    void Process(Parameters* prev_params, Parameters* params, float* output) {

      /* compute volume increment */
      float volume_end;
      if (queued_ && *busy_voices_ > kMaxTaps) {
        volume_end = volume_;
      } else {
        queued_ = false;
        volume_end = volume_ + volume_increment_;

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

      float volume_increment = (volume_end - volume_) / kBlockSize;

      /* set filter parameters */
      if (velocity_type == VELOCITY_LP) {
        filter_.set_f_q<FREQUENCY_FAST>(velocity_ * velocity_ / 8.0f, 0.6f);
      } else if (velocity_type == VELOCITY_BP) {
        filter_.set_f_q<FREQUENCY_FAST>(velocity_ * velocity_ / 24.0f, 3.0f);
      }

      float time_start = time_ * prev_params->scale;
      float time_end = time_ * params->scale;

      /* add random LFO */
      lfo_.set_slope(params->jitter_frequency / 100.0f);
      float lfo_sample = lfo_.Next();

      float amplitude = 0.2f * SAMPLE_RATE;
      if (amplitude > time_end) amplitude = time_end;
      time_start += amplitude * previous_lfo_sample_ * prev_params->jitter_amount;
      time_end += amplitude * lfo_sample * params->jitter_amount;
      previous_lfo_sample_ = lfo_sample;

      /* assert (time_start > 0.0f && time_end > 0.0f); */

      float time = 0.0f;
      float time_increment = (time_end - time_start - kBlockSize) / static_cast<float>(kBlockSize);

      size_t size = kBlockSize;
      while(size--) {

        /* doing the addition here avoids rounding errors with large times */
        float sample = buffer_->ReadLinear(time_start + time);

        if (velocity_type == VELOCITY_AMP) {
          sample *= velocity_ * velocity_;
        } else if (velocity_type == VELOCITY_LP) {
          sample = filter_.Process<FILTER_MODE_LOW_PASS>(sample);
          sample *= (2.0f - velocity_) * velocity_;
        } else if (velocity_type == VELOCITY_BP) {
          sample = filter_.Process<FILTER_MODE_BAND_PASS>(sample);
        }

        sample *= volume_ * volume_;
        *output += sample;

        time += time_increment;
        volume_ += volume_increment;

        output++;
      }
    };

  private:

    RingBuffer* buffer_;
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
