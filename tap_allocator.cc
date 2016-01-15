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
// Tap allocator

#include "tap_allocator.h"


namespace mtd 
{
  void TapAllocator::Init(Tap* taps) {
    taps_ = taps;
    for (size_t i=0; i<kMaxTaps; i++) {
      taps_[i].set_time(i * i * SAMPLE_RATE * 1.0f / kMaxTaps);
      taps_[i].set_velocity(static_cast<float>(i+1) / kMaxTaps);
    }
  }

  void TapAllocator::AddTap(float time, float velocity) {
  }

  void TapAllocator::RemoveTap() {
  }

}
