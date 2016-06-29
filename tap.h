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

#include "stmlib/dsp/dsp.h"
#include "stmlib/dsp/filter.h"

#include "parameters.h"
#include "audio_buffer.h"
#include "random_oscillator.h"
#include "fader.h"

#ifndef TAP_H_
#define TAP_H_

using namespace stmlib;

class Tap
{
 public:
  Tap() { }
  ~Tap() { }

  void Init() {
    lfo_.Init();
    previous_lfo_sample_ = 0.0f;
    time_ = kBlockSize;
    velocity_ = 0.0f;
    gain_l_ = gain_r_ = 1.0f;
  };

  /* minimum time is block size */
  inline void set_time(float time) { time_ = time + kBlockSize; }
  inline void set_velocity(float velocity) { velocity_ = velocity; }
  inline void set_gains(float gain_l, float gain_r) {
    gain_l_ = gain_l;
    gain_r_ = gain_r;
  }

  float time() { return time_; }
  bool active() { return fader_.volume() > 0.01f; }
  void fade_in(float length) { fader_.fade_in(length); }
  void fade_out(float length) { fader_.fade_out(length); }

  /* Dispatch function */
  void Process(VelocityType velocity_type,
               float prev_scale, float prev_jitter_amount,
               float scale, float jitter_amount, float jitter_frequency,
               AudioBuffer *buffer, FloatFrame* output) {
    if (velocity_type == VELOCITY_AMP)
      Process<VELOCITY_AMP>(prev_scale, prev_jitter_amount, scale,
                            jitter_amount, jitter_frequency, buffer, output);
    else if (velocity_type == VELOCITY_LP)
      Process<VELOCITY_LP>(prev_scale, prev_jitter_amount, scale,
                           jitter_amount, jitter_frequency, buffer, output);
    else if (velocity_type == VELOCITY_BP)
      Process<VELOCITY_BP>(prev_scale, prev_jitter_amount, scale,
                           jitter_amount, jitter_frequency, buffer, output);
  }

  template<VelocityType velocity_type>
    void Process(float prev_scale, float prev_jitter_amount,
                 float scale, float jitter_amount, float jitter_frequency,
                 AudioBuffer *buffer, FloatFrame* output) {

    /* TODO: enable this in the final version to save energy */
    // if (!active())
    //  return;

    /* set filter parameters */
    if (velocity_type == VELOCITY_LP) {
      filter_.set_f_q<FREQUENCY_FAST>(velocity_ * velocity_ * velocity_ / 8.0f, 0.8f);
    } else if (velocity_type == VELOCITY_BP) {
      filter_.set_f_q<FREQUENCY_FAST>(velocity_ * velocity_ / 20.0f, 2.5f);
    }

    /* compute random LFO */
    lfo_.set_slope(jitter_frequency / 20.0f);
    float lfo_sample = lfo_.Next();

    float time_start = time_ * prev_scale;
    float time_end = time_ * scale;

    float amplitude = 0.2f * SAMPLE_RATE;
    if (amplitude > time_end) amplitude = time_end;
    time_start += amplitude * previous_lfo_sample_ * prev_jitter_amount;
    time_end += amplitude * lfo_sample * jitter_amount;
    previous_lfo_sample_ = lfo_sample;

    /* assert (time_start > 0.0f && time_end > 0.0f); */

    float time = 0.0f;
    const float time_increment = (time_end - time_start - kBlockSize)
      / static_cast<float>(kBlockSize);

    fader_.Prepare();

    size_t size = kBlockSize;
    while(size--) {

      /* read sample from buffer */
      /* NOTE: doing the addition here avoids rounding errors with large times */
      float sample = buffer->ReadLinear(time_start + time);

      /* apply envelope */
      fader_.Process(sample);

      /* apply velocity */
      if (velocity_type == VELOCITY_AMP) {
        sample *= velocity_ * velocity_;
      } else if (velocity_type == VELOCITY_LP) {
        sample = filter_.Process<FILTER_MODE_LOW_PASS>(sample);
        sample *= (2.0f - velocity_) * velocity_;
      } else if (velocity_type == VELOCITY_BP) {
        sample = filter_.Process<FILTER_MODE_BAND_PASS>(sample);
      }

      /* write to buffer */
      output->l += sample * gain_l_;
      output->r += sample * gain_r_;

      /* increment stuff */
      output++;
      time += time_increment;
    }
  };

 private:

  NaiveSvf filter_;

  float time_;
  float velocity_;
  float gain_l_, gain_r_;

  Fader fader_;

  RandomOscillator lfo_;
  float previous_lfo_sample_;

  DISALLOW_COPY_AND_ASSIGN(Tap);
};

#endif
