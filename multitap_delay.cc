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

using namespace stmlib;

namespace mtd 
{
  void MultitapDelay::Init(short* buffer, int32_t buffer_size) {
    buffer_.Init(buffer, buffer_size);
    dc_blocker_.Init();
    dc_blocker_.set_f_q<FREQUENCY_FAST>(20.0f / SAMPLE_RATE, 1.0f);

    for (size_t i=0; i<kMaxTaps; i++) {
      tap[i].Init(&buffer_, &tap_params_[0], i);
      tap_params_[i].time = i * i * SAMPLE_RATE * 0.01f + 5000;
      tap_params_[i].velocity = (float)(i+1) / kMaxTaps;
    }
  };

  void MultitapDelay::Process(DelayParameters *params, ShortFrame* input, ShortFrame* output, size_t size) {

    { /* Write into the buffer */
      int16_t buf[size];

      for (size_t i=0; i<size; i++) {
        int32_t sample = input[i].l
          + params->feedback * feedback_buffer[i];
        buf[i] = Clip16(sample);
      }
      buffer_.Write(buf, size);
    }

    float buf[size];
    std::fill(buf, buf+size, 0.0f);

    /* Read & accumulate buffers of all taps */
    for (int i=0; i<kMaxTaps; i++) {
      tap[i].Process(&prev_params_, params, buf, size);
    }

    /* convert, output and feed back */
    for (size_t i=0; i<size; i++) {
      float s = dc_blocker_.Process<FILTER_MODE_HIGH_PASS>(buf[i]);
      int16_t sample = Clip16(static_cast<int32_t>(s * 32768.0f));
      feedback_buffer[i] = sample;
      output[i].l = sample;
    }

    prev_params_ = *params;
  };
}
