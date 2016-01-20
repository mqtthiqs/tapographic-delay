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
// Multitap delay

#include "multitap_delay.h"
#include "stmlib/dsp/dsp.h"
#include "resources.h"

using namespace stmlib;

namespace mtd 
{
  void MultitapDelay::Init(short* buffer, int32_t buffer_size, Clock* clock) {
    clock_ = clock;
    counter_ = 0;
    counter_running_ = false;

    buffer_.Init(buffer, buffer_size);
    dc_blocker_.Init();
    dc_blocker_.set_f_q<FREQUENCY_FAST>(30.0f / SAMPLE_RATE, 1.0f);

    for (size_t i=0; i<kMaxTaps; i++) {
      taps_[i].Init(&buffer_);
    }

    tap_allocator_.Init(taps_);
  };

  void MultitapDelay::AddTap(float velocity, EditMode edit_mode, QuantizerMode quantizer_mode) {

    // first tap does not count, it just starts the counter
    if (!counter_running_) {
      counter_running_ = true;
      return;
    }

    float time = static_cast<float>(counter_);

    if (repeat_time_) {
      float repeat = static_cast<float>(repeat_time_);
      float quantize =
        quantizer_mode == QUANTIZER_MODE_8 ? 4.0f :
        quantizer_mode == QUANTIZER_MODE_16 ? 8.0f :
        repeat;
      time = round(time / repeat * quantize)
        * repeat / quantize;
    }


    if (edit_mode == EDIT_MODE_NORMAL && time < buffer_.size()) {
      tap_allocator_.Add(time, velocity);
    } else if (edit_mode == EDIT_MODE_OVERDUB) {
      tap_allocator_.Add(time, velocity);
    } else if (edit_mode == EDIT_MODE_OVERWRITE) {
      tap_allocator_.Add(time, velocity);
      tap_allocator_.Remove();
    }
  }

  void MultitapDelay::Clear() {
    counter_running_ = false;
    tap_allocator_.Clear();
    counter_ = 0;
  }

  void MultitapDelay::Process(Parameters *params, ShortFrame* input, ShortFrame* output) {

    // repeat time, in samples
    repeat_time_ = clock_->running() ?
      clock_->period() * kBlockSize :
      tap_allocator_.max_time() * params->scale;

    if (repeat_time_ > buffer_.size() ||
        repeat_time_ < 100) {
      repeat_time_ = 0;
    }

    // increment sample counter
    if (counter_running_) {
      counter_ += kBlockSize;
      // in the right edit modes, reset counter
      if (params->edit_mode != EDIT_MODE_NORMAL &&
          counter_ > repeat_time_) {
        counter_ = 0;
      }
    }

    // set fade time
    tap_allocator_.set_fade_time(params->morph * 5000);

    { /* Write into the buffer */
      int16_t buf[kBlockSize];

      if (params->repeat && repeat_time_) {
        buffer_.Read(buf, repeat_time_, kBlockSize);
      } else {
        std::fill(buf, buf+kBlockSize, 0);
      }

      for (size_t i=0; i<kBlockSize; i++) {
        int32_t sample = static_cast<int32_t>(input[i].l)
          + (params->feedback / 2.0f) * feedback_buffer[i]; // TODO: / busy_voices
        buf[i] += Clip16(sample);
      }
      buffer_.Write(buf, kBlockSize);
    }

    float buf_l[kBlockSize];
    float buf_r[kBlockSize];
    std::fill(buf_l, buf_r+kBlockSize, 0.0f);
    std::fill(buf_r, buf_r+kBlockSize, 0.0f);

    for (int i=0; i<kMaxTaps; i++) {
      if (i & 1) {
        taps_[i].Process(&prev_params_, params, buf_l);
      } else {
        taps_[i].Process(&prev_params_, params, buf_r);
      }
    }

    /* convert, output and feed back */
    for (size_t i=0; i<kBlockSize; i++) {
      float sample_l = buf_l[i];
      float sample_r = buf_r[i];

      float fb = dc_blocker_.Process<FILTER_MODE_HIGH_PASS>(sample_l + sample_r);

      // add dry signal
      float dry = static_cast<float>(input[i].l) / 32768.0f;
      sample_l += (dry - sample_l) * params->drywet;
      sample_r += (dry - sample_r) * params->drywet;

      feedback_buffer[i] = Clip16(static_cast<int32_t>(32768.0f * fb));

      output[i].l = Clip16(static_cast<int32_t>(32768.0f * sample_l));
      output[i].r = Clip16(static_cast<int32_t>(32768.0f * sample_r));
    }

    prev_params_ = *params;
  };
}
