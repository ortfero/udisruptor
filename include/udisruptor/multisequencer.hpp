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
#include "base_sequencer.hpp"


namespace udisruptor {
  
  
  class multisequencer : public base_sequencer {
  public:
  
    using base = base_sequencer;
    
    multisequencer() noexcept = default;
    multisequencer(multisequencer const&) = delete;
    multisequencer& operator = (multisequencer const&) = delete;
    
    multisequencer(multisequencer&& other) noexcept:
      base(std::move(other)),
      index_mask_{other.index_mask_},      
      producer_{other.producer_.load()},
      published_{std::move(other.published_)}
    { }


    multisequencer& operator = (multisequencer&& other) noexcept {
      base::operator = (std::move(other));
      index_mask_ = other.index_mask_;
      producer_.store(other.producer_.load());
      published_ = std::move(other.published_);
      return *this;
    }
    
    
    explicit multisequencer(size_type capacity) {
      reserve(capacity);
    }


    void reserve(size_type capacity) {
      base::reserve(capacity);
      capacity = base::capacity();
      published_ = std::make_unique<index_type[]>(capacity);
      for(size_type n = 0; n != capacity; ++n)
        published_[n] = 0;
      index_mask_ = index_type(capacity - 1);
    }
    
    
    index_type claim() noexcept {
      if(!published_)
        return sequence::invalid;
      index_type const p = producer_.fetch_add(1);      
      base::wait(p);
      return p;
    }
    
    
    void publish(index_type n) noexcept {
      published_[n & index_mask_] = n + 1;
    }


    index_type try_fetch(index_type consumer) noexcept {
      if(!published_)
        return sequence::invalid;
      if(published_[consumer & index_mask_] != consumer + 1)
        return sequence::invalid;
      return consumer;
    }


    index_type try_fetch_all(index_type consumer) noexcept {
      if(!published_)
        return consumer;
      while(published_[consumer & index_mask_] == consumer + 1)
        ++consumer;
      return consumer;
    }

    
  private:
  
    size_type capacity_{0};
    index_type index_mask_{0};
    alignas(sequence::cacheline) std::atomic<index_type> producer_{0};
    std::unique_ptr<index_type[]> published_;
    
  }; // multisequencer
  
  
} // udisruptor
