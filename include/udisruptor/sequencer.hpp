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


#include "base_sequencer.hpp"


namespace udisruptor {
  
  
  class sequencer : public base_sequencer {
  public:
  
    using base = base_sequencer;
    
    sequencer() noexcept = default;
    sequencer(sequencer const&) = delete;
    sequencer& operator = (sequencer const&) = delete;
    sequencer(sequencer&& other) noexcept = default;
    sequencer& operator = (sequencer&& other) noexcept = default;
        
    explicit sequencer(size_type capacity) {
      base::reserve(capacity);
    }
    
    index_type claim() noexcept {      
      index_type const p = producer_++;
      base::wait(p);
      return p;
    }
    
    
    void publish(index_type n) noexcept {
      publisher_ = n;
    }


    index_type try_fetch(index_type consumer) noexcept {
      if(publisher_ < consumer)
        return sequence::invalid;
      return consumer;
    }


    index_type try_fetch_all(index_type consumer) noexcept {
      if(publisher_ < consumer)
        return consumer;
      return publisher_ + 1;
    }


    
  private:
  
    index_type producer_{0};
    index_type publisher_{-1};
    
  }; // sequencer
  
  
} // udisruptor
