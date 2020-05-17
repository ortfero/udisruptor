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


#include <cstddef>
#include <vector>
#include <limits>
#include <thread>
#include "sequence.hpp"


namespace udisruptor {
  

  class barrier {
  public:
  
    using index_type = sequence::value_type;
    using size_type = sequence::value_type;
  
    barrier() noexcept = default;
    barrier(barrier const&) = default;
    barrier& operator = (barrier const&) = default;
    barrier(barrier&&) noexcept = default;
    barrier& operator = (barrier&&) noexcept = default;
    
    
    void depends_on(sequence const& n) {
      dependencies_.push_back(&n);
    }
    
    
    index_type wait(index_type n) {
      
      auto last_value = std::numeric_limits<index_type>::max();
      sequence const* last_sequence = nullptr;
      
      for(auto dependency: dependencies_) {
        auto const m = dependency->value();
        if(m >= last_value)
          continue;
        last_value = m;
        last_sequence = dependency;
      }
      
      if(last_sequence == nullptr)
        return n;
      
      if(n <= last_value)
        return last_value;
      
      do {
        std::this_thread::yield();
        last_value = last_sequence->value();
      } while(n > last_value);

      return last_value;
    }  
  
  private:
  
    std::vector<sequence const*> dependencies_;
    
  }; // barrier
  
  
} // udisruptor
