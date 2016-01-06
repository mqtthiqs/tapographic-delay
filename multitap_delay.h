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

#include "parameters.h"
#include "ring_buffer.h"
#include "tap.h"

#include "stmlib/dsp/dsp.h"

using namespace stmlib;

namespace mtd 
{
  class MultitapDelay
  {
  public:
    MultitapDelay() { }
    ~MultitapDelay() { }

    void Init(short* buffer, int32_t buffer_size) {
      state_.buffer.Init(buffer, buffer_size);
      for (size_t i=0; i<kMaxTaps; i++)
        tap[i].Init(i);
    };

    void Process(DelayParameters *params, ShortFrame* input, ShortFrame* output, size_t size) {

      { /* Write into the buffer */
        int16_t buf[size];

        for (size_t i=0; i<size; i++) {
          buf[i] = input[i].l
            + params->feedback * feedback_buffer[i];
        }
        state_.buffer.Write(buf, size);
      }

      float buf[size];

      /* Accumulate buffers of all taps */
      for (int i=0; i<kMaxTaps; i++) {
        tap[i].Process(&state_, buf, size);
      }

      /* convert, output and feed back */
      for (size_t i=0; i<size; i++) {
        int16_t sample = Clip16(static_cast<int32_t>(buf[i] * 32768.0f));
        feedback_buffer[i] = sample;
        output[i].l = sample;
      }

    };

    void SimpleDelay(DelayParameters *params, ShortFrame* input, ShortFrame* output, size_t size) {
      int16_t buf[size];

      for (size_t i=0; i<size; i++)
        buf[i] = input[i].l + input[i].r;

      uint32_t time = state_.buffer.size()-size; /* = max delay */
      state_.buffer.Write(buf, size);
      state_.buffer.Read(buf, time, size);

      for (size_t i=0; i<size; i++)
        output[i].l = output[i].r = buf[i];
    };

  private:

    Tap tap[kMaxTaps];
    TapState state_;

    int16_t feedback_buffer[kBlockSize];   /* max block size */

    DISALLOW_COPY_AND_ASSIGN(MultitapDelay);
  };
}
