/* This file is part of udisruptor library
 * Copyright 2020 Andrei Ilin <ortfero@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once


#include <memory>
#include "sequence.hpp"


namespace udisruptor {


  template<typename T>
  class ring_buffer {
  public:

    using size_type = sequence::value_type;
    using index_type = sequence::value_type;
    using value_type = T;


    ring_buffer() noexcept = default;
    ring_buffer(ring_buffer const&) = delete;
    ring_buffer& operator = (ring_buffer const&) = delete;
    explicit operator bool () noexcept { return !!pool_; }
    size_type capacity() const noexcept { return capacity_; }
    ring_buffer(ring_buffer&&) noexcept = default;
    ring_buffer& operator = (ring_buffer&&) noexcept = default;    
    
    
    explicit ring_buffer(size_type capacity) {
      reserve(capacity);
    }


    void reserve(size_type capacity) {
      capacity = nearest_power_of_2(capacity);
      capacity_ = capacity;
      index_mask_ = index_type(capacity - 1);
      pool_ = std::make_unique<T[]>(capacity);
    }


    T& operator [] (index_type n) noexcept {
      return pool_[n & index_mask_];
    }


    T const& operator [] (index_type n) const noexcept {
      return pool_[n & index_mask_];
    }

  private:

    size_type capacity_{0};
    index_type index_mask_{0};
    std::unique_ptr<T[]> pool_;
 

    static uint64_t nearest_power_of_2(uint64_t n) {
      if(n < 2)
        return 2;
      n--;
      n |= n >> 1;
      n |= n >> 2;
      n |= n >> 4;
      n |= n >> 8;
      n |= n >> 16;
      n |= n >> 32;
      n++;
      return n;
    }

  }; // ring_buffer


} // udisruptor
