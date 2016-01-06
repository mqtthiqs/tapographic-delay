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
// Simple ring buffer with multiple taps

#ifndef MTD_RING_BUFFER_H_
#define MTD_RING_BUFFER_H_

#include "stmlib/stmlib.h"
#include <stm32f4xx.h>

#include <algorithm>

namespace mtd 
{
  template<typename T>
  class RingBuffer
  {
  public:
    RingBuffer() { }
    ~RingBuffer() { }

    void Init(T* buffer, uint32_t buffer_size) {
      buffer_ = buffer;
      buffer_size_ = buffer_size;
    }

    void Clear() {
      std::fill(buffer_, buffer_ + buffer_size_, 0);
    }

    uint32_t size() { return buffer_size_; }

    /* Write one value at cursor and increment it */
    void Write(T value) {
      buffer_[cursor_] = value;
      if (cursor_ == buffer_size_) {
        cursor_ = 0;
      } else {
        cursor_++;
      }
    }

    /* Write [size] values at cursor and increment it. */
    void Write(T *src, size_t size) {
      size_t written = size;
      if (cursor_ > buffer_size_ - size) {
        written = buffer_size_ - cursor_;
      }
      std::copy(src, src + written, buffer_ + cursor_);
      if (written < size) {
        cursor_ = size - written;
        std::copy(src + written, src + size, buffer_);
      } else {
        cursor_ += size;
      }
    }

    /* Reads the value from [pos] writes ago */
    T Read(uint32_t pos) {
      uint32_t index;// = cursor_ - pos;
      if (cursor_ < pos) {
        index = buffer_size_ - pos + cursor_;
      } else {
        index = cursor_ - pos;
      }
      return buffer_[index];
    }

    /* Reads the [size] values until [pos] writes ago.
     * assert (pos + size < buffer_size_) */
    void Read(T* dest, uint32_t pos, size_t size) {
      uint32_t index;
      size_t read = size;
      if (cursor_ < pos + size) {
        index = buffer_size_ - pos - size + cursor_;
        if (index > buffer_size_ - size) {
          read = buffer_size_ - index;
          std::copy(buffer_, buffer_ + size - read, dest + read);
        }
      } else {
        index = cursor_ - pos - size;
      }
      std::copy(buffer_ + index, buffer_ + index + read, dest);
    }

  private:

    T* buffer_;
    uint32_t cursor_;
    uint32_t buffer_size_;

    DISALLOW_COPY_AND_ASSIGN(RingBuffer);
  };
}

#endif
