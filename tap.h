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

namespace mtd 
{
  const uint8_t kMaxTaps = 16;

  struct TapParameter {
    float time;               /* in samples */
    float velocity;           /* 0..1 */
  };

  struct TapState {
    RingBuffer<short> buffer;
    VelocityType velocity_type;
    TapParameter tap_params[kMaxTaps];
  };

  class Tap
  {
  public:
    Tap() { }
    ~Tap() { }

    void Init(uint8_t tap_nr) {
      tap_nr_ = tap_nr;
    };

    void Process(TapState* state, float* output, size_t size) {
      int16_t buf[size];
      float time = state->tap_params[tap_nr_].time;

      state->buffer.Read(buf, static_cast<uint32_t>(time), size);
    };

  private:
    uint8_t tap_nr_;

    DISALLOW_COPY_AND_ASSIGN(Tap);
  };
}

