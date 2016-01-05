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

#include "delay.h"

namespace multitap 
{

  void Delay::Init(ShortFrame* buffer, size_t buffer_size) {
    cursor_ = 0;
    buffer_ = buffer;
    buffer_size_ = buffer_size;
  }

  void Delay::Process(ShortFrame* input, ShortFrame* output, size_t size) {
    while (size--) {

      buffer_[cursor_].l = input->l;
      buffer_[cursor_].r = input->r;

      int16_t time = buffer_size_-1;
      volatile int16_t index = static_cast<int16_t>(cursor_) - time;
      if (index < 0) index += buffer_size_;

      output->l = buffer_[index].l;
      output->r = buffer_[index].r;

      input++;
      output++;
      cursor_ = (cursor_ + 1) % buffer_size_;
    }
  }

}
