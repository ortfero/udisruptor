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


#include <cstdint>
#include <atomic>


namespace udisruptor {


  class sequence {
  public:
  
    static constexpr std::size_t cacheline = 64;

    using value_type = int64_t;
    
    static constexpr value_type invalid = -1;

    constexpr sequence() noexcept: value_{invalid} { }
    explicit constexpr sequence(value_type n) noexcept: value_{n} { }
    value_type value() const noexcept { return value_; }
    value_type next() const noexcept { return value_ + 1; }    
    
    sequence& operator = (value_type n) noexcept {
      value_ = n;
      return *this;
    }


    sequence(sequence const& other) noexcept:
      value_{other.value_}
    { }
    
    sequence& operator = (sequence const& other) noexcept {
      value_ = other.value_;
      return *this;
    }


  private:

    alignas(cacheline) value_type value_;

  }; // sequence


} // udisruptor
