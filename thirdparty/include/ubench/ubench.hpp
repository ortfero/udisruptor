/* This file is part of ubench library
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


#include <chrono>
#include <set>
#include <iosfwd>
#include <iomanip>


namespace ubench {



struct result {

  enum code {
    ok, optimized
  };
  
  using timing = std::chrono::duration<double, std::nano>;

  code code{ok};
  timing time{0.};
  std::chrono::microseconds took{0};

  result() noexcept = default;
  result(result const&) noexcept = default;
  result& operator = (result const&) noexcept = default;
  explicit operator bool () const noexcept { return code == ok; }


  result(enum code c, std::chrono::microseconds took) noexcept:
    code{c}, took{took} { }


  result(timing time, std::chrono::microseconds took) noexcept:
    code{ok}, time{time}, took{took}
  { }


  char const* message() const noexcept {
    switch(code) {
      case ok:
        return "ok";
      case optimized:
        return "value was probably optimized";
      default:
        return "unknown";
    }
  }
  
  template<typename charT, typename traits> friend
  std::basic_ostream<charT, traits>&
    operator << (std::basic_ostream<charT, traits>& os, result const& r) noexcept {
      if(!r)
        return os << "unable to benchmark because " << r.message();
      return os << std::setprecision(1) << std::fixed << r.time.count() << " ns";
    }
}; // result



#ifdef _MSC_VER
#define UBENCH_NOINLINE __declspec(noinline)
#else
#define UBENCH_NOINLINE __attribute__(noinline)
#endif



namespace detail {
  
inline std::chrono::microseconds elapsed_us(std::chrono::steady_clock::time_point tp) {
  using namespace std::chrono;  
  return duration_cast<microseconds>(steady_clock::now() - tp);
}
  
} // detail



template<typename F> result UBENCH_NOINLINE
run(F&& f,
    unsigned nof_tests = 10,
    std::chrono::steady_clock::duration min_duration = std::chrono::steady_clock::duration{5000}) {
  
  using namespace std::chrono;

  auto const origin = steady_clock::now();
  
  constexpr unsigned max_iterations = 100000;
  unsigned n = 1; auto elapsed = steady_clock::duration{};
  
  do {
    auto const started = steady_clock::now();
    for(unsigned i = 0; i != n; ++i)
      f();
    elapsed = steady_clock::now() - started;
    if(elapsed >= min_duration)
      break;
    n *= 10;
  } while(n <= max_iterations);

  if(elapsed < min_duration)
    return {result::optimized, detail::elapsed_us(origin)};

  std::multiset<steady_clock::duration> samples;
  for(unsigned i = 0; i != nof_tests; ++i) {
    auto const started = steady_clock::now();
    for(unsigned j = 0; j != n; ++j)
      f();
    samples.emplace(steady_clock::now() - started);
  }

  // filter "cold" results
  for(unsigned i = 0; i != nof_tests / 5; ++i) {
    auto end = samples.end();
    samples.erase(--end);
  }

  steady_clock::duration::rep average = 0;
  for(steady_clock::duration const& each: samples)
    average += each.count();
  average /= samples.size();
  
  auto const ns = duration_cast<nanoseconds>(steady_clock::duration{average}).count();
  return {result::timing{double(ns) / n}, detail::elapsed_us(origin)};
}


} // ubench
