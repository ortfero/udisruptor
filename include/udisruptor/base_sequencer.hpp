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


#include <vector>
#include <thread>
#include <memory>
#include "sequence.hpp"



namespace udisruptor {
  
  
  class base_sequencer {
  public:
  
    using index_type = sequence::value_type;
    using size_type = sequence::value_type;
    
    base_sequencer() noexcept = default;
    base_sequencer(base_sequencer const&) = delete;
    base_sequencer& operator = (base_sequencer const&) = delete;
    base_sequencer(base_sequencer&& other) noexcept = default;
    base_sequencer& operator = (base_sequencer&& other) noexcept = default;


    sequence* add_consumer() {
      consumers_.emplace_back(sequence{});
      return &consumers_.back();
    }


    explicit operator bool () noexcept {
      return capacity_ != 0 && !consumers_.empty();
    }


    void reserve(size_type capacity) {
      capacity_ = nearest_power_of_2(capacity);
    }

    
  protected:
  
    size_type capacity() const noexcept {
      return capacity_;
    }
      

    index_type wait(index_type n) {

      if(n - cached_last_.value() < capacity_)
        return n;

      index_type last_value = consumers_.front().value();
      sequence const* last_sequence = &consumers_.front();

      if(consumers_.size() > 1) {
        for(auto it = consumers_.begin() + 1; it != consumers_.end(); ++it) {
          auto const m = it->value();
          if(m >= last_value)
            continue;
          last_value = m;
          last_sequence = &*it;
        }
      }
      
      while(n - last_value >= capacity_) {
        std::this_thread::yield();
        last_value = last_sequence->value();
      }

      cached_last_ = last_value;

      return n;
    }      


  private:
  
    size_type capacity_{0};
    std::vector<sequence> consumers_;
    sequence cached_last_;
    

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
  }; // base_sequencer
  
  
} // udisruptor
