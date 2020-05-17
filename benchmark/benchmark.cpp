#include <cstdio>
#include <chrono>
#include <thread>
#include <vector>
#include <algorithm>
#include <mutex>
#include <ubench/ubench.hpp>

#include <udisruptor/ring_buffer.hpp>
#include <udisruptor/multisequencer.hpp>
#include <udisruptor/sequencer.hpp>


constexpr auto events_count = 10000000;
constexpr auto buffer_size = 100000;
constexpr auto consumers_count = 1;
constexpr auto producers_count = 8;


template<typename S > void microbench(char const* title) {
  udisruptor::ring_buffer<int64_t> buffer{8192};
  S sequencer{buffer.capacity()};
  auto consumer_seq = sequencer.add_consumer();
  auto const publish_us = ubench::run([&] {
    auto const index = sequencer.claim();
    buffer[index] = index;
    sequencer.publish(index);
    auto const next = consumer_seq->next();
    sequencer.try_fetch(next);
    *consumer_seq = next;
  });

  printf("%s publish/fetch - %.1f ns\n", title, publish_us.time.count());
}


int main() {

  microbench<udisruptor::sequencer>("sequencer");
  microbench<udisruptor::multisequencer>("multisequencer");

  udisruptor::ring_buffer<int64_t> buffer{buffer_size};
  udisruptor::multisequencer sequencer{buffer.capacity()};
  std::vector<std::chrono::nanoseconds> producer_timings;
  std::mutex producer_timings_sync;

  constexpr auto events_per_producer = events_count / producers_count;
  constexpr auto events_to_consume = events_per_producer * producers_count;

  auto const consumer = [&](udisruptor::sequence* consumer_seq) {
    auto events_consumed = 0;
    while(events_consumed != events_to_consume) {
      auto const next = consumer_seq->next();
      auto const until = sequencer.try_fetch_all(next);
      if(until == next) {
        std::this_thread::sleep_for(std::chrono::microseconds{20});
        continue;
      }
      for(auto i = next; i != until; ++i) {
        auto const value = buffer[i];
        if(value != i)
          puts("Oops");
        ++events_consumed;
      }
      *consumer_seq = until - 1;
    }

  };

  std::vector<std::thread> consumers;
  consumers.reserve(consumers_count);
  for(auto i = 0; i != consumers_count; ++i)
    consumers.emplace_back(std::thread{consumer, sequencer.add_consumer()});

  if(!buffer || !sequencer) {
    printf("Disruptor is not ready\n");
    return 1;
  }

  auto const producer = [&] {
    using namespace std::chrono;
    auto const started = steady_clock::now();
    for(auto i = 0; i != events_per_producer; ++i) {
      auto const index = sequencer.claim();
      buffer[index] = index;
      sequencer.publish(index);
    }
    auto const ended = steady_clock::now();
    std::unique_lock g(producer_timings_sync);
    producer_timings.push_back(duration_cast<nanoseconds>(ended - started));
  };

  std::vector<std::thread> producers;
  producers.reserve(producers_count);
  for(auto i = 0; i != producers_count; ++i)
    producers.emplace_back(std::thread{producer});

  for(auto& producer: producers)
    producer.join();

  for(auto& consumer: consumers)
    consumer.join();

  auto const max_timing = std::max_element(producer_timings.begin(), producer_timings.end());
  double const events_per_ns = double(events_count) / max_timing->count();
  double const events_per_s = events_per_ns * 1000000000;

  printf("Throughtput - %.0f events/second\n", events_per_s);

  return 0;
}
